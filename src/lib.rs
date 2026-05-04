#![feature(rustc_private, box_patterns)]

extern crate rustc_ast;
extern crate rustc_span;

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::{Crate, ItemKind, PathSegment, Ty, TyKind};
use rustc_span::symbol::{Ident, Symbol};

#[derive(Clone, Debug)]
struct Platform {
    c_int_bits: u32,
    pointer_bits: u32,
    windows_llp64: bool,
}

impl Platform {
    fn c_long_bits(&self) -> u32 {
        if self.windows_llp64 {
            32
        } else {
            self.pointer_bits
        }
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
        _ => "i32",
    }
}

fn replacement_for_type_name(name: &str, platform: &Platform) -> Option<&'static str> {
    match name {
        "__int8_t" | "int8_t" => Some("i8"),
        "__uint8_t" | "uint8_t" => Some("u8"),

        "__int16_t" | "int16_t" => Some("i16"),
        "__uint16_t" | "uint16_t" => Some("u16"),

        "__int32_t" | "int32_t" => Some("i32"),
        "__uint32_t" | "uint32_t" => Some("u32"),

        "__int64_t" | "int64_t" => Some("i64"),
        "__uint64_t" | "uint64_t" => Some("u64"),

        "c_schar" => Some("i8"),
        "c_uchar" => Some("u8"),

        "c_short" => Some("i16"),
        "c_ushort" => Some("u16"),

        "c_int" => Some(rust_int(platform.c_int_bits, true)),
        "c_uint" => Some(rust_int(platform.c_int_bits, false)),

        "c_long" => Some(rust_int(platform.c_long_bits(), true)),
        "c_ulong" => Some(rust_int(platform.c_long_bits(), false)),

        "c_longlong" => Some("i64"),
        "c_ulonglong" => Some("u64"),

        _ => None,
    }
}

fn is_removed_typedef_name(name: &str) -> bool {
    matches!(
        name,
        "__int8_t"
            | "int8_t"
            | "__uint8_t"
            | "uint8_t"
            | "__int16_t"
            | "int16_t"
            | "__uint16_t"
            | "uint16_t"
            | "__int32_t"
            | "int32_t"
            | "__uint32_t"
            | "uint32_t"
            | "__int64_t"
            | "int64_t"
            | "__uint64_t"
            | "uint64_t"
    )
}

fn set_path_to_single_ident(path: &mut rustc_ast::Path, name: &str) {
    path.segments.clear();
    path.segments.push(PathSegment::from_ident(Ident::with_dummy_span(
        Symbol::intern(name),
    )));
}

struct ReplaceIntegers {
    platform: Platform,
}

impl MutVisitor for ReplaceIntegers {
    fn visit_crate(&mut self, krate: &mut Crate) {
        krate.items.retain(|item| {
            let name = item.ident.name.to_string();

            !(matches!(item.kind, ItemKind::TyAlias(..)) && is_removed_typedef_name(&name))
        });

        mut_visit::noop_visit_crate(krate, self);
    }

    fn visit_ty(&mut self, ty: &mut P<Ty>) {
        if let TyKind::Path(ref mut qself, ref mut path) = ty.kind {
            let last = path.segments.last().map(|s| s.ident.name.to_string());

            if let Some(last) = last {
                if let Some(replacement) = replacement_for_type_name(&last, &self.platform) {
                    *qself = None;
                    set_path_to_single_ident(path, replacement);
                    return;
                }
            }
        }

        mut_visit::noop_visit_ty(ty, self);
    }
}

fn replace_integers(krate: &mut Crate, platform: Platform) {
    let mut visitor = ReplaceIntegers { platform };
    visitor.visit_crate(krate);
}

#[no_mangle]
pub fn register_commands(reg: &mut Registry) {
    reg.register("replace_integers", |_args| {
        Box::new(DriverCommand::new(Phase::Phase1, |st, cx| {
            let target = &cx.session().target;

            let c_int_bits = target.c_int_width.parse::<u32>().unwrap_or(32);
            let pointer_bits = target.pointer_width;

            let windows_llp64 = target.os == "windows" || target.env == "msvc";

            let platform = Platform {
                c_int_bits,
                pointer_bits,
                windows_llp64,
            };

            replace_integers(&mut st.krate_mut(), platform);
        }))
    });
}
