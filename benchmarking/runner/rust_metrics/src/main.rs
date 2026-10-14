#![feature(rustc_private)]

extern crate rustc_driver;
extern crate rustc_hir;
extern crate rustc_interface;
extern crate rustc_middle;
extern crate rustc_span;
extern crate rustc_lexer;

use std::collections::HashSet;
use std::env;
use std::fs;
use std::path::PathBuf;

use rustc_driver::{Callbacks, Compilation, RunCompiler};
use rustc_hir as hir;
use rustc_hir::def::{DefKind, Res};
use rustc_hir::def_id::DefId;
use rustc_hir::intravisit::{self, Visitor};
use rustc_middle::hir::nested_filter;
use rustc_interface::{interface, Queries};
use rustc_middle::ty::{self, TyCtxt};
use rustc_span::Span;

// Check the written type path, not the text of arbitrary expressions/comments.
// e.g. ::core::ffi::c_int, core::ffi::c_char; no `i32` false positives.
fn spells_c_ffi_type(snippet: &str) -> bool {
    let compact: String = snippet.chars().filter(|c| !c.is_whitespace()).collect();
    let trimmed = compact.strip_prefix("::").unwrap_or(&compact);
    let Some(tail) = trimmed.strip_prefix("core::ffi::") else {
        return false;
    };
    let name = tail.split(|c: char| !(c.is_ascii_alphanumeric() || c == '_'))
        .next().unwrap_or("");
    name.starts_with("c_") && name.len() > 2
}

#[cfg(test)]
mod tests {
    use super::spells_c_ffi_type;

    #[test]
    fn c_ffi_spelling() {
        assert!(spells_c_ffi_type("::core::ffi::c_int"));
        assert!(spells_c_ffi_type("core::ffi::c_char"));
        assert!(spells_c_ffi_type("::core :: ffi :: c_void"));
        assert!(!spells_c_ffi_type("i32"));
        assert!(!spells_c_ffi_type("other::ffi::c_int"));
    }
}

// Only pointer-offset/distance operations count as pointer arithmetic.
// In particular, pointer comparisons, dereferences, and address extraction do not.
fn is_pointer_arithmetic_name(name: &str) -> bool {
    matches!(name,
        "add" | "sub" | "offset" |
        "wrapping_add" | "wrapping_sub" | "wrapping_offset" |
        "offset_from" | "wrapping_offset_from" |
        "byte_add" | "byte_sub" | "byte_offset" |
        "wrapping_byte_add" | "wrapping_byte_sub" | "wrapping_byte_offset"
    )
}

struct MetricsCallbacks {
    output_dir: PathBuf,
}

impl Callbacks for MetricsCallbacks {
    fn after_analysis<'tcx>(
        &mut self,
        _compiler: &interface::Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> Compilation {
        queries.global_ctxt().unwrap().peek_mut().enter(|tcx| {
            let mut visitor = MetricsVisitor {
                tcx,
                events: HashSet::new(),
            };
            tcx.hir().visit_all_item_likes_in_crate(&mut visitor);

            let mut lines = Vec::new();
            for (metric, path, start_line, start_col, end_line, end_col) in &visitor.events {
                lines.push(format!(
                    "E\t{}\t{}\t{}\t{}\t{}\t{}",
                    metric, path, start_line, start_col, end_line, end_col
                ));
            }
            // The SourceMap includes files without any counted constructs.
            // Include these so Python can count characters for every compiled file.
            for file in tcx.sess.source_map().files().iter() {
                let path = file.name.prefer_local().to_string();
                if path.contains('\t') || path.contains('\n') { continue; }
                if let Ok(source) = fs::read_to_string(&path) {
                    let mut chars: usize = 0;
                    let mut offset: usize = 0;
                    for token in rustc_lexer::tokenize(&source) {
                        let end = offset + token.len as usize;
                        if !matches!(token.kind, rustc_lexer::TokenKind::LineComment { .. }
                            | rustc_lexer::TokenKind::BlockComment { .. }) {
                            chars += source[offset..end].chars().count();
                        }
                        offset = end;
                    }
                    lines.push(format!("F\t{}\t{}", path, chars));
                }

            }
            lines.sort();
            let filename = format!("{}-{}.tsv", tcx.crate_name(hir::def_id::LOCAL_CRATE), std::process::id());
            fs::write(self.output_dir.join(filename), lines.join("\n") + "\n")
                .expect("cannot write Rust analyzer results");
        });
        // `cargo check` needs successful compiler metadata for downstream targets.
        Compilation::Continue
    }
}

struct MetricsVisitor<'tcx> {
    tcx: TyCtxt<'tcx>,
    // Include both ends of each source span: nested casts can begin at the same
    // location but end at different positions, so a start-only key loses them.
    events: HashSet<(&'static str, String, usize, usize, usize, usize)>,
}

