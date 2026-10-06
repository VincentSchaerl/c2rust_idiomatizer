mod initial_transformation;
mod pass_arrays_as_slices;

use initial_transformation::initial_transformation;
use pass_arrays_as_slices::pass_arrays_as_slices;

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::{Expr, ExprKind, LitIntType, LitKind, Ty, TyKind, UnOp};

#[derive(Clone, Copy, Debug)]
pub(super) struct IdiomatizeLoopsOptions {
    pub pass_arrays_as_slices: bool,
    pub transform_public_functions: bool,
    pub index_type_to_usize: bool,
    pub index_based_to_iterator_based: bool,
}

impl Default for IdiomatizeLoopsOptions {
    fn default() -> Self {
        Self {
            pass_arrays_as_slices: true,
            transform_public_functions: true,
            index_type_to_usize: true,
            index_based_to_iterator_based: true,
        }
    }
}

#[derive(Clone, Copy, Debug)]
pub(super) struct TargetInfo {
    pub pointer_width: u32,
    pub c_char_width: u32,
    pub c_short_width: u32,
    pub c_int_width: u32,
    pub c_long_width: u32,
    pub c_longlong_width: u32,
}

pub fn register_idiomatize_loops(reg: &mut Registry) {
    reg.register("idiomatize_loops", |args| {
        let options = parse_options(args);
        validate_options(options);

        // The initial transformation deliberately runs in Phase 3: its stability
        // checks need rustc's type-checking results/adjustments (in particular
        // auto-borrows)
        // and ExprUseVisitor.
        Box::new(DriverCommand::new(Phase::Phase3, move |st, cx| {
            let target = &cx.session().target;
            let c_int_width = target
                .c_int_width
                .parse::<u32>()
                .expect("Invalid rustc target c_int_width");
            let c_long_width = if target.os == "windows" || target.env == "msvc" {
                32
            } else {
                target.pointer_width
            };

            let target_info = TargetInfo {
                pointer_width: target.pointer_width,
                c_char_width: 8,
                c_short_width: 16,
                c_int_width,
                c_long_width,
                c_longlong_width: 64,
            };

            initial_transformation(st, cx, target_info);
            if options.pass_arrays_as_slices {
                pass_arrays_as_slices(
                    st,
                    cx,
                    target_info,
                    options.transform_public_functions,
                );
            }
        }))
    });
}

fn parse_options(args: &[String]) -> IdiomatizeLoopsOptions {
    let mut options = IdiomatizeLoopsOptions::default();
    let mut index_based_to_iterator_based_explicit = false;

    for arg in args {
        if let Some(value) = arg.strip_prefix("pass-arrays-as-slices=") {
            options.pass_arrays_as_slices = parse_bool_arg("pass-arrays-as-slices", value);
        } else if let Some(value) = arg.strip_prefix("transform-public-functions=") {
            options.transform_public_functions =
                parse_bool_arg("transform-public-functions", value);
        } else if let Some(value) = arg.strip_prefix("index-type-to-usize=") {
            options.index_type_to_usize = parse_bool_arg("index-type-to-usize", value);
        } else if let Some(value) = arg.strip_prefix("index-based-to-iterator-based=") {
            options.index_based_to_iterator_based =
                parse_bool_arg("index-based-to-iterator-based", value);
            index_based_to_iterator_based_explicit = true;
        } else {
            panic!("Unknown argument for idiomatize_loops: {}", arg);
        }
    }

    if !index_based_to_iterator_based_explicit && !options.index_type_to_usize {
        options.index_based_to_iterator_based = false;
    }

    options
}

fn validate_options(options: IdiomatizeLoopsOptions) {
    if options.transform_public_functions && !options.pass_arrays_as_slices {
        panic!(
            concat!(
                "Invalid arguments for idiomatize_loops: ",
                "transform-public-functions=true requires ",
                "pass-arrays-as-slices=true. Public-function transformation ",
                "is part of pass-arrays-as-slices."
            )
        );
    }

    if options.index_based_to_iterator_based && !options.index_type_to_usize {
        panic!(
            concat!(
                "Invalid arguments for idiomatize_loops: ",
                "index-based-to-iterator-based=true requires ",
                "index-type-to-usize=true. Iterator-based loop conversion ",
                "depends on converting index types to usize first."
            )
        );
    }
}

fn parse_bool_arg(name: &str, value: &str) -> bool {
    match value {
        "true" => true,
        "false" => false,
        _ => panic!(
            "Invalid value for {}: expected true or false, got {}",
            name, value
        ),
    }
}

