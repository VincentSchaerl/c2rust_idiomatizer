use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{Crate, Expr, ExprKind, LitKind, Local, LocalKind, Ty, TyKind};

struct RemoveRedundantIntegerCasts {
    pointer_bits: u32,
}

pub fn register_remove_redundant_integer_casts(reg: &mut Registry) {
    reg.register("remove_redundant_integer_casts", |_args| {
        Box::new(DriverCommand::new(Phase::Phase1, |st, cx| {
            let pointer_bits = cx.session().target.pointer_width;

            remove_redundant_integer_casts(&mut st.krate_mut(), pointer_bits);
        }))
    });
}

fn remove_redundant_integer_casts(krate: &mut Crate, pointer_bits: u32) {
    let mut visitor = RemoveRedundantIntegerCasts { pointer_bits };
    visitor.visit_crate(krate);
}

impl MutVisitor for RemoveRedundantIntegerCasts {
    fn visit_local(&mut self, local: &mut P<Local>) {
        let expected_type_name = match local.ty {
            Some(ref expected_ty) => primitive_integer_type_name(expected_ty),
            None => None,
        };
        if let Some(expected_type_name) = expected_type_name {
            match local.kind {
                LocalKind::Init(ref mut init) => {
                    remove_cast_if_redundant(init, expected_type_name, self.pointer_bits);
                }
                LocalKind::InitElse(ref mut init, _) => {
                    remove_cast_if_redundant(init, expected_type_name, self.pointer_bits);
                }
                LocalKind::Decl => {}
            }
        }
        mut_visit::noop_visit_local(local, self);
    }
}

fn remove_cast_if_redundant(
    expr: &mut P<Expr>,
    expected_type_name: &'static str,
    pointer_bits: u32,
) {
    let (inner_expr, cast_ty) = match expr.kind {
        ExprKind::Cast(ref inner_expr, ref cast_ty) => (inner_expr, cast_ty),
        _ => return,
    };
    let cast_type_name = match primitive_integer_type_name(cast_ty) {
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
    if !integer_value_fits_type(value, cast_type_name, pointer_bits) {
        return;
    }
    *expr = inner_expr.clone();
}

fn primitive_integer_type_name(ty: &Ty) -> Option<&'static str> {
    if let TyKind::Path(None, ref path) = ty.kind {
        if path.segments.len() != 1 {
            return None;
        }
        let name = path.segments[0].ident.name.as_str();
        match &*name {
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

            _ => None,
        }
    } else {
        None
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