impl<'tcx> MetricsVisitor<'tcx> {
    fn event(&mut self, metric: &'static str, span: Span) {
        // Avoid recording compiler-generated desugaring or external macros.
        // For loops are an exception: they are lowered to an internal loop.
        if span.from_expansion() && metric != "for_loops" {
            return;
        }
        let span = span.source_callsite();
        let source_map = self.tcx.sess.source_map();
        let start = source_map.lookup_char_pos(span.lo());
        let end = source_map.lookup_char_pos(span.hi());
        let path = start.file.name.prefer_local().to_string();
        if !path.contains('\t') && !path.contains('\n')
            && start.file.name == end.file.name
        {
            self.events.insert((
                metric, path, start.line, start.col.0, end.line, end.col.0,
            ));
        }
    }

    // Rust's type checker erases type aliases such as core::ffi::c_int to i32.
    // Furthermore, a compiler DefId may point to a re-exported implementation
    // whose canonical DefPath does not begin with `core::ffi::` on this nightly.
    // Read the spelling of explicit paths as well as compiler resolution.
    fn source_spells_c_ffi(&self, ty: &hir::Ty<'_>) -> bool {
        if !matches!(&ty.kind, hir::TyKind::Path(..)) {
            return false;
        }
        if let Ok(snippet) = self.tcx.sess.source_map().span_to_snippet(ty.span) {
            return spells_c_ffi_type(&snippet);
        }
        false
    }

    fn matches_c_ffi_alias(&self, def_id: DefId, seen: &mut HashSet<DefId>) -> bool {
        if !seen.insert(def_id) {
            return false;
        }
        let path = self.tcx.def_path_str(def_id);
        // On this nightly some C aliases are defined below core::ffi in
        // implementation submodules rather than directly under core::ffi.
        // Their last path component still identifies the actual C FFI type.
        if path.starts_with("core::ffi::") {
            return path.rsplit("::").next().map_or(false, |name| {
                name.starts_with("c_") && name.len() > 2
            });
        }
        if let Some(local) = def_id.as_local() {
            if let hir::Node::Item(item) = self.tcx.hir().get_by_def_id(local) {
                if let hir::ItemKind::TyAlias(rhs, _) = item.kind {
                    return self.alias_rhs_matches(rhs, seen);
                }
            }
        }
        false
    }

    fn alias_rhs_matches(&self, ty: &hir::Ty<'_>, seen: &mut HashSet<DefId>) -> bool {
        if self.source_spells_c_ffi(ty) {
            return true;
        }
        if let hir::TyKind::Path(hir::QPath::Resolved(_, path)) = ty.kind {
            if let Res::Def(DefKind::TyAlias | DefKind::Struct | DefKind::Enum, id) = path.res {
                return self.matches_c_ffi_alias(id, seen);
            }
        }
        false
    }

    fn is_raw_pointer_ty(&self, ty: &hir::Ty<'_>, seen: &mut HashSet<DefId>) -> bool {
        match ty.kind {
            hir::TyKind::Ptr(..) => true,
            hir::TyKind::Path(hir::QPath::Resolved(_, path)) => {
                if let Res::Def(DefKind::TyAlias, id) = path.res {
                    if !seen.insert(id) { return false; }
                    if let Some(local) = id.as_local() {
                        if let hir::Node::Item(item) = self.tcx.hir().get_by_def_id(local) {
                            if let hir::ItemKind::TyAlias(rhs, _) = item.kind {
                                return self.is_raw_pointer_ty(rhs, seen);
                            }
                        }
                    }
                    // An alias imported from another crate has no local HIR
                    // body. Ask rustc for its resolved underlying type.
                    return matches!(self.tcx.type_of(id).kind(), ty::RawPtr(_));
                }
                false
            }
            _ => false,
        }
    }

    fn count_params(&mut self, decl: &hir::FnDecl<'_>) {
        for ty in decl.inputs {
            if self.is_raw_pointer_ty(ty, &mut HashSet::new()) {
                self.event("raw_pointer_parameters", ty.span);
            }
        }
    }

    fn expression_is_raw_pointer(&self, expr: &hir::Expr<'_>) -> bool {
        let results = self.tcx.typeck(expr.hir_id.owner.def_id);
        matches!(results.expr_ty(expr).kind(), ty::RawPtr(_))
    }
}