// -----------------------------------------------------------------------------
// Shared integer helpers used by the loop passes
// -----------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(super) enum CIntegerType {
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
pub(super) enum IntegerType {
    Primitive(&'static str),
    C(CIntegerType),
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub(super) enum IntValue {
    NonNegative(u128),
    Negative(u128),
}

impl IntValue {
    fn is_zero(self) -> bool {
        matches!(self, IntValue::NonNegative(0))
    }

    fn is_positive(self) -> bool {
        matches!(self, IntValue::NonNegative(value) if value > 0)
    }

    fn is_negative(self) -> bool {
        matches!(self, IntValue::Negative(value) if value > 0)
    }

    fn abs_magnitude(self) -> u128 {
        match self {
            IntValue::NonNegative(value) | IntValue::Negative(value) => value,
        }
    }

    fn decimal_string(self) -> String {
        match self {
            IntValue::NonNegative(value) => value.to_string(),
            IntValue::Negative(value) => format!("-{}", value),
        }
    }
}

pub(super) fn integer_type_from_ty(ty: &Ty) -> Option<IntegerType> {
    let path = match &ty.kind {
        TyKind::Path(None, path) => path,
        _ => return None,
    };

    if !path.is_global() && path.segments.len() == 1 {
        return match path.segments[0].ident.name.as_str() {
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

    if path.is_global()
        && path.segments.len() == 4
        && path.segments[1].ident.name.as_str() == "core"
        && path.segments[2].ident.name.as_str() == "ffi"
    {
        return match path.segments[3].ident.name.as_str() {
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

pub(super) fn unsuffixed_integer_literal_value(expr: &Expr) -> Option<IntValue> {
    match &expr.kind {
        ExprKind::Lit(lit) => match lit.kind {
            LitKind::Int(value, LitIntType::Unsuffixed) if lit.token_lit.suffix.is_none() => {
                Some(IntValue::NonNegative(value))
            }
            _ => None,
        },
        ExprKind::Unary(UnOp::Neg, inner) => match &inner.kind {
            ExprKind::Lit(lit) => match lit.kind {
                LitKind::Int(value, LitIntType::Unsuffixed) if lit.token_lit.suffix.is_none() => {
                    Some(IntValue::Negative(value))
                }
                _ => None,
            },
            _ => None,
        },
        _ => None,
    }
}

fn integer_type_width_and_signedness(ty: IntegerType, target: TargetInfo) -> (u32, bool) {
    match ty {
        IntegerType::Primitive(name) => match name {
            "i8" => (8, true),
            "i16" => (16, true),
            "i32" => (32, true),
            "i64" => (64, true),
            "i128" => (128, true),
            "isize" => (target.pointer_width, true),
            "u8" => (8, false),
            "u16" => (16, false),
            "u32" => (32, false),
            "u64" => (64, false),
            "u128" => (128, false),
            "usize" => (target.pointer_width, false),
            _ => unreachable!("unsupported primitive integer type {}", name),
        },
        IntegerType::C(c_type) => {
            let signed = matches!(
                c_type,
                CIntegerType::SChar
                    | CIntegerType::Short
                    | CIntegerType::Int
                    | CIntegerType::Long
                    | CIntegerType::LongLong
            );
            let width = match c_type {
                CIntegerType::SChar | CIntegerType::UChar => target.c_char_width,
                CIntegerType::Short | CIntegerType::UShort => target.c_short_width,
                CIntegerType::Int | CIntegerType::UInt => target.c_int_width,
                CIntegerType::Long | CIntegerType::ULong => target.c_long_width,
                CIntegerType::LongLong | CIntegerType::ULongLong => target.c_longlong_width,
            };
            (width, signed)
        }
    }
}

pub(super) fn integer_value_fits_type(
    value: IntValue,
    ty: IntegerType,
    target: TargetInfo,
) -> bool {
    let (bits, signed) = integer_type_width_and_signedness(ty, target);
    match value {
        IntValue::NonNegative(value) => {
            if signed {
                value <= signed_max_for_bits(bits)
            } else {
                value <= unsigned_max_for_bits(bits)
            }
        }
        IntValue::Negative(magnitude) => signed && magnitude <= signed_min_magnitude_for_bits(bits),
    }
}

pub(super) fn integer_type_range_is_subset(
    from: IntegerType,
    to: IntegerType,
    target: TargetInfo,
) -> bool {
    let (from_bits, from_signed) = integer_type_width_and_signedness(from, target);
    let (to_bits, to_signed) = integer_type_width_and_signedness(to, target);
    match (from_signed, to_signed) {
        (true, true) | (false, false) => from_bits <= to_bits,
        // An unsigned n-bit range fits in a signed m-bit range exactly when
        // the signed destination has at least one additional value bit.
        (false, true) => from_bits < to_bits,
        // A signed integer type always contains negative values.
        (true, false) => false,
    }
}

fn signed_max_for_bits(bits: u32) -> u128 {
    match bits {
        8 => i8::MAX as u128,
        16 => i16::MAX as u128,
        32 => i32::MAX as u128,
        64 => i64::MAX as u128,
        128 => i128::MAX as u128,
        _ => (1u128 << (bits - 1)) - 1,
    }
}

fn signed_min_magnitude_for_bits(bits: u32) -> u128 {
    1u128 << (bits - 1)
}

fn unsigned_max_for_bits(bits: u32) -> u128 {
    if bits == 128 {
        u128::MAX
    } else {
        (1u128 << bits) - 1
    }
}
