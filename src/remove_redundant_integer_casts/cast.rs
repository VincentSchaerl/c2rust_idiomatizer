use super::{BitWidths, CIntegerType, IntegerType, TypeInfo, TypePath};

use rustc_ast::ptr::P;
use rustc_ast::{Expr, ExprKind, LitKind, Ty, TyKind, UnOp};

#[derive(Clone, Copy, Debug)]
enum IntegerLiteralValue {
    NonNegative(u128),
    Negative(u128),
}

pub(super) fn remove_cast_if_redundant(
    expr: &mut P<Expr>,
    expected_type: IntegerType,
    bit_widths: &BitWidths,
) {
    let (inner_expr, cast_ty) = match expr.kind {
        ExprKind::Cast(ref inner_expr, ref cast_ty) => (inner_expr, cast_ty),
        ExprKind::Paren(ref paren_expr) => match paren_expr.kind {
            ExprKind::Cast(ref inner_expr, ref cast_ty) => (inner_expr, cast_ty),
            _ => return,
        },
        _ => return,
    };
    let cast_type = match integer_type_from_ty(cast_ty) {
        Some(ty) => ty,
        None => return,
    };
    if expected_type != cast_type {
        return;
    }
    let value = match unsuffixed_integer_literal_value(inner_expr) {
        Some(value) => value,
        None => return,
    };
    if !integer_value_fits_type(value, cast_type, bit_widths) {
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
        TypeInfo::Integer(expected_integer_type) => {
            remove_cast_if_redundant(expr, *expected_integer_type, bit_widths);
        }
        TypeInfo::Array(element_type) => match expr.kind {
            ExprKind::Array(ref mut elements) => {
                for element in elements.iter_mut() {
                    remove_casts_if_redundant_for_expected_type(
                        element,
                        element_type,
                        bit_widths,
                    );
                }
            }
            ExprKind::Repeat(ref mut element, _) => {
                remove_casts_if_redundant_for_expected_type(
                    element,
                    element_type,
                    bit_widths,
                );
            }
            _ => {}
        },
        TypeInfo::Pointer(_) | TypeInfo::Struct(_) => {}
    }
}

pub(super) fn type_info_from_ty(
    ty: &Ty,
    bit_widths: &BitWidths,
    module_path: &[String],
) -> Option<TypeInfo> {
    if let Some(integer_type) = integer_type_from_ty(ty) {
        if let IntegerType::C(c_type) = integer_type {
            let _ = c_integer_width(c_type, bit_widths);
        }
        return Some(TypeInfo::Integer(integer_type));
    }
    match ty.kind {
        TyKind::Path(None, ref path) => {
            let struct_path = TypePath::from_ast_path(path, module_path)?;
            Some(TypeInfo::Struct(struct_path))
        }
        TyKind::Ptr(ref mut_ty) => {
            let pointee_type = type_info_from_ty(&mut_ty.ty, bit_widths, module_path)?;
            Some(TypeInfo::Pointer(Box::new(pointee_type)))
        }
        TyKind::Array(ref element_ty, _) => {
            let element_type = type_info_from_ty(element_ty, bit_widths, module_path)?;
            Some(TypeInfo::Array(Box::new(element_type)))
        }
        _ => None,
    }
}

pub(super) fn integer_type_from_ty(ty: &Ty) -> Option<IntegerType> {
    let path = match ty.kind {
        TyKind::Path(None, ref path) => path,
        _ => return None,
    };
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
