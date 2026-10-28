mod index_based_to_iterator_based;
mod index_type_to_usize;
mod initial_transformation;
mod iterate_arrays_idiomatically;
mod pass_arrays_as_slices;

use index_based_to_iterator_based::index_based_to_iterator_based;
use index_type_to_usize::index_type_to_usize;
use initial_transformation::initial_transformation;
use iterate_arrays_idiomatically::iterate_arrays_idiomatically;
use pass_arrays_as_slices::pass_arrays_as_slices;

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;
use c2rust_refactor::RefactorCtxt;

use rustc_ast::ptr::P;
use rustc_ast::visit::{self, Visitor};
use rustc_ast::{
    BindingAnnotation, ByRef, Expr, ExprKind, IntTy, Item, ItemKind, LitIntType, LitKind,
    Mutability, Pat, PatKind, Ty, TyKind, UintTy, UnOp,
};
use rustc_hir::def::Res;
use rustc_hir::def_id::DefId;
use rustc_hir::HirId;
use rustc_middle::ty;

use std::collections::{HashMap, HashSet};

// For bindings newly introduced by an assignment-based initial transformation,
// keep the original HIR identity of the induction variable for later Phase-3 passes.
pub(super) type LoopBindingOrigins = HashMap<rustc_span::Span, HirId>;

#[derive(Clone, Copy, Debug)]
pub(super) struct IdiomatizeLoopsOptions {
    pub iterate_arrays_idiomatically: bool,
    pub pass_arrays_as_slices: bool,
    pub transform_public_functions: bool,
    pub index_type_to_usize: bool,
    pub index_based_to_iterator_based: bool,
}

impl Default for IdiomatizeLoopsOptions {
    fn default() -> Self {
        Self {
            iterate_arrays_idiomatically: true,
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

            let binding_origins = initial_transformation(st, cx, target_info);
            if options.iterate_arrays_idiomatically {
                iterate_arrays_idiomatically(st, cx, target_info, &binding_origins);
            }
            if options.pass_arrays_as_slices {
                pass_arrays_as_slices(
                    st,
                    cx,
                    target_info,
                    options.transform_public_functions,
                    &binding_origins,
                );
            }
            if options.index_type_to_usize {
                index_type_to_usize(st, cx, &binding_origins);
            }
            if options.index_based_to_iterator_based {
                index_based_to_iterator_based(st, cx, &binding_origins);
            }
        }))
    });
}

fn parse_options(args: &[String]) -> IdiomatizeLoopsOptions {
    let mut options = IdiomatizeLoopsOptions::default();
    let mut transform_public_functions_explicit = false;
    let mut index_type_to_usize_explicit = false;
    let mut index_based_to_iterator_based_explicit = false;

    for arg in args {
        if let Some(value) = arg.strip_prefix("iterate-arrays-idiomatically=") {
            options.iterate_arrays_idiomatically =
                parse_bool_arg("iterate-arrays-idiomatically", value);
        } else if let Some(value) = arg.strip_prefix("pass-arrays-as-slices=") {
            options.pass_arrays_as_slices = parse_bool_arg("pass-arrays-as-slices", value);
        } else if let Some(value) = arg.strip_prefix("transform-public-functions=") {
            options.transform_public_functions =
                parse_bool_arg("transform-public-functions", value);
            transform_public_functions_explicit = true;
        } else if let Some(value) = arg.strip_prefix("index-type-to-usize=") {
            options.index_type_to_usize = parse_bool_arg("index-type-to-usize", value);
            index_type_to_usize_explicit = true;
        } else if let Some(value) = arg.strip_prefix("index-based-to-iterator-based=") {
            options.index_based_to_iterator_based =
                parse_bool_arg("index-based-to-iterator-based", value);
            index_based_to_iterator_based_explicit = true;
        } else {
            panic!("Unknown argument for idiomatize_loops: {}", arg);
        }
    }

    if !transform_public_functions_explicit && !options.pass_arrays_as_slices {
        options.transform_public_functions = false;
    }

    if !index_type_to_usize_explicit
        && !options.iterate_arrays_idiomatically
        && !options.pass_arrays_as_slices
    {
        options.index_type_to_usize = false;
    }

    if !index_based_to_iterator_based_explicit
        && ((!options.iterate_arrays_idiomatically && !options.pass_arrays_as_slices)
            || !options.index_type_to_usize)
    {
        options.index_based_to_iterator_based = false;
    }

    options
}

