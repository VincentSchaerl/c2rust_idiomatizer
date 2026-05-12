#![feature(rustc_private, box_patterns)]

extern crate rustc_ast;
extern crate rustc_span;

mod remove_redundant_integer_casts;
mod replace_integers;

use c2rust_refactor::command::Registry;
use remove_redundant_integer_casts::register_remove_redundant_integer_casts;
use replace_integers::register_replace_integers;

#[no_mangle]
pub fn register_commands(reg: &mut Registry) {
    register_remove_redundant_integer_casts(reg);
    register_replace_integers(reg);
}
