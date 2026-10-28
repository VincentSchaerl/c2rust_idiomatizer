use super::{
    array_len_receiver, collect_integer_type_aliases, expr_matches_loop_binding,
    integer_type_from_ty, integer_type_source, simple_loop_binding, simple_path_name, strip_parens,
    underlying_integer_type_from_ty, unsuffixed_integer_literal_value, IntValue, IntegerType,
    LoopBindingOrigins,
};

use c2rust_refactor::command::CommandState;
use c2rust_refactor::RefactorCtxt;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::ptr::P as AstP;
use rustc_ast::visit::{self, Visitor};
use rustc_ast::Ty;
use rustc_ast::{Expr, ExprKind, Item, RangeLimits};
use rustc_hir::def_id::DefId;
use rustc_hir::HirId;
use rustc_span::Span;
use std::collections::HashMap;

#[derive(Clone)]
struct LoopPlan {
    binding_pat_id: rustc_ast::NodeId,
    binding_pat_span: Span,
    induction_hir: Option<HirId>,
    induction_name: String,
    induction_ty: Option<IntegerType>,
    array_name: String,
}

pub(super) fn index_type_to_usize(
    st: &CommandState,
    cx: &RefactorCtxt,
    binding_origins: &LoopBindingOrigins,
) {
    // This pass deliberately recognizes only the for-loop forms emitted by the
    // preceding idiomatize_loops passes.  The newly created loop shells may not
    // have HIR/typeck entries yet, so matching is primarily structural while
    // preserved body expressions still use HIR when available.
    let snapshot = {
        let krate = st.krate();
        (*krate).clone()
    };

    let aliases = collect_integer_type_aliases(&snapshot, cx);
    let mut plans = Vec::new();
    struct PlanCollector<'out, 'cx, 'a, 'tcx: 'a> {
        plans: &'out mut Vec<LoopPlan>,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        binding_origins: &'out LoopBindingOrigins,
        aliases: &'out HashMap<DefId, AstP<Ty>>,
    }

    impl<'ast, 'out, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for PlanCollector<'out, 'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if let Some(plan) = analyze_loop(expr, self.cx, self.binding_origins, self.aliases) {
                self.plans.push(plan);
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, item: &'ast Item) {
            visit::walk_item(self, item);
        }
    }

    let mut collector = PlanCollector {
        plans: &mut plans,
        cx,
        binding_origins,
        aliases: &aliases,
    };
    collector.visit_crate(&snapshot);

    if plans.is_empty() {
        return;
    }

    let mut krate = st.krate_mut();
    let mut rewriter = LoopRewriter {
        st,
        cx,
        plans: &plans,
    };
    rewriter.visit_crate(&mut *krate);
}

fn analyze_loop(
    expr: &Expr,
    cx: &RefactorCtxt,
    binding_origins: &LoopBindingOrigins,
    aliases: &HashMap<DefId, AstP<Ty>>,
) -> Option<LoopPlan> {
    let (pat, iter, body) = match &expr.kind {
        ExprKind::ForLoop(pat, iter, body, _) => (pat, iter, body),
        _ => return None,
    };
    let (induction_hir, induction_name) = simple_loop_binding(pat, cx, binding_origins)?;

    let (start, end) = match &iter.kind {
        ExprKind::Range(Some(start), Some(end), RangeLimits::HalfOpen) => (start, end),
        _ => return None,
    };

    // The updated input pattern is exactly `0..N`.
    if unsuffixed_integer_literal_value(strip_parens(start)) != Some(IntValue::NonNegative(0)) {
        return None;
    }

    // Constraint 1: N is `array.len()` or `array.len() as T`.  T exists only
    // in the cast form and must be a primitive integer type.
    let (array_name, induction_ty) = array_name_and_type_from_end(end, cx, aliases)?;

    // Constraint 2: at least one array[I] or array[I as usize].
    if !body_contains_matching_access(body, &array_name, induction_hir, &induction_name, cx) {
        return None;
    }

    Some(LoopPlan {
        binding_pat_id: pat.id,
        binding_pat_span: pat.span,
        induction_hir,
        induction_name,
        induction_ty,
        array_name,
    })
}

