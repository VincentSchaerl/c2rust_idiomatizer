mod analysis;
mod cast;

use analysis::{
    collect_function_contexts, collect_function_param_types, collect_global_vars,
    collect_struct_fields, pat_name,
};
use cast::{remove_cast_if_redundant, type_info_from_ty};

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{Block, Crate, Expr, ExprKind, Local, LocalKind};
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
    fn visit_block(&mut self, block: &mut P<Block>) {
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
            if let TypeInfo::Integer(expected_type_name) = expected_type {
                match local.kind {
                    LocalKind::Init(ref mut init) => {
                        remove_cast_if_redundant(init, expected_type_name, &self.bit_widths);
                    }
                    LocalKind::InitElse(ref mut init, _) => {
                        remove_cast_if_redundant(init, expected_type_name, &self.bit_widths);
                    }
                    LocalKind::Decl => {}
                }
            }
        }
        mut_visit::noop_visit_local(local, self);
    }

    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        match expr.kind {
            ExprKind::Assign(ref lhs, ref mut rhs, _) => {
                if let Some(expected_type_name) = self.expected_integer_type_for_assignment_lhs(lhs) {
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
                                remove_cast_if_redundant(
                                    arg,
                                    expected_type_name,
                                    &self.bit_widths,
                                );
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
            _ => {}
        }
        mut_visit::noop_visit_expr(expr, self);
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

    fn expected_integer_type_for_assignment_lhs(&self, lhs: &Expr) -> Option<&'static str> {
        if let Some(var_name) = expr_path_last_segment(lhs) {
            if let Some(TypeInfo::Integer(type_name)) = self.lookup_var_type(&var_name) {
                return Some(type_name);
            }
        }
        if let Some((root_name, fields)) = expr_field_chain(lhs) {
            return self.resolve_field_chain_to_integer_type(&root_name, &fields);
        }
        None
    }

    fn resolve_field_chain_to_integer_type(
        &self,
        root_name: &str,
        fields: &[String],
    ) -> Option<&'static str> {
        let mut current_type = self.lookup_var_type(root_name)?;
        for field_name in fields {
            let struct_name = match current_type {
                TypeInfo::Struct(ref struct_name) => struct_name.clone(),
                TypeInfo::Integer(_) => return None,
            };
            let field_types = self.struct_fields.get(&struct_name)?;
            current_type = field_types.get(field_name)?.clone();
        }
        match current_type {
            TypeInfo::Integer(type_name) => Some(type_name),
            TypeInfo::Struct(_) => None,
        }
    }
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

fn expr_field_chain(expr: &Expr) -> Option<(String, Vec<String>)> {
    match expr.kind {
        ExprKind::Field(ref base, ident) => {
            let (root, mut fields) = expr_field_chain(base)?;
            fields.push(ident.name.to_string());
            Some((root, fields))
        }
        ExprKind::Path(_, ref path) => {
            let root = path.segments.last()?.ident.name.to_string();
            Some((root, Vec::new()))
        }
        _ => None,
    }
}
