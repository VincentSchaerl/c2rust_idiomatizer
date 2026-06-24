mod type_aliases;

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{Crate, Item, ItemKind, ModKind, PathSegment, Ty, TyKind};
use rustc_span::symbol::{Ident, Symbol};

use std::collections::HashMap;

use type_aliases::validated_integer_aliases;

#[derive(Clone, Debug)]
struct BitWidths {
    c_short_bits: u32,
    c_int_bits: u32,
    c_long_bits: u32,
    c_longlong_bits: u32,
}

struct ReplaceIntegers {
    bit_widths: BitWidths,
    valid_aliases: HashMap<String, &'static str>,
}

pub fn register_replace_integers(reg: &mut Registry) {
    reg.register("replace_integers", |args|
    {
        let args = args.to_vec();
        Box::new(DriverCommand::new(Phase::Phase1, move |st, cx|
        {
            let target = &cx.session().target;
            let c_int_bits = target.c_int_width.parse::<u32>().unwrap_or(32);
            let pointer_bits = target.pointer_width;
            let windows_llp64 = target.os == "windows" || target.env == "msvc";
            let c_long_bits = if windows_llp64
            {
                32
            } else
            {
                pointer_bits
            };
            let mut bit_widths = BitWidths
            {
                c_short_bits: 16,
                c_int_bits,
                c_long_bits,
                c_longlong_bits: 64,
            };
            for arg in &args
            {
                if let Some(value) = arg.strip_prefix("short-width=")
                {
                    bit_widths.c_short_bits = value.parse::<u32>().expect("Invalid value for short-width");
                } else if let Some(value) = arg.strip_prefix("int-width=")
                {
                    bit_widths.c_int_bits = value
                        .parse::<u32>().expect("Invalid value for int-width");
                } else if let Some(value) = arg.strip_prefix("long-width=")
                {
                    bit_widths.c_long_bits = value.parse::<u32>().expect("Invalid value for long-width");
                } else if let Some(value) = arg.strip_prefix("longlong-width=")
                {
                    bit_widths.c_longlong_bits = value.parse::<u32>().expect("Invalid value for longlong-width");
                } else
                {
                    panic!("Unknown argument for replace_integers: {}", arg);
                }
            }
            if !(bit_widths.c_short_bits<= bit_widths.c_int_bits && bit_widths.c_int_bits <= bit_widths.c_long_bits
                && bit_widths.c_long_bits <= bit_widths.c_longlong_bits)
            {
                panic!(
                    "Invalid C integer widths: expected short <= int <= long <= long long, got short={}, int={}, long={}, long long={}",
                    bit_widths.c_short_bits,
                    bit_widths.c_int_bits,
                    bit_widths.c_long_bits,
                    bit_widths.c_longlong_bits
                );
            }
            replace_integers(&mut st.krate_mut(), bit_widths);
        }))
    });
}

fn replace_integers(krate: &mut Crate, bit_widths: BitWidths) {
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
                if let Some(replacement) =
                    replacement_for_type_name(&last, &self.bit_widths, &self.valid_aliases)
                {
                    *qself = None;
                    set_path_to_single_ident(path, replacement);
                    return;
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

fn replacement_for_type_name(
    name: &str,
    bit_widths: &BitWidths,
    valid_aliases: &HashMap<String, &'static str>,
) -> Option<&'static str> {
    if let Some(replacement) = valid_aliases.get(name) {
        return Some(*replacement);
    }
    match name {
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
