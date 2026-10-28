use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{Crate, Item, ItemKind, ModKind, PathSegment, Ty, TyKind};
use rustc_span::symbol::{Ident, Symbol};

use std::collections::HashMap;

#[derive(Clone, Debug)]
struct BitWidths {
    c_char_bits: u32,
    c_short_bits: u32,
    c_int_bits: u32,
    c_long_bits: u32,
    c_longlong_bits: u32,
}

struct ReplaceIntegers {
    bit_widths: BitWidths,
    valid_aliases: HashMap<String, &'static str>,
}

pub fn register_replace_integer_types(reg: &mut Registry) {
    reg.register("replace_integer_types", |args| {
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
                    panic!("Unknown argument for replace_integer_types: {}", arg);
                }
            }
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
                        "Invalid C integer widths: expected widths in {{8, 16, 32, 64, 128}}, ",
                        "minimums char>=8, short>=16, int>=16, long>=32, long long>=64, ",
                        "and char <= short <= int <= long <= long long; ",
                        "got char={}, short={}, int={}, long={}, long long={}"
                    ),
                    bit_widths.c_char_bits,
                    bit_widths.c_short_bits,
                    bit_widths.c_int_bits,
                    bit_widths.c_long_bits,
                    bit_widths.c_longlong_bits
                );
            }
            replace_integer_types(&mut st.krate_mut(), bit_widths);
        }))
    });
}

fn replace_integer_types(krate: &mut Crate, bit_widths: BitWidths) {
    let valid_aliases = validated_integer_aliases(krate);
    let mut visitor = ReplaceIntegers {
        bit_widths,
        valid_aliases,
    };
    visitor.visit_crate(krate);
}

impl MutVisitor for ReplaceIntegers {
    fn visit_crate(&mut self, krate: &mut Crate) {
        remove_valid_integer_aliases_from_items(&mut krate.items, &self.valid_aliases);
        mut_visit::noop_visit_crate(krate, self);
    }

    fn visit_ty(&mut self, ty: &mut P<Ty>) {
        if let TyKind::Path(ref mut qself, ref mut path) = ty.kind {
            let last = path.segments.last().map(|s| s.ident.name.to_string());
            if let Some(last) = last {
                if let Some(replacement) = self.valid_aliases.get(&last) {
                    *qself = None;
                    set_path_to_single_ident(path, replacement);
                    return;
                }
                if qself.is_none() && is_core_ffi_path(path) {
                    if let Some(replacement) =
                        replacement_for_c_ffi_type_name(&last, &self.bit_widths)
                    {
                        set_path_to_single_ident(path, replacement);
                        return;
                    }
                }
            }
        }
        mut_visit::noop_visit_ty(ty, self);
    }
}

fn remove_valid_integer_aliases_from_items(
    items: &mut Vec<P<Item>>,
    valid_aliases: &HashMap<String, &'static str>,
) {
    items.retain(|item| {
        let name = item.ident.name.to_string();
        !(matches!(item.kind, ItemKind::TyAlias(..)) && valid_aliases.contains_key(&name))
    });
    for item in items.iter_mut() {
        if let ItemKind::Mod(_, ModKind::Loaded(ref mut module_items, ..)) = item.kind {
            remove_valid_integer_aliases_from_items(module_items, valid_aliases);
        }
    }
}

fn is_core_ffi_path(path: &rustc_ast::Path) -> bool {
    path.is_global()
        && path.segments.len() == 4
        && path.segments[1].ident.name.as_str() == "core"
        && path.segments[2].ident.name.as_str() == "ffi"
}

