use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{
    Block, Crate, Expr, ExprKind, FnRetTy, Item, ItemKind, LitKind, Local, LocalKind, ModKind, Pat,
    PatKind, Ty, TyKind,
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
struct FunctionContext {
    params: HashMap<String, &'static str>,
    return_type: Option<&'static str>,
}

struct RemoveRedundantIntegerCasts {
    bit_widths: BitWidths,
    global_vars: HashMap<String, &'static str>,
    function_param_types: HashMap<String, Vec<Option<&'static str>>>,
    function_contexts: Vec<(Span, FunctionContext)>,
    local_scopes: Vec<HashMap<String, &'static str>>,
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
    let global_vars = collect_global_vars(krate, &bit_widths);
    let function_param_types = collect_function_param_types(krate, &bit_widths);
    let function_contexts = collect_function_contexts(krate, &bit_widths);
    let mut visitor = RemoveRedundantIntegerCasts {
        bit_widths,
        global_vars,
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
        let expected_type_name = match local.ty {
            Some(ref expected_ty) => integer_type_name(expected_ty, &self.bit_widths),
            None => None,
        };
        if let Some(expected_type_name) = expected_type_name {
            if let Some(name) = local_name(local) {
                self.insert_local_var(name, expected_type_name);
            }
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
        mut_visit::noop_visit_local(local, self);
    }

    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        match expr.kind {
            ExprKind::Assign(ref lhs, ref mut rhs, _) => {
                if let Some(var_name) = expr_path_last_segment(lhs) {
                    if let Some(expected_type_name) = self.lookup_var_type(&var_name) {
                        remove_cast_if_redundant(rhs, expected_type_name, &self.bit_widths);
                    }
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
                        for (arg, expected_type_name) in
                            args.iter_mut().zip(param_types.into_iter())
                        {
                            if let Some(expected_type_name) = expected_type_name {
                                remove_cast_if_redundant(arg, expected_type_name, &self.bit_widths);
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
    fn insert_local_var(&mut self, name: String, type_name: &'static str) {
        if let Some(scope) = self.local_scopes.last_mut() {
            scope.insert(name, type_name);
        }
    }

    fn lookup_var_type(&self, name: &str) -> Option<&'static str> {
        for scope in self.local_scopes.iter().rev() {
            if let Some(type_name) = scope.get(name) {
                return Some(*type_name);
            }
        }

        self.global_vars.get(name).copied()
    }

    fn function_context_for_block(&self, span: Span) -> Option<&FunctionContext> {
        self.function_contexts
            .iter()
            .find(|(body_span, _)| *body_span == span)
            .map(|(_, context)| context)
    }
}

fn collect_global_vars(krate: &Crate, bit_widths: &BitWidths) -> HashMap<String, &'static str> {
    let mut vars = HashMap::new();
    collect_global_vars_from_items(&krate.items, bit_widths, &mut vars);
    vars
}

fn collect_global_vars_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    vars: &mut HashMap<String, &'static str>,
) {
    for item in items {
        match item.kind {
            ItemKind::Static(ref ty, ..) => {
                if let Some(type_name) = integer_type_name(ty, bit_widths) {
                    vars.insert(item.ident.name.to_string(), type_name);
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                collect_global_vars_from_items(module_items, bit_widths, vars);
            }
            _ => {}
        }
    }
}

fn collect_function_param_types(
    krate: &Crate,
    bit_widths: &BitWidths,
) -> HashMap<String, Vec<Option<&'static str>>> {
    let mut functions = HashMap::new();
    collect_function_param_types_from_items(&krate.items, bit_widths, &mut functions);
    functions
}

fn collect_function_param_types_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    functions: &mut HashMap<String, Vec<Option<&'static str>>>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                let mut params = Vec::new();
                for param in &func.sig.decl.inputs {
                    params.push(integer_type_name(&param.ty, bit_widths));
                }
                functions.insert(item.ident.name.to_string(), params);
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                collect_function_param_types_from_items(module_items, bit_widths, functions);
            }
            _ => {}
        }
    }
}

fn collect_function_contexts(
    krate: &Crate,
    bit_widths: &BitWidths,
) -> Vec<(Span, FunctionContext)> {
    let mut contexts = Vec::new();
    collect_function_contexts_from_items(&krate.items, bit_widths, &mut contexts);
    contexts
}

fn collect_function_contexts_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    contexts: &mut Vec<(Span, FunctionContext)>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                if let Some(ref body) = func.body {
                    let mut params = HashMap::new();
                    for param in &func.sig.decl.inputs {
                        if let Some(type_name) = integer_type_name(&param.ty, bit_widths) {
                            if let Some(name) = pat_name(&param.pat) {
                                params.insert(name, type_name);
                            }
                        }
                    }
                    let return_type = match func.sig.decl.output {
                        FnRetTy::Ty(ref ty) => integer_type_name(ty, bit_widths),
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
                collect_function_contexts_from_items(module_items, bit_widths, contexts);
            }
            _ => {}
        }
    }
}

