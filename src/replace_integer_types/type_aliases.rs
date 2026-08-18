use rustc_ast::ptr::P;
use rustc_ast::{Crate, Item, ItemKind, ModKind, Ty, TyKind};
use std::collections::HashMap;

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
