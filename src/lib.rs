#![feature(rustc_private, box_patterns)]

extern crate rustc_ast;
extern crate rustc_span;

mod replace_integers;

use c2rust_refactor::command::Registry;
use replace_integers::register_replace_integers;

#[no_mangle]
pub fn register_commands(reg: &mut Registry) {
    register_replace_integers(reg);
}
