mod analysis;
mod cast;

use analysis::{
    collect_function_contexts, collect_function_param_types, collect_global_vars,
    collect_struct_fields, pat_name,
};
use cast::{
    remove_cast_if_redundant, remove_casts_if_redundant_for_expected_type, type_info_from_ty,
};

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{
    BinOpKind, Block, Crate, Expr, ExprKind, Item, ItemKind, Local, LocalKind, ModKind, StmtKind,
    UnOp,
};
use rustc_span::Span;

use std::collections::HashMap;

#[derive(Clone, Debug)]
struct BitWidths {
    c_char_bits: u32,
    c_short_bits: u32,
    c_int_bits: u32,
    c_long_bits: u32,
    c_longlong_bits: u32,
    pointer_bits: u32,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum CIntegerType {
    SChar,
    UChar,
    Short,
    UShort,
    Int,
    UInt,
    Long,
    ULong,
    LongLong,
    ULongLong,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum IntegerType {
    Primitive(&'static str),
    C(CIntegerType),
}

#[derive(Clone, Debug)]
struct TypePath {
    candidates: Vec<String>,
}

impl TypePath {
    fn from_ast_path(path: &rustc_ast::Path, module_path: &[String]) -> Option<Self> {
        let candidates = path_candidates(path, module_path);
        if candidates.is_empty() {
            None
        } else {
            Some(Self { candidates })
        }
    }
}

#[derive(Clone, Debug)]
enum TypeInfo {
    Integer(IntegerType),
    Struct(TypePath),
    Pointer(Box<TypeInfo>),
    Array(Box<TypeInfo>),
}

#[derive(Clone, Debug)]
struct FunctionContext {
    params: HashMap<String, TypeInfo>,
    return_type: Option<IntegerType>,
}

struct RemoveRedundantIntegerCasts {
    bit_widths: BitWidths,
    global_vars: HashMap<String, TypeInfo>,
    struct_fields: HashMap<String, HashMap<String, TypeInfo>>,
    function_param_types: HashMap<String, Vec<Option<TypeInfo>>>,
    function_contexts: Vec<(Span, FunctionContext)>,
    local_scopes: Vec<HashMap<String, TypeInfo>>,
    current_return_types: Vec<Option<IntegerType>>,
    current_module_path: Vec<String>,
}

pub fn register_remove_redundant_integer_casts(reg: &mut Registry) {
    reg.register("remove_redundant_integer_casts", |args| {
        let args = args.to_vec();
        Box::new(DriverCommand::new(Phase::Phase1, move |st, cx| {
            let target = &cx.session().target;
            let c_int_bits = target
                .c_int_width
                .parse::<u32>()
                .expect("Invalid rustc target c_int_width");
            let pointer_bits = target.pointer_width;
            let windows_llp64 = target.os == "windows" || target.env == "msvc";
            let c_long_bits = if windows_llp64 { 32 } else { pointer_bits };
            let mut bit_widths = BitWidths {
                c_char_bits: 8,
                c_short_bits: 16,
                c_int_bits,
                c_long_bits,
                c_longlong_bits: 64,
                pointer_bits,
            };
            for arg in &args {
                if let Some(value) = arg.strip_prefix("char-width=") {
                    bit_widths.c_char_bits =
                        value.parse::<u32>().expect("Invalid value for char-width");
                } else if let Some(value) = arg.strip_prefix("short-width=") {
                    bit_widths.c_short_bits =
                        value.parse::<u32>().expect("Invalid value for short-width");
                } else if let Some(value) = arg.strip_prefix("int-width=") {
                    bit_widths.c_int_bits =
                        value.parse::<u32>().expect("Invalid value for int-width");
                } else if let Some(value) = arg.strip_prefix("long-width=") {
                    bit_widths.c_long_bits =
                        value.parse::<u32>().expect("Invalid value for long-width");
                } else if let Some(value) = arg.strip_prefix("longlong-width=") {
                    bit_widths.c_longlong_bits = value
                        .parse::<u32>()
                        .expect("Invalid value for longlong-width");
                } else {
                    panic!(
                        "Unknown argument for \
                             remove_redundant_integer_casts: {}",
                        arg
                    );
                }
            }
            validate_c_integer_widths(&bit_widths);
            remove_redundant_integer_casts(&mut st.krate_mut(), bit_widths);
        }))
    });
}

fn validate_c_integer_widths(bit_widths: &BitWidths) {
    let valid_width = |bits: u32| matches!(bits, 8 | 16 | 32 | 64 | 128);
    if !(valid_width(bit_widths.c_char_bits)
        && valid_width(bit_widths.c_short_bits)
        && valid_width(bit_widths.c_int_bits)
        && valid_width(bit_widths.c_long_bits)
        && valid_width(bit_widths.c_longlong_bits)
        && bit_widths.c_char_bits >= 8
        && bit_widths.c_short_bits >= 16
        && bit_widths.c_int_bits >= 16
        && bit_widths.c_long_bits >= 32
        && bit_widths.c_longlong_bits >= 64
        && bit_widths.c_char_bits <= bit_widths.c_short_bits
        && bit_widths.c_short_bits <= bit_widths.c_int_bits
        && bit_widths.c_int_bits <= bit_widths.c_long_bits
        && bit_widths.c_long_bits <= bit_widths.c_longlong_bits)
    {
        panic!(
            concat!(
                "Invalid C integer widths: expected widths in ",
                "{{8, 16, 32, 64, 128}}, ",
                "minimums char>=8, short>=16, int>=16, ",
                "long>=32, long long>=64, ",
                "and char <= short <= int <= long <= long long; ",
                "got char={}, short={}, int={}, long={}, ",
                "long long={}"
            ),
            bit_widths.c_char_bits,
            bit_widths.c_short_bits,
            bit_widths.c_int_bits,
            bit_widths.c_long_bits,
            bit_widths.c_longlong_bits
        );
    }
}

fn remove_redundant_integer_casts(krate: &mut Crate, bit_widths: BitWidths) {
    let struct_fields = collect_struct_fields(krate, &bit_widths);
    let global_vars = collect_global_vars(krate, &bit_widths);
    let function_param_types = collect_function_param_types(krate, &bit_widths);
    let function_contexts = collect_function_contexts(krate, &bit_widths);
    let mut visitor = RemoveRedundantIntegerCasts {
        bit_widths,
        global_vars,
        struct_fields,
        function_param_types,
        function_contexts,
        local_scopes: Vec::new(),
        current_return_types: Vec::new(),
        current_module_path: Vec::new(),
    };
    visitor.visit_crate(krate);
}

impl MutVisitor for RemoveRedundantIntegerCasts {
    fn visit_crate(&mut self, krate: &mut Crate) {
        let mut module_path = Vec::new();
        remove_redundant_casts_in_item_initializers(
            &mut krate.items,
            &self.bit_widths,
            &mut module_path,
        );
        visit_items_with_module_path(&mut krate.items, self);
    }

    fn visit_block(&mut self, block: &mut P<Block>) {
        let function_context = self.function_context_for_block(block.span).cloned();

        if let Some(ref context) = function_context {
            self.local_scopes.push(context.params.clone());

            self.current_return_types.push(context.return_type);
        } else {
            self.local_scopes.push(HashMap::new());
        }
        for stmt in block.stmts.iter_mut() {
            if let StmtKind::Item(ref mut item) = stmt.kind {
                remove_redundant_casts_in_item_initializer(
                    item,
                    &self.bit_widths,
                    &self.current_module_path,
                );
                match item.kind {
                    ItemKind::Static(ref ty, ..) => {
                        if let Some(type_info) =
                            type_info_from_ty(ty, &self.bit_widths, &self.current_module_path)
                        {
                            self.insert_local_var(item.ident.name.to_string(), type_info);
                        }
                    }
                    ItemKind::Const(_, ref ty, ..) => {
                        if let Some(type_info) =
                            type_info_from_ty(ty, &self.bit_widths, &self.current_module_path)
                        {
                            self.insert_local_var(item.ident.name.to_string(), type_info);
                        }
                    }
                    _ => {}
                }
            }
        }
        mut_visit::noop_visit_block(block, self);
        if function_context.is_some() {
            self.current_return_types.pop();
        }
        self.local_scopes.pop();
    }

    fn visit_local(&mut self, local: &mut P<Local>) {
        let expected_type = match local.ty {
            Some(ref expected_ty) => {
                type_info_from_ty(expected_ty, &self.bit_widths, &self.current_module_path)
            }
            None => None,
        };
        if let Some(expected_type) = expected_type {
            if let Some(name) = local_name(local) {
                self.insert_local_var(name, expected_type.clone());
            }
            match local.kind {
                LocalKind::Init(ref mut init) => {
                    remove_casts_if_redundant_for_expected_type(
                        init,
                        &expected_type,
                        &self.bit_widths,
                    );
                }
                LocalKind::InitElse(ref mut init, _) => {
                    remove_casts_if_redundant_for_expected_type(
                        init,
                        &expected_type,
                        &self.bit_widths,
                    );
                }
                LocalKind::Decl => {}
            }
        }
        mut_visit::noop_visit_local(local, self);
    }

    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        match expr.kind {
            ExprKind::Assign(ref lhs, ref mut rhs, _) => {
                if let Some(TypeInfo::Integer(expected_type)) =
                    self.expected_type_for_assignment_lhs(lhs)
                {
                    remove_cast_if_redundant(rhs, expected_type, &self.bit_widths);
                }
            }
            ExprKind::Ret(Some(ref mut ret_expr)) => {
                if let Some(Some(expected_type)) = self.current_return_types.last() {
                    remove_cast_if_redundant(ret_expr, *expected_type, &self.bit_widths);
                }
            }
            ExprKind::Call(ref callee, ref mut args) => {
                if let ExprKind::Path(_, ref path) = callee.kind {
                    if let Some(function_key) = resolve_path_in_map(
                        path,
                        &self.current_module_path,
                        &self.function_param_types,
                    ) {
                        if let Some(param_types) =
                            self.function_param_types.get(&function_key).cloned()
                        {
                            for (arg, expected_type) in args.iter_mut().zip(param_types.into_iter())
                            {
                                if let Some(TypeInfo::Integer(expected_type)) = expected_type {
                                    remove_cast_if_redundant(arg, expected_type, &self.bit_widths);
                                }
                            }
                        }
                    }
                }
            }
            ExprKind::Struct(ref mut struct_expr) => {
                if let Some(struct_key) = resolve_path_in_map(
                    &struct_expr.path,
                    &self.current_module_path,
                    &self.struct_fields,
                ) {
                    if let Some(field_types) = self.struct_fields.get(&struct_key).cloned() {
                        for field in struct_expr.fields.iter_mut() {
                            let field_name = field.ident.name.to_string();
                            if let Some(TypeInfo::Integer(expected_type)) =
                                field_types.get(&field_name)
                            {
                                remove_cast_if_redundant(
                                    &mut field.expr,
                                    *expected_type,
                                    &self.bit_widths,
                                );
                            }
                        }
                    }
                }
            }
            ExprKind::Binary(ref op, ref mut left, ref mut right) => {
                if is_integer_comparison_op(op.node) {
                    self.remove_redundant_casts_in_comparison(left, right);
                }
            }
            _ => {}
        }
        mut_visit::noop_visit_expr(expr, self);
    }
}

fn visit_items_with_module_path(
    items: &mut Vec<P<Item>>,
    visitor: &mut RemoveRedundantIntegerCasts,
) {
    let old_items = std::mem::take(items);
    let mut new_items = Vec::with_capacity(old_items.len());
    for mut item in old_items {
        let item_name = item.ident.name.to_string();
        if let ItemKind::Mod(_, ModKind::Loaded(ref mut module_items, ..)) = item.kind {
            visitor.current_module_path.push(item_name);
            visit_items_with_module_path(module_items, visitor);
            visitor.current_module_path.pop();
            new_items.push(item);
        } else {
            new_items.extend(mut_visit::noop_flat_map_item(item, visitor));
        }
    }
    *items = new_items;
}

fn remove_redundant_casts_in_item_initializers(
    items: &mut Vec<P<Item>>,
    bit_widths: &BitWidths,
    module_path: &mut Vec<String>,
) {
    for item in items.iter_mut() {
        remove_redundant_casts_in_item_initializer(item, bit_widths, module_path);
        let item_name = item.ident.name.to_string();
        if let ItemKind::Mod(_, ModKind::Loaded(ref mut module_items, ..)) = item.kind {
            module_path.push(item_name);
            remove_redundant_casts_in_item_initializers(module_items, bit_widths, module_path);
            module_path.pop();
        }
    }
}

fn remove_redundant_casts_in_item_initializer(
    item: &mut P<Item>,
    bit_widths: &BitWidths,
    module_path: &[String],
) {
    match item.kind {
        ItemKind::Static(ref ty, _, ref mut init) => {
            if let Some(ref mut init_expr) = *init {
                if let Some(expected_type) = type_info_from_ty(ty, bit_widths, module_path) {
                    remove_casts_if_redundant_for_expected_type(
                        init_expr,
                        &expected_type,
                        bit_widths,
                    );
                }
            }
        }
        ItemKind::Const(_, ref ty, ref mut init) => {
            if let Some(ref mut init_expr) = *init {
                if let Some(expected_type) = type_info_from_ty(ty, bit_widths, module_path) {
                    remove_casts_if_redundant_for_expected_type(
                        init_expr,
                        &expected_type,
                        bit_widths,
                    );
                }
            }
        }
        _ => {}
    }
}

impl RemoveRedundantIntegerCasts {
    fn insert_local_var(&mut self, name: String, type_info: TypeInfo) {
        if let Some(scope) = self.local_scopes.last_mut() {
            scope.insert(name, type_info);
        }
    }

    fn lookup_var_type(&self, path: &rustc_ast::Path) -> Option<TypeInfo> {
        if is_simple_unqualified_path(path) {
            let name = path.segments[0].ident.name.to_string();
            for scope in self.local_scopes.iter().rev() {
                if let Some(type_info) = scope.get(&name) {
                    return Some(type_info.clone());
                }
            }
        }
        let key = resolve_path_in_map(path, &self.current_module_path, &self.global_vars)?;
        self.global_vars.get(&key).cloned()
    }

    fn function_context_for_block(&self, span: Span) -> Option<&FunctionContext> {
        self.function_contexts
            .iter()
            .find(|(body_span, _)| *body_span == span)
            .map(|(_, context)| context)
    }

    fn expected_type_for_assignment_lhs(&self, lhs: &Expr) -> Option<TypeInfo> {
        self.type_info_for_expr(lhs)
    }

    fn remove_redundant_casts_in_comparison(&self, left: &mut P<Expr>, right: &mut P<Expr>) {
        let left_type = self.type_info_for_expr(left);
        let right_type = self.type_info_for_expr(right);
        if let Some(TypeInfo::Integer(expected_type)) = right_type {
            remove_cast_if_redundant(left, expected_type, &self.bit_widths);
        }
        if let Some(TypeInfo::Integer(expected_type)) = left_type {
            remove_cast_if_redundant(right, expected_type, &self.bit_widths);
        }
    }

    fn type_info_for_expr(&self, expr: &Expr) -> Option<TypeInfo> {
        match expr.kind {
            ExprKind::Path(_, ref path) => self.lookup_var_type(path),
            ExprKind::Field(ref base, ident) => {
                let base_type = self.type_info_for_expr(base)?;

                self.field_type(base_type, &ident.name.to_string())
            }
            ExprKind::Unary(UnOp::Deref, ref inner) => {
                let pointer_type = self.type_info_for_expr(inner)?;
                match pointer_type {
                    TypeInfo::Pointer(inner_type) => Some(*inner_type),
                    _ => None,
                }
            }
            ExprKind::Index(ref base, _) => {
                let base_type = self.type_info_for_expr(base)?;
                match base_type {
                    TypeInfo::Array(element_type) => Some(*element_type),
                    TypeInfo::Pointer(element_type) => Some(*element_type),
                    _ => None,
                }
            }
            ExprKind::MethodCall(ref segment, ref receiver, _, _) => {
                if segment.ident.name.to_string() == "offset" {
                    return self.type_info_for_expr(receiver);
                }
                None
            }
            ExprKind::Paren(ref inner) => self.type_info_for_expr(inner),
            _ => None,
        }
    }

    fn field_type(&self, base_type: TypeInfo, field_name: &str) -> Option<TypeInfo> {
        let struct_path = match base_type {
            TypeInfo::Struct(struct_path) => struct_path,
            _ => return None,
        };
        for candidate in struct_path.candidates {
            if let Some(field_types) = self.struct_fields.get(&candidate) {
                return field_types.get(field_name).cloned();
            }
        }
        None
    }
}

fn path_candidates(path: &rustc_ast::Path, module_path: &[String]) -> Vec<String> {
    let mut segments: Vec<String> = path
        .segments
        .iter()
        .map(|segment| segment.ident.name.to_string())
        .collect();
    if segments.is_empty() {
        return Vec::new();
    }
    if path.is_global() {
        segments.remove(0);
        return vec![segments.join("::")];
    }
    match segments[0].as_str() {
        "crate" => {
            segments.remove(0);
            vec![segments.join("::")]
        }
        "self" => {
            segments.remove(0);
            vec![join_module_and_segments(module_path, &segments)]
        }
        "super" => {
            let mut super_count = 0usize;
            while segments.get(super_count).map(|segment| segment.as_str()) == Some("super") {
                super_count += 1;
            }
            if super_count > module_path.len() {
                return Vec::new();
            }
            let base_len = module_path.len() - super_count;
            vec![join_module_and_segments(
                &module_path[..base_len],
                &segments[super_count..],
            )]
        }
        _ => {
            let mut candidates = Vec::new();
            for prefix_len in (0..=module_path.len()).rev() {
                let candidate = join_module_and_segments(&module_path[..prefix_len], &segments);
                if !candidate.is_empty() && !candidates.contains(&candidate) {
                    candidates.push(candidate);
                }
            }
            candidates
        }
    }
}

fn resolve_path_in_map<T>(
    path: &rustc_ast::Path,
    module_path: &[String],
    map: &HashMap<String, T>,
) -> Option<String> {
    path_candidates(path, module_path)
        .into_iter()
        .find(|candidate| map.contains_key(candidate))
}

fn join_module_and_segments(module_path: &[String], segments: &[String]) -> String {
    match (module_path.is_empty(), segments.is_empty()) {
        (true, true) => String::new(),
        (true, false) => segments.join("::"),
        (false, true) => module_path.join("::"),
        (false, false) => format!("{}::{}", module_path.join("::"), segments.join("::")),
    }
}

fn is_simple_unqualified_path(path: &rustc_ast::Path) -> bool {
    !path.is_global() && path.segments.len() == 1
}

fn is_integer_comparison_op(op: BinOpKind) -> bool {
    matches!(
        op,
        BinOpKind::Lt
            | BinOpKind::Le
            | BinOpKind::Ne
            | BinOpKind::Eq
            | BinOpKind::Ge
            | BinOpKind::Gt
    )
}

fn local_name(local: &Local) -> Option<String> {
    pat_name(&local.pat)
}
