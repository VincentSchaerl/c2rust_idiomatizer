use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{
    BinOpKind, Block, BorrowKind, Crate, Expr, ExprKind, FnRetTy, ForeignItemKind, Item, ItemKind,
    LitKind, Local, LocalKind, ModKind, Pat, PatKind, StmtKind, Ty, TyKind, UnOp, VariantData,
};
use rustc_span::Span;

use std::collections::{HashMap, HashSet};

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
struct TypeAliasInfo {
    target: P<Ty>,
    module_path: Vec<String>,
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
    Reference(Box<TypeInfo>),
    Array(Box<TypeInfo>),
    Slice(Box<TypeInfo>),
    Unknown,
}

#[derive(Clone, Debug)]
struct FunctionContext {
    params: HashMap<String, TypeInfo>,
    return_type: Option<IntegerType>,
}

struct RemoveRedundantIntegerCasts {
    bit_widths: BitWidths,
    type_aliases: HashMap<String, TypeAliasInfo>,
    global_vars: HashMap<String, TypeInfo>,
    global_const_arrays: HashMap<String, TypeInfo>,
    struct_fields: HashMap<String, HashMap<String, TypeInfo>>,
    function_param_types: HashMap<String, Vec<Option<TypeInfo>>>,
    function_return_types: HashMap<String, Option<TypeInfo>>,
    function_contexts: Vec<(Span, FunctionContext)>,
    local_scopes: Vec<HashMap<String, TypeInfo>>,
    current_return_types: Vec<Option<IntegerType>>,
    current_module_path: Vec<String>,
    changed: bool,
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
    let type_aliases = collect_type_aliases(krate);
    let struct_fields = collect_struct_fields(krate, &bit_widths, &type_aliases);
    let global_vars = collect_global_vars(krate, &bit_widths, &type_aliases);
    let global_const_arrays = collect_global_const_arrays(krate, &bit_widths, &type_aliases);
    let function_param_types = collect_function_param_types(krate, &bit_widths, &type_aliases);
    let function_return_types = collect_function_return_types(krate, &bit_widths, &type_aliases);
    let function_contexts = collect_function_contexts(krate, &bit_widths, &type_aliases);
    let mut visitor = RemoveRedundantIntegerCasts {
        bit_widths,
        type_aliases,
        global_vars,
        global_const_arrays,
        struct_fields,
        function_param_types,
        function_return_types,
        function_contexts,
        local_scopes: Vec::new(),
        current_return_types: Vec::new(),
        current_module_path: Vec::new(),
        changed: false,
    };

    // Removing one cast can expose another cast that then satisfies the same
    // constraints, e.g. `let x: i32 = 0 as i32 as i32;`. Every successful
    // rewrite removes one Cast node, so this fixed-point iteration terminates.
    loop {
        visitor.changed = false;
        visitor.visit_crate(krate);
        if !visitor.changed {
            break;
        }
    }
}