fn local_name(local: &Local) -> Option<String> {
    pat_name(&local.pat)
}

fn pat_name(pat: &Pat) -> Option<String> {
    match pat.kind {
        PatKind::Ident(_, ident, _) => Some(ident.name.to_string()),
        _ => None,
    }
}

fn expr_path_last_segment(expr: &Expr) -> Option<String> {
    match expr.kind {
        ExprKind::Path(_, ref path) => path.segments.last().map(|s| s.ident.name.to_string()),
        _ => None,
    }
}

fn remove_cast_if_redundant(
    expr: &mut P<Expr>,
    expected_type_name: &'static str,
    bit_widths: &BitWidths,
) {
    let (inner_expr, cast_ty) = match expr.kind {
        ExprKind::Cast(ref inner_expr, ref cast_ty) => (inner_expr, cast_ty),
        _ => return,
    };
    let cast_type_name = match integer_type_name(cast_ty, bit_widths) {
        Some(name) => name,
        None => return,
    };
    if expected_type_name != cast_type_name {
        return;
    }
    let value = match unsuffixed_integer_literal_value(inner_expr) {
        Some(value) => value,
        None => return,
    };
    if !integer_value_fits_type(value, cast_type_name, bit_widths.pointer_bits) {
        return;
    }
    *expr = inner_expr.clone();
}

fn integer_type_name(ty: &Ty, bit_widths: &BitWidths) -> Option<&'static str> {
    let last = match ty.kind {
        TyKind::Path(None, ref path) => path.segments.last().map(|s| s.ident.name.to_string())?,
        _ => return None,
    };
    match &*last {
        "i8" => Some("i8"),
        "i16" => Some("i16"),
        "i32" => Some("i32"),
        "i64" => Some("i64"),
        "i128" => Some("i128"),
        "isize" => Some("isize"),

        "u8" => Some("u8"),
        "u16" => Some("u16"),
        "u32" => Some("u32"),
        "u64" => Some("u64"),
        "u128" => Some("u128"),
        "usize" => Some("usize"),

        "c_schar" => Some("i8"),
        "c_uchar" => Some("u8"),

        "c_short" => Some(rust_int(bit_widths.c_short_bits, true)),
        "c_ushort" => Some(rust_int(bit_widths.c_short_bits, false)),

        "c_int" => Some(rust_int(bit_widths.c_int_bits, true)),
        "c_uint" => Some(rust_int(bit_widths.c_int_bits, false)),

        "c_long" => Some(rust_int(bit_widths.c_long_bits, true)),
        "c_ulong" => Some(rust_int(bit_widths.c_long_bits, false)),

        "c_longlong" => Some(rust_int(bit_widths.c_longlong_bits, true)),
        "c_ulonglong" => Some(rust_int(bit_widths.c_longlong_bits, false)),

        _ => None,
    }
}

fn unsuffixed_integer_literal_value(expr: &Expr) -> Option<u128> {
    let lit = match &expr.kind {
        ExprKind::Lit(lit) => lit,
        _ => return None,
    };
    if !matches!(lit.kind, LitKind::Int(..)) {
        return None;
    }
    if lit.token_lit.suffix.is_some() {
        return None;
    }
    parse_integer_literal(lit.token_lit.symbol.as_str().as_ref())
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

fn integer_value_fits_type(value: u128, ty: &str, pointer_bits: u32) -> bool {
    match ty {
        "i8" => value <= i8::MAX as u128,
        "i16" => value <= i16::MAX as u128,
        "i32" => value <= i32::MAX as u128,
        "i64" => value <= i64::MAX as u128,
        "i128" => value <= i128::MAX as u128,

        "u8" => value <= u8::MAX as u128,
        "u16" => value <= u16::MAX as u128,
        "u32" => value <= u32::MAX as u128,
        "u64" => value <= u64::MAX as u128,
        "u128" => true,

        "isize" => value <= signed_max_for_bits(pointer_bits),
        "usize" => value <= unsigned_max_for_bits(pointer_bits),

        _ => false,
    }
}

fn signed_max_for_bits(bits: u32) -> u128 {
    match bits {
        8 => i8::MAX as u128,
        16 => i16::MAX as u128,
        32 => i32::MAX as u128,
        64 => i64::MAX as u128,
        128 => i128::MAX as u128,
        _ => 0,
    }
}

fn unsigned_max_for_bits(bits: u32) -> u128 {
    match bits {
        8 => u8::MAX as u128,
        16 => u16::MAX as u128,
        32 => u32::MAX as u128,
        64 => u64::MAX as u128,
        128 => u128::MAX,
        _ => 0,
    }
}

fn rust_int(bits: u32, signed: bool) -> &'static str {
    match (bits, signed) {
        (8, true) => "i8",
        (8, false) => "u8",
        (16, true) => "i16",
        (16, false) => "u16",
        (32, true) => "i32",
        (32, false) => "u32",
        (64, true) => "i64",
        (64, false) => "u64",
        (128, true) => "i128",
        (128, false) => "u128",
        _ => panic!("Unsupported integer width: {}", bits),
    }
}
