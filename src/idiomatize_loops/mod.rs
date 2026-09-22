mod base_case;

use base_case::base_case;

use c2rust_refactor::command::{DriverCommand, Registry};
use c2rust_refactor::driver::Phase;

#[derive(Clone, Copy, Debug)]
pub(super) struct IdiomatizeLoopsOptions {
    pub pass_arrays_as_slices: bool,
    pub index_type_to_usize: bool,
    pub index_based_to_iterator_based: bool,
}

impl Default for IdiomatizeLoopsOptions {
    fn default() -> Self {
        Self {
            pass_arrays_as_slices: true,
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

        // The base case deliberately runs in Phase 3: its stability checks need
        // rustc's type-checking results/adjustments (in particular auto-borrows)
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

            base_case(st, cx, target_info);
        }))
    });
}

fn parse_options(args: &[String]) -> IdiomatizeLoopsOptions {
    let mut options = IdiomatizeLoopsOptions::default();
    let mut index_based_to_iterator_based_explicit = false;

    for arg in args {
        if let Some(value) = arg.strip_prefix("pass-arrays-as-slices=") {
            options.pass_arrays_as_slices = parse_bool_arg("pass-arrays-as-slices", value);
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
