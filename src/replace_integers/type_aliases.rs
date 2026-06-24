use rustc_ast::ptr::P;
use rustc_ast::{Crate, Item, ItemKind, ModKind, Ty, TyKind};
use std::collections::{HashMap, HashSet};

pub fn validated_integer_aliases(krate: &Crate) -> HashMap<String, &'static str> {
    let aliases = collect_type_aliases(krate);
    let mut valid = HashMap::new();
    for name in aliases.keys() {
        if let Some(expected) = expected_alias_replacement(name) {
            if let Some(actual) = resolve_alias_chain(name, &aliases) {
                if actual == expected {
                    valid.insert(name.clone(), expected);
                }
            }
        }
    }
    valid
}

fn collect_type_aliases(krate: &Crate) -> HashMap<String, String> {
    let mut aliases = HashMap::new();
    collect_type_aliases_from_items(&krate.items, &mut aliases);
    aliases
}

fn collect_type_aliases_from_items(
    items: &[P<Item>],
    aliases: &mut HashMap<String, String>,
) {
    for item in items {
        match item.kind {
            ItemKind::TyAlias(ref ty_alias) => {
                if let Some(ref ty) = ty_alias.ty {
                    if let Some(target) = type_last_path_segment(ty) {
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

fn type_last_path_segment(ty: &Ty) -> Option<String> {
    if let TyKind::Path(_, ref path) = ty.kind {
        path.segments.last().map(|s| s.ident.name.to_string())
    } else {
        None
    }
}

fn expected_alias_replacement(name: &str) -> Option<&'static str> {
    match name {
        "__int8_t" | "int8_t" => Some("i8"),
        "__uint8_t" | "uint8_t" => Some("u8"),

        "__int16_t" | "int16_t" => Some("i16"),
        "__uint16_t" | "uint16_t" => Some("u16"),

        "__int32_t" | "int32_t" => Some("i32"),
        "__uint32_t" | "uint32_t" => Some("u32"),

        "__int64_t" | "int64_t" => Some("i64"),
        "__uint64_t" | "uint64_t" => Some("u64"),

        _ => None,
    }
}

fn resolve_alias_chain(name: &str, aliases: &HashMap<String, String>) -> Option<String> {
    let mut current = name.to_string();
    let mut seen = HashSet::new();
    loop {
        if !seen.insert(current.clone()) {
            return None;
        }
        match aliases.get(&current) {
            Some(next) => current = next.clone(),
            None => return Some(current),
        }
    }
}