impl MutVisitor for RemoveRedundantIntegerCasts {
    fn visit_crate(&mut self, krate: &mut Crate) {
        let mut module_path = Vec::new();
        if remove_redundant_casts_in_item_initializers(
            &mut krate.items,
            &self.bit_widths,
            &self.type_aliases,
            &mut module_path,
        ) {
            self.changed = true;
        }
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
                if remove_redundant_casts_in_item_initializer(
                    item,
                    &self.bit_widths,
                    &self.type_aliases,
                    &self.current_module_path,
                ) {
                    self.changed = true;
                }
                match item.kind {
                    ItemKind::Static(ref ty, ..) => {
                        if let Some(type_info) = type_info_from_ty(
                            ty,
                            &self.bit_widths,
                            &self.type_aliases,
                            &self.current_module_path,
                        ) {
                            self.insert_local_var(item.ident.name.to_string(), type_info);
                        }
                    }
                    ItemKind::Const(_, ref ty, ..) => {
                        if let Some(type_info) = type_info_from_ty(
                            ty,
                            &self.bit_widths,
                            &self.type_aliases,
                            &self.current_module_path,
                        ) {
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
            Some(ref expected_ty) => type_info_from_ty(
                expected_ty,
                &self.bit_widths,
                &self.type_aliases,
                &self.current_module_path,
            ),
            None => match &local.kind {
                LocalKind::Init(init) | LocalKind::InitElse(init, _) => self
                    .type_info_for_expr(init)
                    .filter(|info| array_or_slice_element_type(info).is_some()),
                LocalKind::Decl => None,
            },
        };
        // Even an unrecognized local binding shadows a global with the same
        // name. Recording it as Unknown prevents case 4.m from mistaking a
        // custom Index implementation for the shadowed global array.
        if let Some(name) = local_name(local) {
            self.insert_local_var(name, expected_type.clone().unwrap_or(TypeInfo::Unknown));
        }
        if let Some(expected_type) = expected_type {
            match local.kind {
                LocalKind::Init(ref mut init) => {
                    if remove_redundant_literal_casts_for_expected_type(
                        init,
                        &expected_type,
                        &self.bit_widths,
                        &self.type_aliases,
                        &self.current_module_path,
                    ) {
                        self.changed = true;
                    }
                }
                LocalKind::InitElse(ref mut init, _) => {
                    if remove_redundant_literal_casts_for_expected_type(
                        init,
                        &expected_type,
                        &self.bit_widths,
                        &self.type_aliases,
                        &self.current_module_path,
                    ) {
                        self.changed = true;
                    }
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
                    if remove_redundant_literal_cast(
                        rhs,
                        expected_type,
                        &self.bit_widths,
                        &self.type_aliases,
                        &self.current_module_path,
                    ) {
                        self.changed = true;
                    }
                }
            }
            ExprKind::Ret(Some(ref mut ret_expr)) => {
                if let Some(Some(expected_type)) = self.current_return_types.last() {
                    if remove_redundant_literal_cast(
                        ret_expr,
                        *expected_type,
                        &self.bit_widths,
                        &self.type_aliases,
                        &self.current_module_path,
                    ) {
                        self.changed = true;
                    }
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
                                    if remove_redundant_literal_cast(
                                        arg,
                                        expected_type,
                                        &self.bit_widths,
                                        &self.type_aliases,
                                        &self.current_module_path,
                                    ) {
                                        self.changed = true;
                                    }
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
                                if remove_redundant_literal_cast(
                                    &mut field.expr,
                                    *expected_type,
                                    &self.bit_widths,
                                    &self.type_aliases,
                                    &self.current_module_path,
                                ) {
                                    self.changed = true;
                                }
                            }
                        }
                    }
                }
            }
            ExprKind::Binary(ref op, ref mut left, ref mut right) => {
                if is_integer_comparison_op(op.node) {
                    self.remove_redundant_literal_casts_in_comparison(left, right);
                }
            }
            ExprKind::Index(ref base, ref mut index) => {
                // Case 4.m: only built-in array/slice indexing has a `usize`
                // index. Do not assume that arbitrary Index/IndexMut impls do.
                if self.is_array_or_slice_index_base(base) {
                    if remove_redundant_literal_cast(
                        index,
                        IntegerType::Primitive("usize"),
                        &self.bit_widths,
                        &self.type_aliases,
                        &self.current_module_path,
                    ) {
                        self.changed = true;
                    }
                }
            }
            ExprKind::Cast(ref mut inner_expr, ref cast_ty) => {
                // Case 4.a: in `L as T as U` and `(L as T) as U`, the outer
                // cast supplies U as the expected type of the inner cast.
                if let Some(expected_type) =
                    integer_type_from_ty(cast_ty, &self.current_module_path, &self.type_aliases)
                {
                    if remove_redundant_literal_cast(
                        inner_expr,
                        expected_type,
                        &self.bit_widths,
                        &self.type_aliases,
                        &self.current_module_path,
                    ) {
                        self.changed = true;
                    }
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
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &mut Vec<String>,
) -> bool {
    let mut changed = false;
    for item in items.iter_mut() {
        changed |=
            remove_redundant_casts_in_item_initializer(item, bit_widths, type_aliases, module_path);
        let item_name = item.ident.name.to_string();
        if let ItemKind::Mod(_, ModKind::Loaded(ref mut module_items, ..)) = item.kind {
            module_path.push(item_name);
            changed |= remove_redundant_casts_in_item_initializers(
                module_items,
                bit_widths,
                type_aliases,
                module_path,
            );
            module_path.pop();
        }
    }
    changed
}

fn remove_redundant_casts_in_item_initializer(
    item: &mut P<Item>,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &[String],
) -> bool {
    match item.kind {
        ItemKind::Static(ref ty, _, ref mut init) => {
            if let Some(ref mut init_expr) = *init {
                if let Some(expected_type) =
                    type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                {
                    return remove_redundant_literal_casts_for_expected_type(
                        init_expr,
                        &expected_type,
                        bit_widths,
                        type_aliases,
                        module_path,
                    );
                }
            }
        }
        ItemKind::Const(_, ref ty, ref mut init) => {
            if let Some(ref mut init_expr) = *init {
                if let Some(expected_type) =
                    type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                {
                    return remove_redundant_literal_casts_for_expected_type(
                        init_expr,
                        &expected_type,
                        bit_widths,
                        type_aliases,
                        module_path,
                    );
                }
            }
        }
        _ => {}
    }
    false
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
        if let Some(key) = resolve_path_in_map(path, &self.current_module_path, &self.global_vars) {
            return self.global_vars.get(&key).cloned();
        }
        let key = resolve_path_in_map(path, &self.current_module_path, &self.global_const_arrays)?;
        self.global_const_arrays.get(&key).cloned()
    }

    fn is_array_or_slice_index_base(&self, base: &Expr) -> bool {
        self.type_info_for_expr(base)
            .as_ref()
            .and_then(array_or_slice_element_type)
            .is_some()
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

    fn remove_redundant_literal_casts_in_comparison(
        &mut self,
        left: &mut P<Expr>,
        right: &mut P<Expr>,
    ) {
        let left_type = self.type_info_for_expr(left);
        let right_type = self.type_info_for_expr(right);
        if let Some(TypeInfo::Integer(expected_type)) = right_type {
            if remove_redundant_literal_cast(
                left,
                expected_type,
                &self.bit_widths,
                &self.type_aliases,
                &self.current_module_path,
            ) {
                self.changed = true;
            }
        }
        if let Some(TypeInfo::Integer(expected_type)) = left_type {
            if remove_redundant_literal_cast(
                right,
                expected_type,
                &self.bit_widths,
                &self.type_aliases,
                &self.current_module_path,
            ) {
                self.changed = true;
            }
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
                    TypeInfo::Pointer(inner_type) | TypeInfo::Reference(inner_type) => {
                        Some(*inner_type)
                    }
                    _ => None,
                }
            }
            ExprKind::AddrOf(BorrowKind::Ref, _, ref inner) => Some(TypeInfo::Reference(Box::new(
                self.type_info_for_expr(inner)?,
            ))),
            ExprKind::AddrOf(BorrowKind::Raw, _, ref inner) => {
                Some(TypeInfo::Pointer(Box::new(self.type_info_for_expr(inner)?)))
            }
            ExprKind::Array(ref elements) => Some(TypeInfo::Array(Box::new(
                elements
                    .first()
                    .and_then(|element| self.type_info_for_expr(element))
                    .unwrap_or(TypeInfo::Unknown),
            ))),
            ExprKind::Repeat(ref element, _) => Some(TypeInfo::Array(Box::new(
                self.type_info_for_expr(element)
                    .unwrap_or(TypeInfo::Unknown),
            ))),
            ExprKind::Index(ref base, _) => {
                let base_type = self.type_info_for_expr(base)?;
                if let Some(element_type) = array_or_slice_element_type(&base_type) {
                    return Some(element_type);
                }
                match base_type {
                    // Preserve the old pointer-element analysis. It does not
                    // qualify the base for the new array/slice index rewrite.
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
            ExprKind::Call(ref callee, _) => {
                let path = match callee.kind {
                    ExprKind::Path(_, ref path) => path,
                    _ => return None,
                };
                let function_key = resolve_path_in_map(
                    path,
                    &self.current_module_path,
                    &self.function_return_types,
                )?;
                match self.function_return_types.get(&function_key) {
                    Some(Some(return_type)) => Some(return_type.clone()),
                    _ => None,
                }
            }
            ExprKind::Paren(ref inner) => self.type_info_for_expr(inner),
            _ => None,
        }
    }

    fn field_type(&self, base_type: TypeInfo, field_name: &str) -> Option<TypeInfo> {
        let struct_path = match base_type {
            TypeInfo::Struct(struct_path) => struct_path,
            TypeInfo::Reference(inner) => return self.field_type(*inner, field_name),
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

// Built-in indexing uses usize for arrays and slices, including any number of
// shared or mutable reference layers. A raw pointer is not auto-dereferenced.
fn array_or_slice_element_type(base_type: &TypeInfo) -> Option<TypeInfo> {
    match base_type {
        TypeInfo::Array(element) | TypeInfo::Slice(element) => Some((**element).clone()),
        TypeInfo::Reference(inner) => array_or_slice_element_type(inner),
        _ => None,
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

// -----------------------------------------------------------------------------
// Analysis helpers
// -----------------------------------------------------------------------------

fn collect_type_aliases(krate: &Crate) -> HashMap<String, TypeAliasInfo> {
    let mut aliases = HashMap::new();
    let mut module_path = Vec::new();
    collect_type_aliases_from_items(&krate.items, &mut module_path, &mut aliases);
    aliases
}

fn collect_type_aliases_from_items(
    items: &[P<Item>],
    module_path: &mut Vec<String>,
    aliases: &mut HashMap<String, TypeAliasInfo>,
) {
    for item in items {
        match item.kind {
            ItemKind::TyAlias(ref alias) => {
                if let Some(ref target) = alias.ty {
                    aliases.insert(
                        qualified_item_name(module_path, &item.ident.name.to_string()),
                        TypeAliasInfo {
                            target: target.clone(),
                            module_path: module_path.to_vec(),
                        },
                    );
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_type_aliases_from_items(module_items, module_path, aliases);
                module_path.pop();
            }
            _ => {}
        }
    }
}

fn collect_struct_fields(
    krate: &Crate,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> HashMap<String, HashMap<String, TypeInfo>> {
    let mut structs = HashMap::new();
    let mut module_path = Vec::new();
    collect_struct_fields_from_items(
        &krate.items,
        bit_widths,
        type_aliases,
        &mut module_path,
        &mut structs,
    );
    structs
}

fn collect_struct_fields_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &mut Vec<String>,
    structs: &mut HashMap<String, HashMap<String, TypeInfo>>,
) {
    for item in items {
        match item.kind {
            ItemKind::Struct(ref variant_data, _) => {
                let mut fields = HashMap::new();
                collect_fields_from_variant_data(
                    variant_data,
                    bit_widths,
                    type_aliases,
                    module_path,
                    &mut fields,
                );
                structs.insert(
                    qualified_item_name(module_path, &item.ident.name.to_string()),
                    fields,
                );
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_struct_fields_from_items(
                    module_items,
                    bit_widths,
                    type_aliases,
                    module_path,
                    structs,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

fn collect_fields_from_variant_data(
    variant_data: &VariantData,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &[String],
    fields: &mut HashMap<String, TypeInfo>,
) {
    for field in variant_data.fields() {
        if let Some(ident) = field.ident {
            if let Some(type_info) =
                type_info_from_ty(&field.ty, bit_widths, type_aliases, module_path)
            {
                fields.insert(ident.name.to_string(), type_info);
            }
        }
    }
}

fn collect_global_vars(
    krate: &Crate,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> HashMap<String, TypeInfo> {
    let mut vars = HashMap::new();
    let mut module_path = Vec::new();
    collect_global_vars_from_items(
        &krate.items,
        bit_widths,
        type_aliases,
        &mut module_path,
        &mut vars,
    );
    vars
}

fn collect_global_vars_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &mut Vec<String>,
    vars: &mut HashMap<String, TypeInfo>,
) {
    for item in items {
        match item.kind {
            ItemKind::Static(ref ty, ..) => {
                if let Some(type_info) =
                    type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                {
                    vars.insert(
                        qualified_item_name(module_path, &item.ident.name.to_string()),
                        type_info,
                    );
                }
            }
            ItemKind::ForeignMod(ref foreign_mod) => {
                // Extern statics are global values as well. Their declared types
                // allow built-in array/slice indexing to be recognized in case 4.m.
                for foreign_item in &foreign_mod.items {
                    if let ForeignItemKind::Static(ref ty, ..) = foreign_item.kind {
                        if let Some(type_info) =
                            type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                        {
                            vars.insert(
                                qualified_item_name(
                                    module_path,
                                    &foreign_item.ident.name.to_string(),
                                ),
                                type_info,
                            );
                        }
                    }
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_global_vars_from_items(
                    module_items,
                    bit_widths,
                    type_aliases,
                    module_path,
                    vars,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

// Constants are also valid array/slice receivers for case 4.m. Keep this
// distinct from the existing globals map to avoid expanding unrelated cases.
fn collect_global_const_arrays(
    krate: &Crate,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> HashMap<String, TypeInfo> {
    fn collect(
        items: &[P<Item>],
        bit_widths: &BitWidths,
        type_aliases: &HashMap<String, TypeAliasInfo>,
        module_path: &mut Vec<String>,
        arrays: &mut HashMap<String, TypeInfo>,
    ) {
        for item in items {
            match item.kind {
                ItemKind::Const(_, ref ty, ..) => {
                    if let Some(info) = type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                    {
                        if array_or_slice_element_type(&info).is_some() {
                            arrays.insert(
                                qualified_item_name(module_path, &item.ident.name.to_string()),
                                info,
                            );
                        }
                    }
                }
                ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                    module_path.push(item.ident.name.to_string());
                    collect(module_items, bit_widths, type_aliases, module_path, arrays);
                    module_path.pop();
                }
                _ => {}
            }
        }
    }

    let mut arrays = HashMap::new();
    collect(
        &krate.items,
        bit_widths,
        type_aliases,
        &mut Vec::new(),
        &mut arrays,
    );
    arrays
}

fn collect_function_param_types(
    krate: &Crate,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> HashMap<String, Vec<Option<TypeInfo>>> {
    let mut functions = HashMap::new();
    let mut module_path = Vec::new();
    collect_function_param_types_from_items(
        &krate.items,
        bit_widths,
        type_aliases,
        &mut module_path,
        &mut functions,
    );
    functions
}

fn collect_function_param_types_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &mut Vec<String>,
    functions: &mut HashMap<String, Vec<Option<TypeInfo>>>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                let mut params = Vec::new();
                for param in &func.sig.decl.inputs {
                    params.push(type_info_from_ty(
                        &param.ty,
                        bit_widths,
                        type_aliases,
                        module_path,
                    ));
                }
                functions.insert(
                    qualified_item_name(module_path, &item.ident.name.to_string()),
                    params,
                );
            }
            ItemKind::ForeignMod(ref foreign_mod) => {
                for foreign_item in &foreign_mod.items {
                    if let ForeignItemKind::Fn(ref func) = foreign_item.kind {
                        let mut params = Vec::new();
                        for param in &func.sig.decl.inputs {
                            params.push(type_info_from_ty(
                                &param.ty,
                                bit_widths,
                                type_aliases,
                                module_path,
                            ));
                        }
                        functions.insert(
                            qualified_item_name(module_path, &foreign_item.ident.name.to_string()),
                            params,
                        );
                    }
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_function_param_types_from_items(
                    module_items,
                    bit_widths,
                    type_aliases,
                    module_path,
                    functions,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

fn collect_function_return_types(
    krate: &Crate,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> HashMap<String, Option<TypeInfo>> {
    let mut functions = HashMap::new();
    let mut module_path = Vec::new();
    collect_function_return_types_from_items(
        &krate.items,
        bit_widths,
        type_aliases,
        &mut module_path,
        &mut functions,
    );
    functions
}

fn collect_function_return_types_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &mut Vec<String>,
    functions: &mut HashMap<String, Option<TypeInfo>>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                let return_type = match func.sig.decl.output {
                    FnRetTy::Ty(ref ty) => {
                        type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                    }
                    FnRetTy::Default(_) => None,
                };
                functions.insert(
                    qualified_item_name(module_path, &item.ident.name.to_string()),
                    return_type,
                );
            }
            ItemKind::ForeignMod(ref foreign_mod) => {
                for foreign_item in &foreign_mod.items {
                    if let ForeignItemKind::Fn(ref func) = foreign_item.kind {
                        let return_type = match func.sig.decl.output {
                            FnRetTy::Ty(ref ty) => {
                                type_info_from_ty(ty, bit_widths, type_aliases, module_path)
                            }
                            FnRetTy::Default(_) => None,
                        };
                        functions.insert(
                            qualified_item_name(module_path, &foreign_item.ident.name.to_string()),
                            return_type,
                        );
                    }
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_function_return_types_from_items(
                    module_items,
                    bit_widths,
                    type_aliases,
                    module_path,
                    functions,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

fn collect_function_contexts(
    krate: &Crate,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> Vec<(Span, FunctionContext)> {
    let mut contexts = Vec::new();
    let mut module_path = Vec::new();
    collect_function_contexts_from_items(
        &krate.items,
        bit_widths,
        type_aliases,
        &mut module_path,
        &mut contexts,
    );
    contexts
}

fn collect_function_contexts_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &mut Vec<String>,
    contexts: &mut Vec<(Span, FunctionContext)>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                if let Some(ref body) = func.body {
                    let mut params = HashMap::new();
                    for param in &func.sig.decl.inputs {
                        if let Some(type_info) =
                            type_info_from_ty(&param.ty, bit_widths, type_aliases, module_path)
                        {
                            if let Some(name) = pat_name(&param.pat) {
                                params.insert(name, type_info);
                            }
                        }
                    }
                    let return_type = match func.sig.decl.output {
                        FnRetTy::Ty(ref ty) => integer_type_from_ty(ty, module_path, type_aliases),
                        FnRetTy::Default(_) => None,
                    };
                    contexts.push((
                        body.span,
                        FunctionContext {
                            params,
                            return_type,
                        },
                    ));
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_function_contexts_from_items(
                    module_items,
                    bit_widths,
                    type_aliases,
                    module_path,
                    contexts,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

fn pat_name(pat: &Pat) -> Option<String> {
    match pat.kind {
        PatKind::Ident(_, ident, _) => Some(ident.name.to_string()),
        _ => None,
    }
}

fn qualified_item_name(module_path: &[String], name: &str) -> String {
    if module_path.is_empty() {
        name.to_string()
    } else {
        format!("{}::{}", module_path.join("::"), name)
    }
}

// -----------------------------------------------------------------------------
// Integer-cast helpers
// -----------------------------------------------------------------------------

#[derive(Clone, Copy, Debug)]
enum IntegerLiteralValue {
    NonNegative(u128),
    Negative(u128),
}

fn remove_redundant_literal_cast(
    expr: &mut P<Expr>,
    expected_type: IntegerType,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &[String],
) -> bool {
    let (inner_expr, cast_ty) = match expr.kind {
        ExprKind::Cast(ref inner_expr, ref cast_ty) => (inner_expr, cast_ty),
        ExprKind::Paren(ref paren_expr) => match paren_expr.kind {
            ExprKind::Cast(ref inner_expr, ref cast_ty) => (inner_expr, cast_ty),
            _ => return false,
        },
        _ => return false,
    };

    // T may itself be a type alias, possibly through multiple alias levels.
    let cast_type = match integer_type_from_ty(cast_ty, module_path, type_aliases) {
        Some(ty) => ty,
        None => return false,
    };

    let value = match unsuffixed_integer_literal_value(inner_expr) {
        Some(value) => value,
        None => return false,
    };

    // The literal must preserve its mathematical value both through the cast
    // being removed (T) and in the surrounding expected integer type (U).
    if !integer_value_fits_type(value, cast_type, bit_widths)
        || !integer_value_fits_type(value, expected_type, bit_widths)
    {
        return false;
    }

    *expr = inner_expr.clone();
    true
}

fn remove_redundant_literal_casts_for_expected_type(
    expr: &mut P<Expr>,
    expected_type: &TypeInfo,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &[String],
) -> bool {
    match expected_type {
        TypeInfo::Integer(expected_integer_type) => remove_redundant_literal_cast(
            expr,
            *expected_integer_type,
            bit_widths,
            type_aliases,
            module_path,
        ),
        TypeInfo::Array(element_type) => match expr.kind {
            ExprKind::Array(ref mut elements) => {
                let mut changed = false;
                for element in elements.iter_mut() {
                    changed |= remove_redundant_literal_casts_for_expected_type(
                        element,
                        element_type,
                        bit_widths,
                        type_aliases,
                        module_path,
                    );
                }
                changed
            }
            ExprKind::Repeat(ref mut element, _) => {
                remove_redundant_literal_casts_for_expected_type(
                    element,
                    element_type,
                    bit_widths,
                    type_aliases,
                    module_path,
                )
            }
            _ => false,
        },
        TypeInfo::Pointer(_)
        | TypeInfo::Reference(_)
        | TypeInfo::Slice(_)
        | TypeInfo::Struct(_)
        | TypeInfo::Unknown => false,
    }
}

fn type_info_from_ty(
    ty: &Ty,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &[String],
) -> Option<TypeInfo> {
    let mut resolving = HashSet::new();
    type_info_from_ty_inner(ty, bit_widths, type_aliases, module_path, &mut resolving)
}

fn type_info_from_ty_inner(
    ty: &Ty,
    bit_widths: &BitWidths,
    type_aliases: &HashMap<String, TypeAliasInfo>,
    module_path: &[String],
    resolving: &mut HashSet<String>,
) -> Option<TypeInfo> {
    if let Some(integer_type) = integer_type_from_ty(ty, module_path, type_aliases) {
        if let IntegerType::C(c_type) = integer_type {
            let _ = c_integer_width(c_type, bit_widths);
        }
        return Some(TypeInfo::Integer(integer_type));
    }

    match ty.kind {
        TyKind::Path(None, ref path) => {
            // A type alias can also resolve to an array, slice, or reference.
            // Track aliases across modules and reject recursive resolutions.
            if let Some(key) = resolve_path_in_map(path, module_path, type_aliases) {
                if !resolving.insert(key.clone()) {
                    return None;
                }
                let alias = type_aliases.get(&key)?;
                let result = type_info_from_ty_inner(
                    &alias.target,
                    bit_widths,
                    type_aliases,
                    &alias.module_path,
                    resolving,
                );
                resolving.remove(&key);
                if result.is_some() {
                    return result;
                }
            }
            Some(TypeInfo::Struct(TypePath::from_ast_path(
                path,
                module_path,
            )?))
        }
        TyKind::Ptr(ref mut_ty) => {
            let pointee = type_info_from_ty_inner(
                &mut_ty.ty,
                bit_widths,
                type_aliases,
                module_path,
                resolving,
            )
            .unwrap_or(TypeInfo::Unknown);
            Some(TypeInfo::Pointer(Box::new(pointee)))
        }
        TyKind::Rptr(_, ref mut_ty) => {
            let referred = type_info_from_ty_inner(
                &mut_ty.ty,
                bit_widths,
                type_aliases,
                module_path,
                resolving,
            )
            .unwrap_or(TypeInfo::Unknown);
            Some(TypeInfo::Reference(Box::new(referred)))
        }
        TyKind::Array(ref element_ty, _) => {
            let element = type_info_from_ty_inner(
                element_ty,
                bit_widths,
                type_aliases,
                module_path,
                resolving,
            )
            .unwrap_or(TypeInfo::Unknown);
            Some(TypeInfo::Array(Box::new(element)))
        }
        TyKind::Slice(ref element_ty) => {
            let element = type_info_from_ty_inner(
                element_ty,
                bit_widths,
                type_aliases,
                module_path,
                resolving,
            )
            .unwrap_or(TypeInfo::Unknown);
            Some(TypeInfo::Slice(Box::new(element)))
        }
        _ => None,
    }
}

fn integer_type_from_ty(
    ty: &Ty,
    module_path: &[String],
    type_aliases: &HashMap<String, TypeAliasInfo>,
) -> Option<IntegerType> {
    let mut resolving = HashSet::new();
    integer_type_from_ty_inner(ty, module_path, type_aliases, &mut resolving)
}

fn integer_type_from_ty_inner(
    ty: &Ty,
    module_path: &[String],
    type_aliases: &HashMap<String, TypeAliasInfo>,
    resolving: &mut HashSet<String>,
) -> Option<IntegerType> {
    let path = match ty.kind {
        TyKind::Path(None, ref path) => path,
        _ => return None,
    };

    if let Some(integer_type) = direct_integer_type_from_path(path) {
        return Some(integer_type);
    }

    let alias_key = resolve_path_in_map(path, module_path, type_aliases)?;
    if !resolving.insert(alias_key.clone()) {
        // Recursive aliases do not resolve to a primitive integer type.
        return None;
    }

    let alias = type_aliases.get(&alias_key)?;
    let result =
        integer_type_from_ty_inner(&alias.target, &alias.module_path, type_aliases, resolving);
    resolving.remove(&alias_key);
    result
}

fn direct_integer_type_from_path(path: &rustc_ast::Path) -> Option<IntegerType> {
    if let Some(name) = unqualified_type_name(path) {
        return match name {
            "i8" => Some(IntegerType::Primitive("i8")),
            "i16" => Some(IntegerType::Primitive("i16")),
            "i32" => Some(IntegerType::Primitive("i32")),
            "i64" => Some(IntegerType::Primitive("i64")),
            "i128" => Some(IntegerType::Primitive("i128")),
            "isize" => Some(IntegerType::Primitive("isize")),

            "u8" => Some(IntegerType::Primitive("u8")),
            "u16" => Some(IntegerType::Primitive("u16")),
            "u32" => Some(IntegerType::Primitive("u32")),
            "u64" => Some(IntegerType::Primitive("u64")),
            "u128" => Some(IntegerType::Primitive("u128")),
            "usize" => Some(IntegerType::Primitive("usize")),

            _ => None,
        };
    }

    if is_core_ffi_path(path) {
        let name = path.segments.last()?.ident.name.as_str();
        return match name {
            "c_schar" => Some(IntegerType::C(CIntegerType::SChar)),
            "c_uchar" => Some(IntegerType::C(CIntegerType::UChar)),

            "c_short" => Some(IntegerType::C(CIntegerType::Short)),
            "c_ushort" => Some(IntegerType::C(CIntegerType::UShort)),

            "c_int" => Some(IntegerType::C(CIntegerType::Int)),
            "c_uint" => Some(IntegerType::C(CIntegerType::UInt)),

            "c_long" => Some(IntegerType::C(CIntegerType::Long)),
            "c_ulong" => Some(IntegerType::C(CIntegerType::ULong)),

            "c_longlong" => Some(IntegerType::C(CIntegerType::LongLong)),
            "c_ulonglong" => Some(IntegerType::C(CIntegerType::ULongLong)),

            _ => None,
        };
    }

    None
}

fn unqualified_type_name(path: &rustc_ast::Path) -> Option<&str> {
    if path.is_global() || path.segments.len() != 1 {
        return None;
    }
    Some(path.segments[0].ident.name.as_str())
}

fn is_core_ffi_path(path: &rustc_ast::Path) -> bool {
    path.is_global()
        && path.segments.len() == 4
        && path.segments[1].ident.name.as_str() == "core"
        && path.segments[2].ident.name.as_str() == "ffi"
}

fn unsuffixed_integer_literal_value(expr: &Expr) -> Option<IntegerLiteralValue> {
    match expr.kind {
        ExprKind::Lit(ref lit) => {
            if !matches!(lit.kind, LitKind::Int(..)) {
                return None;
            }
            if lit.token_lit.suffix.is_some() {
                return None;
            }
            let value = parse_integer_literal(lit.token_lit.symbol.as_str().as_ref())?;
            Some(IntegerLiteralValue::NonNegative(value))
        }
        ExprKind::Unary(UnOp::Neg, ref inner_expr) => {
            let lit = match inner_expr.kind {
                ExprKind::Lit(ref lit) => lit,
                _ => return None,
            };
            if !matches!(lit.kind, LitKind::Int(..)) {
                return None;
            }
            if lit.token_lit.suffix.is_some() {
                return None;
            }
            let magnitude = parse_integer_literal(lit.token_lit.symbol.as_str().as_ref())?;
            Some(IntegerLiteralValue::Negative(magnitude))
        }
        _ => None,
    }
}

fn parse_integer_literal(raw: &str) -> Option<u128> {
    let without_underscores = raw.replace('_', "");
    if let Some(rest) = without_underscores.strip_prefix("0x") {
        u128::from_str_radix(rest, 16).ok()
    } else if let Some(rest) = without_underscores.strip_prefix("0o") {
        u128::from_str_radix(rest, 8).ok()
    } else if let Some(rest) = without_underscores.strip_prefix("0b") {
        u128::from_str_radix(rest, 2).ok()
    } else {
        u128::from_str_radix(&without_underscores, 10).ok()
    }
}

fn integer_value_fits_type(
    value: IntegerLiteralValue,
    ty: IntegerType,
    bit_widths: &BitWidths,
) -> bool {
    let (bits, signed) = integer_type_width_and_signedness(ty, bit_widths);
    match value {
        IntegerLiteralValue::NonNegative(value) => {
            if signed {
                value <= signed_max_for_bits(bits)
            } else {
                value <= unsigned_max_for_bits(bits)
            }
        }
        IntegerLiteralValue::Negative(magnitude) => {
            signed && magnitude <= signed_min_magnitude_for_bits(bits)
        }
    }
}

fn integer_type_width_and_signedness(ty: IntegerType, bit_widths: &BitWidths) -> (u32, bool) {
    match ty {
        IntegerType::Primitive(name) => match name {
            "i8" => (8, true),
            "i16" => (16, true),
            "i32" => (32, true),
            "i64" => (64, true),
            "i128" => (128, true),
            "isize" => (bit_widths.pointer_bits, true),

            "u8" => (8, false),
            "u16" => (16, false),
            "u32" => (32, false),
            "u64" => (64, false),
            "u128" => (128, false),
            "usize" => (bit_widths.pointer_bits, false),

            _ => unreachable!("unsupported primitive integer type: {}", name),
        },
        IntegerType::C(c_type) => (
            c_integer_width(c_type, bit_widths),
            matches!(
                c_type,
                CIntegerType::SChar
                    | CIntegerType::Short
                    | CIntegerType::Int
                    | CIntegerType::Long
                    | CIntegerType::LongLong
            ),
        ),
    }
}

fn c_integer_width(c_type: CIntegerType, bit_widths: &BitWidths) -> u32 {
    match c_type {
        CIntegerType::SChar | CIntegerType::UChar => bit_widths.c_char_bits,
        CIntegerType::Short | CIntegerType::UShort => bit_widths.c_short_bits,
        CIntegerType::Int | CIntegerType::UInt => bit_widths.c_int_bits,
        CIntegerType::Long | CIntegerType::ULong => bit_widths.c_long_bits,
        CIntegerType::LongLong | CIntegerType::ULongLong => bit_widths.c_longlong_bits,
    }
}

fn signed_max_for_bits(bits: u32) -> u128 {
    match bits {
        8 => i8::MAX as u128,
        16 => i16::MAX as u128,
        32 => i32::MAX as u128,
        64 => i64::MAX as u128,
        128 => i128::MAX as u128,
        _ => unreachable!("unsupported signed integer width: {}", bits),
    }
}

fn signed_min_magnitude_for_bits(bits: u32) -> u128 {
    match bits {
        8 => 1u128 << 7,
        16 => 1u128 << 15,
        32 => 1u128 << 31,
        64 => 1u128 << 63,
        128 => 1u128 << 127,
        _ => unreachable!("unsupported signed integer width: {}", bits),
    }
}

fn unsigned_max_for_bits(bits: u32) -> u128 {
    match bits {
        8 => u8::MAX as u128,
        16 => u16::MAX as u128,
        32 => u32::MAX as u128,
        64 => u64::MAX as u128,
        128 => u128::MAX,
        _ => unreachable!("unsupported unsigned integer width: {}", bits),
    }
}