fn replacement_for_c_ffi_type_name(name: &str, bit_widths: &BitWidths) -> Option<&'static str> {
    match name {
        "c_schar" => Some(rust_int(bit_widths.c_char_bits, true)),
        "c_uchar" => Some(rust_int(bit_widths.c_char_bits, false)),

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

fn set_path_to_single_ident(path: &mut rustc_ast::Path, name: &str) {
    let span = path.span;
    path.segments.clear();
    path.segments.push(PathSegment::from_ident(Ident::new(
        Symbol::intern(name),
        span,
    )));
}

// -----------------------------------------------------------------------------
// Validated C integer type aliases
// -----------------------------------------------------------------------------

struct IntegerAliasPair {
    public_name: &'static str,
    internal_name: &'static str,
    replacement: &'static str,
}

const INTEGER_ALIAS_PAIRS: &[IntegerAliasPair] = &[
    IntegerAliasPair {
        public_name: "int8_t",
        internal_name: "__int8_t",
        replacement: "i8",
    },
    IntegerAliasPair {
        public_name: "uint8_t",
        internal_name: "__uint8_t",
        replacement: "u8",
    },
    IntegerAliasPair {
        public_name: "int16_t",
        internal_name: "__int16_t",
        replacement: "i16",
    },
    IntegerAliasPair {
        public_name: "uint16_t",
        internal_name: "__uint16_t",
        replacement: "u16",
    },
    IntegerAliasPair {
        public_name: "int32_t",
        internal_name: "__int32_t",
        replacement: "i32",
    },
    IntegerAliasPair {
        public_name: "uint32_t",
        internal_name: "__uint32_t",
        replacement: "u32",
    },
    IntegerAliasPair {
        public_name: "int64_t",
        internal_name: "__int64_t",
        replacement: "i64",
    },
    IntegerAliasPair {
        public_name: "uint64_t",
        internal_name: "__uint64_t",
        replacement: "u64",
    },
];

struct DirectIntegerAlias {
    name: &'static str,
    replacement: &'static str,
}

const DIRECT_INTEGER_ALIASES: &[DirectIntegerAlias] = &[
    DirectIntegerAlias {
        name: "intptr_t",
        replacement: "isize",
    },
    DirectIntegerAlias {
        name: "uintptr_t",
        replacement: "usize",
    },
    DirectIntegerAlias {
        name: "size_t",
        replacement: "usize",
    },
];

pub fn validated_integer_aliases(krate: &Crate) -> HashMap<String, &'static str> {
    let aliases = collect_type_aliases(krate);
    let mut valid = HashMap::new();
    for pair in INTEGER_ALIAS_PAIRS {
        let public_target = aliases.get(pair.public_name).map(String::as_str);
        let internal_target = aliases.get(pair.internal_name).map(String::as_str);
        if public_target == Some(pair.internal_name) && internal_target == Some(pair.replacement) {
            valid.insert(pair.public_name.to_string(), pair.replacement);
            valid.insert(pair.internal_name.to_string(), pair.replacement);
        }
    }
    for alias in DIRECT_INTEGER_ALIASES {
        let target = aliases.get(alias.name).map(String::as_str);
        if target == Some(alias.replacement) {
            valid.insert(alias.name.to_string(), alias.replacement);
        }
    }
    valid
}

fn collect_type_aliases(krate: &Crate) -> HashMap<String, String> {
    let mut aliases = HashMap::new();
    collect_type_aliases_from_items(&krate.items, &mut aliases);
    aliases
}

fn collect_type_aliases_from_items(items: &[P<Item>], aliases: &mut HashMap<String, String>) {
    for item in items {
        match item.kind {
            ItemKind::TyAlias(ref ty_alias) => {
                if let Some(ref ty) = ty_alias.ty {
                    if let Some(target) = simple_type_path_name(ty) {
                        aliases.insert(item.ident.name.to_string(), target);
                    }
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                collect_type_aliases_from_items(module_items, aliases);
            }
            _ => {}
        }
    }
}

fn simple_type_path_name(ty: &Ty) -> Option<String> {
    let TyKind::Path(ref qself, ref path) = ty.kind else {
        return None;
    };
    if qself.is_some() || path.is_global() || path.segments.len() != 1 {
        return None;
    }
    Some(path.segments[0].ident.name.to_string())
}
