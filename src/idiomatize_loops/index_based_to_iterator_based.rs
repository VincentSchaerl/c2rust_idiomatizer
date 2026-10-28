use super::{
    adjusted_reference_mutability, array_len_receiver, collect_pattern_binding_names,
    expr_matches_loop_binding, fresh_name, simple_loop_binding, simple_path_name, strip_parens,
    unsuffixed_integer_literal_value, IntValue, LoopBindingOrigins,
};

use c2rust_refactor::command::CommandState;
use c2rust_refactor::RefactorCtxt;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::visit::{self, Visitor};
use rustc_ast::{
    Block, Expr, ExprKind, Item, ItemKind, Local, LocalKind, Mutability, Pat, RangeLimits, StmtKind,
};
use rustc_hir::HirId;
use rustc_span::Span;

use std::collections::HashSet;

#[derive(Clone)]
struct LoopPlan {
    binding_pat_id: rustc_ast::NodeId,
    binding_pat_span: Span,
    induction_hir: Option<HirId>,
    induction_name: String,
    array_name: String,
    element_name: String,
    enumerate: bool,
    mutable: bool,
}

pub(super) fn index_based_to_iterator_based(
    st: &CommandState,
    cx: &RefactorCtxt,
    binding_origins: &LoopBindingOrigins,
) {
    let snapshot = {
        let krate = st.krate();
        (*krate).clone()
    };

    let mut plans = Vec::new();
    collect_function_plans(&snapshot.items, cx, binding_origins, &mut plans);
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

fn collect_function_plans(
    items: &[P<Item>],
    cx: &RefactorCtxt,
    binding_origins: &LoopBindingOrigins,
    plans: &mut Vec<LoopPlan>,
) {
    // Item names in the containing module are resolvable throughout each
    // function in that module. Keep module scopes separate so an unrelated
    // item in another module does not force an unnecessary element_N name.
    let module_names = items
        .iter()
        .map(|item| item.ident.name.to_string())
        .collect::<HashSet<_>>();

    for item in items {
        match &item.kind {
            ItemKind::Fn(func) => {
                if let Some(body) = &func.body {
                    let mut visible_names = module_names.clone();
                    for param in &func.sig.decl.inputs {
                        collect_pattern_binding_names(&param.pat, &mut visible_names);
                    }
                    let mut analyzer = FunctionAnalyzer {
                        visible_names,
                        cx,
                        binding_origins,
                        plans,
                    };
                    analyzer.visit_block(body);
                }
            }
            ItemKind::Mod(_, rustc_ast::ModKind::Loaded(module_items, ..)) => {
                collect_function_plans(module_items, cx, binding_origins, plans);
            }
            _ => {}
        }
    }
}

struct FunctionAnalyzer<'out, 'cx, 'a, 'tcx: 'a> {
    visible_names: HashSet<String>,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    binding_origins: &'out LoopBindingOrigins,
    plans: &'out mut Vec<LoopPlan>,
}

impl<'out, 'cx, 'a, 'tcx: 'a> FunctionAnalyzer<'out, 'cx, 'a, 'tcx> {
    fn visit_local_initializer(&mut self, local: &Local) {
        match &local.kind {
            LocalKind::Init(init) => self.visit_expr(init),
            LocalKind::InitElse(init, else_block) => {
                self.visit_expr(init);
                self.visit_block(else_block);
            }
            LocalKind::Decl => {}
        }
    }
}

impl<'ast, 'out, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for FunctionAnalyzer<'out, 'cx, 'a, 'tcx> {
    fn visit_block(&mut self, block: &'ast Block) {
        let names_at_entry = self.visible_names.clone();

        // Block items are visible throughout their containing block.
        for stmt in &block.stmts {
            if let StmtKind::Item(item) = &stmt.kind {
                self.visible_names.insert(item.ident.name.to_string());
            }
        }

        for stmt in &block.stmts {
            match &stmt.kind {
                StmtKind::Local(local) => {
                    self.visit_local_initializer(local);
                    collect_pattern_binding_names(&local.pat, &mut self.visible_names);
                }
                StmtKind::Item(_) => {
                    // Nested items are separate scopes and are analyzed by the
                    // outer item traversal, not with this function's locals.
                }
                StmtKind::Expr(expr) | StmtKind::Semi(expr) => self.visit_expr(expr),
                _ => visit::walk_stmt(self, stmt),
            }
        }

        self.visible_names = names_at_entry;
    }