fn array_name_and_type_from_end(
    expr: &Expr,
    cx: &RefactorCtxt,
    aliases: &HashMap<DefId, AstP<Ty>>,
) -> Option<(String, Option<IntegerType>)> {
    let expr = strip_parens(expr);
    let (len_expr, induction_ty) = match &expr.kind {
        ExprKind::Cast(inner, ty) => {
            let ty = underlying_integer_type_from_ty(ty, cx, aliases)?;
            (strip_parens(inner), Some(ty))
        }
        _ => (expr, None),
    };
    let receiver = array_len_receiver(len_expr)?;
    Some((simple_path_name(receiver)?.to_string(), induction_ty))
}

fn body_contains_matching_access(
    body: &rustc_ast::Block,
    array_name: &str,
    induction_hir: Option<HirId>,
    induction_name: &str,
    cx: &RefactorCtxt,
) -> bool {
    struct Finder<'name, 'cx, 'a, 'tcx: 'a> {
        array_name: &'name str,
        induction_hir: Option<HirId>,
        induction_name: &'name str,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        found: bool,
    }

    impl<'ast, 'name, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for Finder<'name, 'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }
            if let ExprKind::Index(base, index) = &expr.kind {
                if simple_path_name(base) == Some(self.array_name)
                    && matches_index(index, self.induction_hir, self.induction_name, self.cx)
                {
                    self.found = true;
                    return;
                }
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items are separate scopes and are not part of this loop body.
        }
    }

    let mut finder = Finder {
        array_name,
        induction_hir,
        induction_name,
        cx,
        found: false,
    };
    finder.visit_block(body);
    finder.found
}

fn matches_index(
    expr: &Expr,
    induction_hir: Option<HirId>,
    induction_name: &str,
    cx: &RefactorCtxt,
) -> bool {
    let expr = strip_parens(expr);
    if expr_matches_loop_binding(expr, induction_hir, induction_name, cx) {
        return true;
    }

    match &expr.kind {
        ExprKind::Cast(inner, ty)
            if integer_type_from_ty(ty) == Some(IntegerType::Primitive("usize")) =>
        {
            expr_matches_loop_binding(inner, induction_hir, induction_name, cx)
        }
        _ => false,
    }
}

struct LoopRewriter<'plans, 'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    plans: &'plans [LoopPlan],
}

impl<'plans, 'cx, 'a, 'tcx: 'a> MutVisitor for LoopRewriter<'plans, 'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        let plan = match &expr.kind {
            ExprKind::ForLoop(pat, _, _, _) => self
                .plans
                .iter()
                .find(|plan| plan.binding_pat_id == pat.id && plan.binding_pat_span == pat.span)
                .cloned(),
            _ => None,
        };

        if let Some(plan) = plan {
            if let ExprKind::ForLoop(_, iter, body, _) = &mut expr.kind {
                // N' is exactly array.len(), and the range start is the
                // unsuffixed literal 0 so the new induction variable is usize.
                *iter = self
                    .st
                    .parse_expr(self.cx, &format!("0..{}.len()", plan.array_name));

                let mut body_rewriter = BodyIndexTypeRewriter {
                    st: self.st,
                    cx: self.cx,
                    induction_hir: plan.induction_hir,
                    induction_name: &plan.induction_name,
                    induction_ty: plan.induction_ty,
                };
                body_rewriter.visit_block(body);
            }
            mut_visit::noop_visit_expr(expr, self);
            return;
        }

        mut_visit::noop_visit_expr(expr, self);
    }
}

struct BodyIndexTypeRewriter<'name, 'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    induction_hir: Option<HirId>,
    induction_name: &'name str,
    induction_ty: Option<IntegerType>,
}

impl<'name, 'cx, 'a, 'tcx: 'a> MutVisitor for BodyIndexTypeRewriter<'name, 'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        // The special case I as usize must be handled before visiting its I;
        // otherwise the general I -> I as T rewrite would create a redundant
        // nested cast instead of removing the usize cast.
        if let ExprKind::Cast(inner, ty) = &expr.kind {
            if integer_type_from_ty(ty) == Some(IntegerType::Primitive("usize"))
                && expr_matches_loop_binding(
                    inner,
                    self.induction_hir,
                    self.induction_name,
                    self.cx,
                )
            {
                *expr = inner.clone();
                return;
            }
        }

        if let Some(induction_ty) = self.induction_ty {
            if induction_ty != IntegerType::Primitive("usize")
                && expr_matches_loop_binding(expr, self.induction_hir, self.induction_name, self.cx)
            {
                *expr = self.st.parse_expr(
                    self.cx,
                    &format!(
                        "{} as {}",
                        self.induction_name,
                        integer_type_source(induction_ty)
                    ),
                );
                return;
            }
        }

        mut_visit::noop_visit_expr(expr, self);
    }
}