impl<'tcx> Visitor<'tcx> for MetricsVisitor<'tcx> {
    type NestedFilter = nested_filter::OnlyBodies;
    fn nested_visit_map(&mut self) -> Self::Map { self.tcx.hir() }

    fn visit_item(&mut self, item: &'tcx hir::Item<'tcx>) {
        if matches!(item.kind, hir::ItemKind::TyAlias(..)) {
            // The alias declaration's RHS is not an occurrence.
            return;
        }
        intravisit::walk_item(self, item);
    }

    fn visit_fn(
        &mut self,
        kind: intravisit::FnKind<'tcx>,
        decl: &'tcx hir::FnDecl<'tcx>,
        body: hir::BodyId,
        span: Span,
        id: hir::HirId,
    ) {
        if !matches!(kind, intravisit::FnKind::Closure) {
            self.count_params(decl);
        }
        intravisit::walk_fn(self, kind, decl, body, id);
        let _ = span;
    }

    fn visit_foreign_item(&mut self, item: &'tcx hir::ForeignItem<'tcx>) {
        if let hir::ForeignItemKind::Fn(decl, ..) = item.kind {
            self.count_params(decl);
        }
        intravisit::walk_foreign_item(self, item);
    }

    fn visit_trait_item(&mut self, item: &'tcx hir::TraitItem<'tcx>) {
        if matches!(item.kind, hir::TraitItemKind::Type(..)) {
            return; // Associated type declaration, not a use.
        }
        if let hir::TraitItemKind::Fn(ref sig, hir::TraitFn::Required(..)) = item.kind {
            self.count_params(sig.decl);
        }
        intravisit::walk_trait_item(self, item);
    }

    fn visit_impl_item(&mut self, item: &'tcx hir::ImplItem<'tcx>) {
        if matches!(item.kind, hir::ImplItemKind::Type(..)) {
            return; // Associated type alias RHS is not a use.
        }
        intravisit::walk_impl_item(self, item);
    }

    fn visit_ty(&mut self, node: &'tcx hir::Ty<'tcx>) {
        if let hir::TyKind::Path(hir::QPath::Resolved(_, path)) = node.kind {
            let resolves_to_c_type = match path.res {
                Res::Def(_, id) => self.matches_c_ffi_alias(id, &mut HashSet::new()),
                _ => false,
            };
            if resolves_to_c_type || self.source_spells_c_ffi(node) {
                self.event("c_type_occurrences", node.span);
            }
        }
        intravisit::walk_ty(self, node);
    }

    fn visit_expr(&mut self, expr: &'tcx hir::Expr<'tcx>) {
        match expr.kind {
            hir::ExprKind::Cast(..) => self.event("casts", expr.span),
            hir::ExprKind::Index(..) => self.event("indexing_operations", expr.span),
            hir::ExprKind::Loop(_, _, hir::LoopSource::ForLoop, _) => {
                self.event("for_loops", expr.span)
            }
            hir::ExprKind::Unary(hir::UnOp::Deref, operand) => {
                if self.expression_is_raw_pointer(operand) {
                    self.event("raw_pointer_dereferences", expr.span);
                }
            }
            hir::ExprKind::MethodCall(segment, receiver, _, _) => {
                let name = segment.ident.name.to_string();
                if is_pointer_arithmetic_name(&name)
                    && self.expression_is_raw_pointer(receiver)
                {
                    self.event("raw_pointer_arithmetic", expr.span);
                }
            }
            hir::ExprKind::Call(callee, args) => {
                // Also recognize fully-qualified calls such as
                // core::ptr::mut_ptr::offset(ptr, 1), when rustc resolves them
                // to the standard raw-pointer operations. Avoid similarly
                // named user functions taking pointer arguments.
                if let (hir::ExprKind::Path(hir::QPath::Resolved(_, path)), Some(first_arg)) =
                    (&callee.kind, args.first())
                {
                    let is_std_pointer_op = match path.res {
                        Res::Def(_, def_id) => {
                            let def_path = self.tcx.def_path_str(def_id);
                            def_path.starts_with("core::ptr::") || def_path.starts_with("std::ptr::")
                        }
                        _ => false,
                    };
                    if is_std_pointer_op
                        && path.segments.last().map_or(false, |segment| {
                            is_pointer_arithmetic_name(&segment.ident.name.to_string())
                        })
                        && self.expression_is_raw_pointer(first_arg)
                    {
                        self.event("raw_pointer_arithmetic", expr.span);
                    }
                }
            }
            _ => {}
        }
        intravisit::walk_expr(self, expr);
    }
}

fn main() {
    // Cargo's RUSTC_WORKSPACE_WRAPPER calls this as `wrapper rustc args...`.
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 { eprintln!("expected rustc path as first argument"); std::process::exit(2); }
    let output = PathBuf::from(env::var("RUST_METRICS_OUTPUT_DIR").expect("RUST_METRICS_OUTPUT_DIR missing"));
    let mut rustc_args = Vec::with_capacity(args.len() + 1);
    rustc_args.push(args[1].clone());
    rustc_args.extend(args[2..].iter().cloned());

    // Cargo invokes this analyzer instead of rustc for workspace crates.
    // rustc_driver discovers its default sysroot relative to this executable,
    // which is in runner/rust_metrics/target/, NOT in the installed nightly.
    // Therefore pass the real toolchain sysroot explicitly. Preserve any
    // --sysroot supplied by Cargo instead of adding a conflicting one.
    if !rustc_args.iter().any(|arg| arg == "--sysroot" || arg.starts_with("--sysroot=")) {
        let sysroot = env::var("RUST_METRICS_SYSROOT")
            .expect("RUST_METRICS_SYSROOT not set by rust_analysis.py");
        rustc_args.push("--sysroot".to_owned());
        rustc_args.push(sysroot);
    }

    let mut callback = MetricsCallbacks { output_dir: output };
    let exit_code = rustc_driver::catch_with_exit_code(|| RunCompiler::new(&rustc_args, &mut callback).run());
    std::process::exit(exit_code);
}