    fn visit_expr(&mut self, expr: &'ast Expr) {
        if let ExprKind::ForLoop(pat, iter, body, _) = &expr.kind {
            let mut names_for_loop = self.visible_names.clone();
            collect_pattern_binding_names(pat, &mut names_for_loop);
            collect_names_in_loop_body(body, &mut names_for_loop);

            if let Some(plan) = analyze_loop(expr, &names_for_loop, self.cx, self.binding_origins) {
                self.plans.push(plan);
            }

            // The for binding is not in scope in the iterator expression, but
            // it is in scope throughout the body and any nested loops.
            self.visit_expr(iter);
            let names_before_body = self.visible_names.clone();
            collect_pattern_binding_names(pat, &mut self.visible_names);
            self.visit_block(body);
            self.visible_names = names_before_body;
            return;
        }

        if matches!(expr.kind, ExprKind::Closure(..)) {
            // The closure has a separate local binding environment. Names used
            // in a candidate outer loop are already considered by its body-name
            // collection, so do not leak closure locals into this function scope.
            return;
        }

        visit::walk_expr(self, expr);
    }

    fn visit_item(&mut self, _item: &'ast Item) {
        // Nested items are separate scopes.
    }
}

fn collect_names_in_loop_body(body: &Block, names: &mut HashSet<String>) {
    struct Collector<'names> {
        names: &'names mut HashSet<String>,
    }

    impl<'ast, 'names> Visitor<'ast> for Collector<'names> {
        fn visit_pat(&mut self, pat: &'ast Pat) {
            collect_pattern_binding_names(pat, self.names);
            // collect_pattern_binding_names already walks the complete pattern.
        }

        fn visit_expr(&mut self, expr: &'ast Expr) {
            if let Some(name) = simple_path_name(expr) {
                self.names.insert(name.to_string());
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, item: &'ast Item) {
            self.names.insert(item.ident.name.to_string());
            // The nested item's internal bindings do not share the loop body scope.
        }
    }

    let mut collector = Collector { names };
    collector.visit_block(body);
}

fn analyze_loop(
    expr: &Expr,
    names_for_loop: &HashSet<String>,
    cx: &RefactorCtxt,
    binding_origins: &LoopBindingOrigins,
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
    if unsuffixed_integer_literal_value(strip_parens(start)) != Some(IntValue::NonNegative(0)) {
        return None;
    }

    let receiver = array_len_receiver(end)?;
    let array_name = simple_path_name(receiver)?.to_string();

    let usage = analyze_body_usage(body, &array_name, induction_hir, &induction_name, cx);
    if !usage.valid || usage.access_count == 0 {
        return None;
    }

    let element_name = fresh_name("element", names_for_loop);
    Some(LoopPlan {
        binding_pat_id: pat.id,
        binding_pat_span: pat.span,
        induction_hir,
        induction_name,
        array_name,
        element_name,
        enumerate: usage.other_induction_use,
        mutable: usage.modified_access,
    })
}

#[derive(Default)]
struct BodyUsage {
    valid: bool,
    access_count: usize,
    other_induction_use: bool,
    modified_access: bool,
}

