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
    c_short_bits: u32,
    c_int_bits: u32,
    c_long_bits: u32,
    c_longlong_bits: u32,
    pointer_bits: u32,
}

#[derive(Clone, Debug)]
enum TypeInfo {
    Integer(&'static str),
    Struct(String),
    Pointer(Box<TypeInfo>),
    Array(Box<TypeInfo>),
}

#[derive(Clone, Debug)]
struct FunctionContext {
    params: HashMap<String, TypeInfo>,
    return_type: Option<&'static str>,
}

struct RemoveRedundantIntegerCasts {
    bit_widths: BitWidths,
    global_vars: HashMap<String, TypeInfo>,
    struct_fields: HashMap<String, HashMap<String, TypeInfo>>,
    function_param_types: HashMap<String, Vec<Option<TypeInfo>>>,
    function_contexts: Vec<(Span, FunctionContext)>,
    local_scopes: Vec<HashMap<String, TypeInfo>>,
    current_return_types: Vec<Option<&'static str>>,
}

pub fn register_remove_redundant_integer_casts(reg: &mut Registry) {
    reg.register("remove_redundant_integer_casts", |_args| {
        Box::new(DriverCommand::new(Phase::Phase1, |st, cx| {
            let target = &cx.session().target;
            let c_int_bits = target.c_int_width.parse::<u32>().unwrap_or(32);
            let pointer_bits = target.pointer_width;
            let windows_llp64 = target.os == "windows" || target.env == "msvc";
            let c_long_bits = if windows_llp64 { 32 } else { pointer_bits };
            let bit_widths = BitWidths {
                c_short_bits: 16,
                c_int_bits,
                c_long_bits,
                c_longlong_bits: 64,
                pointer_bits,
            };
            remove_redundant_integer_casts(&mut st.krate_mut(), bit_widths);
        }))
    });
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
    };
    visitor.visit_crate(krate);
}

impl MutVisitor for RemoveRedundantIntegerCasts {
    fn visit_crate(&mut self, krate: &mut Crate) {
        remove_redundant_casts_in_item_initializers(&mut krate.items, &self.bit_widths);
        mut_visit::noop_visit_crate(krate, self);
    }

    fn visit_block(&mut self, block: &mut P<Block>) {
        for stmt in block.stmts.iter_mut() {
            if let StmtKind::Item(ref mut item) = stmt.kind {
                remove_redundant_casts_in_item_initializer(item, &self.bit_widths);
            }
        }
        let function_context = self.function_context_for_block(block.span).cloned();
        if let Some(context) = function_context {
            self.local_scopes.push(context.params);
            self.current_return_types.push(context.return_type);
            mut_visit::noop_visit_block(block, self);
            self.current_return_types.pop();
            self.local_scopes.pop();
        } else {
            self.local_scopes.push(HashMap::new());
            mut_visit::noop_visit_block(block, self);
            self.local_scopes.pop();
        }
    }

    fn visit_local(&mut self, local: &mut P<Local>) {
        let expected_type = match local.ty {
            Some(ref expected_ty) => type_info_from_ty(expected_ty, &self.bit_widths),
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
                if let Some(TypeInfo::Integer(expected_type_name)) =
                    self.expected_type_for_assignment_lhs(lhs)
                {
                    remove_cast_if_redundant(rhs, expected_type_name, &self.bit_widths);
                }
            }
            ExprKind::Ret(Some(ref mut ret_expr)) => {
                if let Some(Some(expected_type_name)) = self.current_return_types.last() {
                    remove_cast_if_redundant(ret_expr, *expected_type_name, &self.bit_widths);
                }
            }
            ExprKind::Call(ref callee, ref mut args) => {
                if let Some(function_name) = expr_path_last_segment(callee) {
                    if let Some(param_types) =
                        self.function_param_types.get(&function_name).cloned()
                    {
                        for (arg, expected_type) in args.iter_mut().zip(param_types.into_iter()) {
                            if let Some(TypeInfo::Integer(expected_type_name)) = expected_type {
                                remove_cast_if_redundant(arg, expected_type_name, &self.bit_widths);
                            }
                        }
                    }
                }
            }
            ExprKind::Struct(ref mut struct_expr) => {
                let struct_name = struct_expr
                    .path
                    .segments
                    .last()
                    .map(|segment| segment.ident.name.to_string());
                if let Some(struct_name) = struct_name {
                    if let Some(field_types) = self.struct_fields.get(&struct_name).cloned() {
                        for field in struct_expr.fields.iter_mut() {
                            let field_name = field.ident.name.to_string();
                            if let Some(TypeInfo::Integer(expected_type_name)) =
                                field_types.get(&field_name)
                            {
                                remove_cast_if_redundant(
                                    &mut field.expr,
                                    *expected_type_name,
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

fn remove_redundant_casts_in_item_initializers(items: &mut Vec<P<Item>>, bit_widths: &BitWidths) {
    for item in items.iter_mut() {
        remove_redundant_casts_in_item_initializer(item, bit_widths);
    }
}

fn remove_redundant_casts_in_item_initializer(item: &mut P<Item>, bit_widths: &BitWidths) {
    match item.kind {
        ItemKind::Static(ref ty, _, ref mut init) => {
            if let Some(ref mut init_expr) = *init {
                if let Some(expected_type) = type_info_from_ty(ty, bit_widths) {
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
                if let Some(expected_type) = type_info_from_ty(ty, bit_widths) {
                    remove_casts_if_redundant_for_expected_type(
                        init_expr,
                        &expected_type,
                        bit_widths,
                    );
                }
            }
        }
        ItemKind::Mod(_, ModKind::Loaded(ref mut module_items, ..)) => {
            remove_redundant_casts_in_item_initializers(module_items, bit_widths);
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
    fn lookup_var_type(&self, name: &str) -> Option<TypeInfo> {
        for scope in self.local_scopes.iter().rev() {
            if let Some(type_info) = scope.get(name) {
                return Some(type_info.clone());
            }
        }
        self.global_vars.get(name).cloned()
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
        if let Some(TypeInfo::Integer(expected_type_name)) = right_type {
            remove_cast_if_redundant(left, expected_type_name, &self.bit_widths);
        }
        if let Some(TypeInfo::Integer(expected_type_name)) = left_type {
            remove_cast_if_redundant(right, expected_type_name, &self.bit_widths);
        }
    }

    fn type_info_for_expr(&self, expr: &Expr) -> Option<TypeInfo> {
        match expr.kind {
            ExprKind::Path(_, ref path) => {
                let name = path.segments.last()?.ident.name.to_string();
                self.lookup_var_type(&name)
            }
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
                let method_name = segment.ident.name.to_string();
                if method_name == "offset" {
                    return self.type_info_for_expr(receiver);
                }
                None
            }
            ExprKind::Paren(ref inner) => self.type_info_for_expr(inner),
            _ => None,
        }
    }

    fn field_type(&self, base_type: TypeInfo, field_name: &str) -> Option<TypeInfo> {
        let struct_name = match base_type {
            TypeInfo::Struct(struct_name) => struct_name,
            _ => return None,
        };
        let field_types = self.struct_fields.get(&struct_name)?;
        field_types.get(field_name).cloned()
    }
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

fn expr_path_last_segment(expr: &Expr) -> Option<String> {
    match expr.kind {
        ExprKind::Path(_, ref path) => path.segments.last().map(|s| s.ident.name.to_string()),
        _ => None,
    }
}
