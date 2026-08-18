#![feature(rustc_private, box_patterns)]

extern crate rustc_ast;
extern crate rustc_span;

mod remove_redundant_integer_casts;
mod replace_integer_types;

use c2rust_refactor::command::Registry;
use remove_redundant_integer_casts::register_remove_redundant_integer_casts;
use replace_integer_types::register_replace_integer_types;

#[no_mangle]
pub fn register_commands(reg: &mut Registry) {
    register_remove_redundant_integer_casts(reg);
    register_replace_integer_types(reg);
}
