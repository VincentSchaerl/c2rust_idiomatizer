use super::{BitWidths, TypeInfo};

use rustc_ast::ptr::P;
use rustc_ast::{Expr, ExprKind, LitKind, Ty, TyKind, UnOp};

#[derive(Clone, Copy, Debug)]
enum IntegerLiteralValue {
    NonNegative(u128),
    Negative(u128),
}

pub(super) fn remove_cast_if_redundant(
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

pub(super) fn remove_casts_if_redundant_for_expected_type(
    expr: &mut P<Expr>,
    expected_type: &TypeInfo,
    bit_widths: &BitWidths,
) {
    match expected_type {
        TypeInfo::Integer(expected_type_name) => {
            remove_cast_if_redundant(expr, *expected_type_name, bit_widths);
        }
        TypeInfo::Array(element_type) => match expr.kind {
            ExprKind::Array(ref mut elements) => {
                for element in elements.iter_mut() {
                    remove_casts_if_redundant_for_expected_type(element, element_type, bit_widths);
                }
            }
            ExprKind::Repeat(ref mut element, _) => {
                remove_casts_if_redundant_for_expected_type(element, element_type, bit_widths);
            }
            _ => {}
        },
        TypeInfo::Pointer(_) | TypeInfo::Struct(_) => {}
    }
}

pub(super) fn type_info_from_ty(ty: &Ty, bit_widths: &BitWidths) -> Option<TypeInfo> {
    if let Some(type_name) = integer_type_name(ty, bit_widths) {
        return Some(TypeInfo::Integer(type_name));
    }
    match ty.kind {
        TyKind::Path(None, ref path) => {
            let struct_name = path.segments.last()?.ident.name.to_string();
            Some(TypeInfo::Struct(struct_name))
        }
        TyKind::Ptr(ref mut_ty) => {
            let pointee_type = type_info_from_ty(&mut_ty.ty, bit_widths)?;
            Some(TypeInfo::Pointer(Box::new(pointee_type)))
        }
        TyKind::Array(ref element_ty, _) => {
            let element_type = type_info_from_ty(element_ty, bit_widths)?;
            Some(TypeInfo::Array(Box::new(element_type)))
        }
        _ => None,
    }
}

pub(super) fn integer_type_name(ty: &Ty, bit_widths: &BitWidths) -> Option<&'static str> {
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

fn integer_value_fits_type(value: IntegerLiteralValue, ty: &str, pointer_bits: u32) -> bool {
    match value {
        IntegerLiteralValue::NonNegative(value) => {
            nonnegative_integer_value_fits_type(value, ty, pointer_bits)
        }
        IntegerLiteralValue::Negative(magnitude) => {
            negative_integer_value_fits_type(magnitude, ty, pointer_bits)
        }
    }
}

fn nonnegative_integer_value_fits_type(value: u128, ty: &str, pointer_bits: u32) -> bool {
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

fn negative_integer_value_fits_type(magnitude: u128, ty: &str, pointer_bits: u32) -> bool {
    match ty {
        "i8" => magnitude <= signed_min_magnitude_for_bits(8),
        "i16" => magnitude <= signed_min_magnitude_for_bits(16),
        "i32" => magnitude <= signed_min_magnitude_for_bits(32),
        "i64" => magnitude <= signed_min_magnitude_for_bits(64),
        "i128" => magnitude <= signed_min_magnitude_for_bits(128),

        "isize" => magnitude <= signed_min_magnitude_for_bits(pointer_bits),

        "u8" | "u16" | "u32" | "u64" | "u128" | "usize" => false,

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

fn signed_min_magnitude_for_bits(bits: u32) -> u128 {
    match bits {
        8 => 1u128 << 7,
        16 => 1u128 << 15,
        32 => 1u128 << 31,
        64 => 1u128 << 63,
        128 => 1u128 << 127,
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