fn analyze_body_usage(
    body: &Block,
    array_name: &str,
    induction_hir: Option<HirId>,
    induction_name: &str,
    cx: &RefactorCtxt,
) -> BodyUsage {
    struct UsageVisitor<'name, 'cx, 'a, 'tcx: 'a> {
        array_name: &'name str,
        induction_hir: Option<HirId>,
        induction_name: &'name str,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        modified_context: bool,
        usage: BodyUsage,
    }

    impl<'name, 'cx, 'a, 'tcx: 'a> UsageVisitor<'name, 'cx, 'a, 'tcx> {
        fn visit_with_modified<'ast>(&mut self, expr: &'ast Expr, modified: bool) {
            let old = self.modified_context;
            self.modified_context = modified;
            self.visit_expr(expr);
            self.modified_context = old;
        }

        fn valid_array_index(&self, expr: &Expr) -> bool {
            let (base, index) = match &expr.kind {
                ExprKind::Index(base, index) => (base, index),
                _ => return false,
            };
            simple_path_name(base) == Some(self.array_name)
                && expr_matches_loop_binding(
                    index,
                    self.induction_hir,
                    self.induction_name,
                    self.cx,
                )
        }
    }

    impl<'ast, 'name, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for UsageVisitor<'name, 'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            match &expr.kind {
                ExprKind::Assign(lhs, rhs, _) => {
                    self.visit_with_modified(lhs, true);
                    self.visit_with_modified(rhs, false);
                    return;
                }
                ExprKind::AssignOp(_, lhs, rhs) => {
                    self.visit_with_modified(lhs, true);
                    self.visit_with_modified(rhs, false);
                    return;
                }
                ExprKind::AddrOf(_, Mutability::Mut, operand) => {
                    self.visit_with_modified(operand, true);
                    return;
                }
                _ => {}
            }

            if self.valid_array_index(expr) {
                self.usage.access_count += 1;
                if self.modified_context
                    || adjusted_reference_mutability(expr, self.cx) == Some(Mutability::Mut)
                {
                    self.usage.modified_access = true;
                }
                // Do not descend into the valid access: its array occurrence is
                // permitted, and its I occurrence is precisely the index form
                // excluded from the enumerate decision.
                return;
            }

            if simple_path_name(expr) == Some(self.array_name) {
                self.usage.valid = false;
                return;
            }

            if expr_matches_loop_binding(expr, self.induction_hir, self.induction_name, self.cx) {
                self.usage.other_induction_use = true;
                return;
            }

            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items are not part of the loop body's variable uses.
        }
    }

    let mut visitor = UsageVisitor {
        array_name,
        induction_hir,
        induction_name,
        cx,
        modified_context: false,
        usage: BodyUsage {
            valid: true,
            ..BodyUsage::default()
        },
    };
    visitor.visit_block(body);
    visitor.usage
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
            rewrite_loop(expr, &plan, self.st, self.cx);
            mut_visit::noop_visit_expr(expr, self);
            return;
        }

        mut_visit::noop_visit_expr(expr, self);
    }
}

fn rewrite_loop(expr: &mut P<Expr>, plan: &LoopPlan, st: &CommandState, cx: &RefactorCtxt) {
    let (pat, iter, body) = match &mut expr.kind {
        ExprKind::ForLoop(pat, iter, body, _) => (pat, iter, body),
        _ => return,
    };

    let iterator_method = if plan.mutable { "iter_mut" } else { "iter" };
    let iter_source = if plan.enumerate {
        format!("{}.{}().enumerate()", plan.array_name, iterator_method)
    } else {
        format!("{}.{}()", plan.array_name, iterator_method)
    };
    *iter = st.parse_expr(cx, &iter_source);

    let pat_source = if plan.enumerate {
        format!("({}, {})", plan.induction_name, plan.element_name)
    } else {
        plan.element_name.clone()
    };
    let parsed = st.parse_expr(cx, &format!("for {} in 0..0 {{}}", pat_source));
    if let ExprKind::ForLoop(parsed_pat, _, _, _) = &parsed.kind {
        *pat = parsed_pat.clone();
    } else {
        return;
    }

    let mut body_rewriter = ArrayIndexRewriter {
        st,
        cx,
        induction_hir: plan.induction_hir,
        induction_name: &plan.induction_name,
        array_name: &plan.array_name,
        element_name: &plan.element_name,
    };
    body_rewriter.visit_block(body);
}

struct ArrayIndexRewriter<'name, 'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    induction_hir: Option<HirId>,
    induction_name: &'name str,
    array_name: &'name str,
    element_name: &'name str,
}

impl<'name, 'cx, 'a, 'tcx: 'a> MutVisitor for ArrayIndexRewriter<'name, 'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        let replace = match &expr.kind {
            ExprKind::Index(base, index) => {
                simple_path_name(base) == Some(self.array_name)
                    && expr_matches_loop_binding(
                        index,
                        self.induction_hir,
                        self.induction_name,
                        self.cx,
                    )
            }
            _ => false,
        };

        if replace {
            *expr = self
                .st
                .parse_expr(self.cx, &format!("*{}", self.element_name));
            return;
        }

        mut_visit::noop_visit_expr(expr, self);
    }
}
