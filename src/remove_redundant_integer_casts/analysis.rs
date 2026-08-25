use super::cast::{integer_type_from_ty, type_info_from_ty};
use super::{BitWidths, FunctionContext, TypeInfo};

use rustc_ast::ptr::P;
use rustc_ast::{Crate, FnRetTy, Item, ItemKind, ModKind, Pat, PatKind, VariantData};
use rustc_span::Span;

use std::collections::HashMap;

pub(super) fn collect_struct_fields(
    krate: &Crate,
    bit_widths: &BitWidths,
) -> HashMap<String, HashMap<String, TypeInfo>> {
    let mut structs = HashMap::new();
    let mut module_path = Vec::new();
    collect_struct_fields_from_items(&krate.items, bit_widths, &mut module_path, &mut structs);
    structs
}

fn collect_struct_fields_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    module_path: &mut Vec<String>,
    structs: &mut HashMap<String, HashMap<String, TypeInfo>>,
) {
    for item in items {
        match item.kind {
            ItemKind::Struct(ref variant_data, _) => {
                let mut fields = HashMap::new();
                collect_fields_from_variant_data(
                    variant_data,
                    bit_widths,
                    module_path,
                    &mut fields,
                );
                structs.insert(
                    qualified_item_name(module_path, &item.ident.name.to_string()),
                    fields,
                );
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_struct_fields_from_items(module_items, bit_widths, module_path, structs);
                module_path.pop();
            }
            _ => {}
        }
    }
}

fn collect_fields_from_variant_data(
    variant_data: &VariantData,
    bit_widths: &BitWidths,
    module_path: &[String],
    fields: &mut HashMap<String, TypeInfo>,
) {
    for field in variant_data.fields() {
        if let Some(ident) = field.ident {
            if let Some(type_info) = type_info_from_ty(&field.ty, bit_widths, module_path) {
                fields.insert(ident.name.to_string(), type_info);
            }
        }
    }
}

pub(super) fn collect_global_vars(
    krate: &Crate,
    bit_widths: &BitWidths,
) -> HashMap<String, TypeInfo> {
    let mut vars = HashMap::new();
    let mut module_path = Vec::new();
    collect_global_vars_from_items(&krate.items, bit_widths, &mut module_path, &mut vars);
    vars
}

fn collect_global_vars_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    module_path: &mut Vec<String>,
    vars: &mut HashMap<String, TypeInfo>,
) {
    for item in items {
        match item.kind {
            ItemKind::Static(ref ty, ..) => {
                if let Some(type_info) = type_info_from_ty(ty, bit_widths, module_path) {
                    vars.insert(
                        qualified_item_name(module_path, &item.ident.name.to_string()),
                        type_info,
                    );
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_global_vars_from_items(module_items, bit_widths, module_path, vars);
                module_path.pop();
            }
            _ => {}
        }
    }
}

pub(super) fn collect_function_param_types(
    krate: &Crate,
    bit_widths: &BitWidths,
) -> HashMap<String, Vec<Option<TypeInfo>>> {
    let mut functions = HashMap::new();
    let mut module_path = Vec::new();
    collect_function_param_types_from_items(
        &krate.items,
        bit_widths,
        &mut module_path,
        &mut functions,
    );
    functions
}

fn collect_function_param_types_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    module_path: &mut Vec<String>,
    functions: &mut HashMap<String, Vec<Option<TypeInfo>>>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                let mut params = Vec::new();
                for param in &func.sig.decl.inputs {
                    params.push(type_info_from_ty(&param.ty, bit_widths, module_path));
                }
                functions.insert(
                    qualified_item_name(module_path, &item.ident.name.to_string()),
                    params,
                );
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_function_param_types_from_items(
                    module_items,
                    bit_widths,
                    module_path,
                    functions,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

pub(super) fn collect_function_contexts(
    krate: &Crate,
    bit_widths: &BitWidths,
) -> Vec<(Span, FunctionContext)> {
    let mut contexts = Vec::new();
    let mut module_path = Vec::new();
    collect_function_contexts_from_items(&krate.items, bit_widths, &mut module_path, &mut contexts);
    contexts
}

fn collect_function_contexts_from_items(
    items: &[P<Item>],
    bit_widths: &BitWidths,
    module_path: &mut Vec<String>,
    contexts: &mut Vec<(Span, FunctionContext)>,
) {
    for item in items {
        match item.kind {
            ItemKind::Fn(ref func) => {
                if let Some(ref body) = func.body {
                    let mut params = HashMap::new();
                    for param in &func.sig.decl.inputs {
                        if let Some(type_info) =
                            type_info_from_ty(&param.ty, bit_widths, module_path)
                        {
                            if let Some(name) = pat_name(&param.pat) {
                                params.insert(name, type_info);
                            }
                        }
                    }
                    let return_type = match func.sig.decl.output {
                        FnRetTy::Ty(ref ty) => integer_type_from_ty(ty),
                        FnRetTy::Default(_) => None,
                    };
                    contexts.push((
                        body.span,
                        FunctionContext {
                            params,
                            return_type,
                        },
                    ));
                }
            }
            ItemKind::Mod(_, ModKind::Loaded(ref module_items, ..)) => {
                module_path.push(item.ident.name.to_string());
                collect_function_contexts_from_items(
                    module_items,
                    bit_widths,
                    module_path,
                    contexts,
                );
                module_path.pop();
            }
            _ => {}
        }
    }
}

pub(super) fn pat_name(pat: &Pat) -> Option<String> {
    match pat.kind {
        PatKind::Ident(_, ident, _) => Some(ident.name.to_string()),
        _ => None,
    }
}

fn qualified_item_name(module_path: &[String], name: &str) -> String {
    if module_path.is_empty() {
        name.to_string()
    } else {
        format!("{}::{}", module_path.join("::"), name)
    }
}