fn validate_options(options: IdiomatizeLoopsOptions) {
    if options.index_type_to_usize
        && !options.iterate_arrays_idiomatically
        && !options.pass_arrays_as_slices
    {
        panic!(concat!(
            "Invalid arguments for idiomatize_loops: ",
            "index-type-to-usize=true requires ",
            "iterate-arrays-idiomatically=true or pass-arrays-as-slices=true. ",
            "The index-type pass only operates on loops produced by one of those passes."
        ));
    }

    if options.transform_public_functions && !options.pass_arrays_as_slices {
        panic!(concat!(
            "Invalid arguments for idiomatize_loops: ",
            "transform-public-functions=true requires ",
            "pass-arrays-as-slices=true. Public-function transformation ",
            "is part of pass-arrays-as-slices."
        ));
    }

    if options.index_based_to_iterator_based && !options.index_type_to_usize {
        panic!(concat!(
            "Invalid arguments for idiomatize_loops: ",
            "index-based-to-iterator-based=true requires ",
            "index-type-to-usize=true. Iterator-based loop conversion ",
            "depends on converting index types to usize first."
        ));
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

// -----------------------------------------------------------------------------
// Shared source-level helpers used by multiple loop passes
// -----------------------------------------------------------------------------

#[derive(Clone, Debug)]
pub(super) struct IntegerConstMeta {
    pub integer_ty: IntegerType,
    pub value: IntValue,
}

// All alias definitions, regardless of visibility, are needed to resolve
// primitive integer types through arbitrary alias chains. Visibility checks
// for public API transformations remain separate from this type resolution.
pub(super) fn collect_integer_type_aliases(
    krate: &rustc_ast::Crate,
    cx: &RefactorCtxt,
) -> HashMap<DefId, P<Ty>> {
    let mut aliases = HashMap::new();
    // Unlike a module-only item walk, visit_nodes also finds function-local
    // type aliases, which can participate in arbitrarily long alias chains.
    c2rust_refactor::ast_manip::visit_nodes(krate, |item: &Item| {
        if let ItemKind::TyAlias(alias) = &item.kind {
            if let Some(ty) = &alias.ty {
                aliases.insert(cx.node_def_id(item.id), ty.clone());
            }
        }
    });
    aliases
}

pub(super) fn collect_public_integer_consts(
    krate: &rustc_ast::Crate,
    cx: &RefactorCtxt,
    target: TargetInfo,
    aliases: &HashMap<DefId, P<Ty>>,
) -> HashMap<DefId, IntegerConstMeta> {
    fn collect(
        items: &[P<Item>],
        cx: &RefactorCtxt,
        target: TargetInfo,
        aliases: &HashMap<DefId, P<Ty>>,
        consts: &mut HashMap<DefId, IntegerConstMeta>,
    ) {
        for item in items {
            match &item.kind {
                ItemKind::Const(_, ty, Some(init)) if item.vis.kind.is_pub() => {
                    let value = match unsuffixed_integer_literal_value(init) {
                        Some(value) => value,
                        None => continue,
                    };
                    let integer_ty = match underlying_integer_type_from_ty(ty, cx, aliases) {
                        Some(integer_ty) => integer_ty,
                        None => continue,
                    };
                    if integer_value_fits_type(value, integer_ty, target) {
                        consts.insert(
                            cx.node_def_id(item.id),
                            IntegerConstMeta { integer_ty, value },
                        );
                    }
                }
                ItemKind::Mod(_, rustc_ast::ModKind::Loaded(module_items, ..)) => {
                    collect(module_items, cx, target, aliases, consts);
                }
                _ => {}
            }
        }
    }

    let mut consts = HashMap::new();
    collect(&krate.items, cx, target, aliases, &mut consts);
    consts
}

pub(super) fn underlying_integer_type_from_ty(
    ty: &Ty,
    cx: &RefactorCtxt,
    aliases: &HashMap<DefId, P<Ty>>,
) -> Option<IntegerType> {
    fn resolve(
        ty: &Ty,
        cx: &RefactorCtxt,
        aliases: &HashMap<DefId, P<Ty>>,
        visiting: &mut HashSet<DefId>,
    ) -> Option<IntegerType> {
        if let Some(integer) = integer_type_from_ty(ty) {
            return Some(integer);
        }
        let alias_id = cx.try_resolve_ty(ty)?;
        if !visiting.insert(alias_id) {
            return None; // Recursive alias.
        }
        let result = resolve(aliases.get(&alias_id)?, cx, aliases, visiting);
        visiting.remove(&alias_id);
        result
    }
    resolve(ty, cx, aliases, &mut HashSet::new())
}

pub(super) fn array_type_with_literal_length(ty: &Ty) -> Option<(P<Ty>, u128)> {
    let (element_ty, len) = match &ty.kind {
        TyKind::Array(element_ty, len) => (element_ty, len),
        _ => return None,
    };
    match unsuffixed_integer_literal_value(&len.value)? {
        IntValue::NonNegative(value) => Some((element_ty.clone(), value)),
        IntValue::Negative(_) => None,
    }
}

pub(super) fn simple_loop_binding(
    pat: &Pat,
    cx: &RefactorCtxt,
    binding_origins: &LoopBindingOrigins,
) -> Option<(Option<HirId>, String)> {
    match pat.kind {
        PatKind::Ident(BindingAnnotation(ByRef::No, Mutability::Not), ident, None) => Some((
            cx.hir_map()
                .opt_node_to_hir_id(pat.id)
                .or_else(|| binding_origins.get(&pat.span).copied()),
            ident.name.to_string(),
        )),
        _ => None,
    }
}

pub(super) fn record_pattern_binding_name(pat: &Pat, names: &mut HashSet<String>) {
    if let PatKind::Ident(_, ident, _) = &pat.kind {
        names.insert(ident.name.to_string());
    }
}

pub(super) fn collect_pattern_binding_names(pat: &Pat, names: &mut HashSet<String>) {
    struct Collector<'names> {
        names: &'names mut HashSet<String>,
    }

    impl<'ast, 'names> Visitor<'ast> for Collector<'names> {
        fn visit_pat(&mut self, pat: &'ast Pat) {
            record_pattern_binding_name(pat, self.names);
            visit::walk_pat(self, pat);
        }
    }

    let mut collector = Collector { names };
    collector.visit_pat(pat);
}

pub(super) fn adjusted_reference_mutability(expr: &Expr, cx: &RefactorCtxt) -> Option<Mutability> {
    let original = cx.opt_node_type(expr.id)?;
    let adjusted = cx.opt_adjusted_node_type(expr.id)?;
    if matches!(original.kind(), ty::TyKind::Ref(..)) {
        return None;
    }
    match adjusted.kind() {
        ty::TyKind::Ref(_, _, mutability) => Some(*mutability),
        _ => None,
    }
}

pub(super) fn fresh_name(requested: &str, names: &HashSet<String>) -> String {
    if !names.contains(requested) {
        return requested.to_string();
    }
    for suffix in 0u64.. {
        let candidate = format!("{}_{}", requested, suffix);
        if !names.contains(&candidate) {
            return candidate;
        }
    }
    unreachable!()
}

pub(super) fn simple_path_name(expr: &Expr) -> Option<&str> {
    match &strip_parens(expr).kind {
        ExprKind::Path(None, path) if !path.is_global() && path.segments.len() == 1 => {
            Some(path.segments[0].ident.name.as_str())
        }
        _ => None,
    }
}

pub(super) fn expr_matches_loop_binding(
    expr: &Expr,
    binding_hir: Option<HirId>,
    binding_name: &str,
    cx: &RefactorCtxt,
) -> bool {
    let expr = strip_parens(expr);
    match cx.try_resolve_expr_hir(expr) {
        Some(Res::Local(id)) => binding_hir == Some(id),
        Some(_) => false,
        None => simple_path_name(expr) == Some(binding_name),
    }
}

pub(super) fn suffixed_zero_integer_type(expr: &Expr) -> Option<IntegerType> {
    let expr = strip_parens(expr);
    let lit = match &expr.kind {
        ExprKind::Lit(lit) => lit,
        _ => return None,
    };
    match lit.kind {
        LitKind::Int(0, LitIntType::Signed(ty)) => Some(match ty {
            IntTy::I8 => IntegerType::Primitive("i8"),
            IntTy::I16 => IntegerType::Primitive("i16"),
            IntTy::I32 => IntegerType::Primitive("i32"),
            IntTy::I64 => IntegerType::Primitive("i64"),
            IntTy::I128 => IntegerType::Primitive("i128"),
            IntTy::Isize => IntegerType::Primitive("isize"),
        }),
        LitKind::Int(0, LitIntType::Unsigned(ty)) => Some(match ty {
            UintTy::U8 => IntegerType::Primitive("u8"),
            UintTy::U16 => IntegerType::Primitive("u16"),
            UintTy::U32 => IntegerType::Primitive("u32"),
            UintTy::U64 => IntegerType::Primitive("u64"),
            UintTy::U128 => IntegerType::Primitive("u128"),
            UintTy::Usize => IntegerType::Primitive("usize"),
        }),
        _ => None,
    }
}

pub(super) fn array_len_receiver(expr: &Expr) -> Option<&Expr> {
    match &strip_parens(expr).kind {
        ExprKind::MethodCall(segment, receiver, args, _)
            if segment.ident.name.as_str() == "len" && args.is_empty() =>
        {
            Some(strip_parens(receiver))
        }
        _ => None,
    }
}

pub(super) fn strip_parens(mut expr: &Expr) -> &Expr {
    while let ExprKind::Paren(inner) = &expr.kind {
        expr = inner;
    }
    expr
}

pub(super) fn integer_type_source(ty: IntegerType) -> &'static str {
    match ty {
        IntegerType::Primitive(name) => name,
        IntegerType::C(CIntegerType::SChar) => "::core::ffi::c_schar",
        IntegerType::C(CIntegerType::UChar) => "::core::ffi::c_uchar",
        IntegerType::C(CIntegerType::Short) => "::core::ffi::c_short",
        IntegerType::C(CIntegerType::UShort) => "::core::ffi::c_ushort",
        IntegerType::C(CIntegerType::Int) => "::core::ffi::c_int",
        IntegerType::C(CIntegerType::UInt) => "::core::ffi::c_uint",
        IntegerType::C(CIntegerType::Long) => "::core::ffi::c_long",
        IntegerType::C(CIntegerType::ULong) => "::core::ffi::c_ulong",
        IntegerType::C(CIntegerType::LongLong) => "::core::ffi::c_longlong",
        IntegerType::C(CIntegerType::ULongLong) => "::core::ffi::c_ulonglong",
    }
}
