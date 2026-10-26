use super::{
    array_type_with_literal_length, collect_public_integer_consts,
    collect_integer_type_aliases, expr_matches_loop_binding,
    underlying_integer_type_from_ty,
    integer_type_source, integer_value_fits_type, simple_loop_binding, strip_parens,
    suffixed_zero_integer_type, unsuffixed_integer_literal_value, IntegerConstMeta, IntValue,
    IntegerType, TargetInfo, LoopBindingOrigins,
};

use c2rust_refactor::command::CommandState;
use c2rust_refactor::RefactorCtxt;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::visit::{self, Visitor};
use rustc_ast::{
    BindingAnnotation, Block, ByRef, Crate, Expr, ExprKind, Item, ItemKind, Local, LocalKind,
    Mutability, Pat, PatKind, RangeLimits, StmtKind, TyKind,
};
use rustc_hir::def::Res;
use rustc_hir::def_id::DefId;
use rustc_hir::HirId;
use rustc_span::Span;

use std::collections::HashMap;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
enum ArrayKey {
    Local(HirId),
    Item(DefId),
}

#[derive(Clone, Copy, Debug)]
struct ArrayMeta {
    length: u128,
}

#[derive(Default)]
struct GlobalInfo {
    consts: HashMap<DefId, IntegerConstMeta>,
    aliases: HashMap<DefId, P<rustc_ast::Ty>>,
    arrays: HashMap<ArrayKey, ArrayMeta>,
}

#[derive(Clone)]
struct LoopPlan {
    binding_pat_id: rustc_ast::NodeId,
    binding_pat_span: Span,
    induction_ty: IntegerType,
    array_expr: P<Expr>,
}

pub(super) fn iterate_arrays_idiomatically(
    st: &CommandState,
    cx: &RefactorCtxt,
    target: TargetInfo,
    binding_origins: &LoopBindingOrigins,
) {
    // `initial_transformation` has already run in this Phase-3 command.  rustc's
    // semantic tables still describe the pre-rewrite crate, but the loop body
    // expressions and reused induction binding keep the original NodeIds/HirIds.
    let snapshot = {
        let krate = st.krate();
        (*krate).clone()
    };

    let global_info = collect_global_info(&snapshot, cx, target);
    let mut plans = Vec::new();

    collect_function_loop_plans(&snapshot.items, &global_info, cx, target, binding_origins, &mut plans);
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

fn collect_global_info(krate: &Crate, cx: &RefactorCtxt, target: TargetInfo) -> GlobalInfo {
    let aliases = collect_integer_type_aliases(krate, cx);
    let consts = collect_public_integer_consts(krate, cx, target, &aliases);
    let mut arrays = HashMap::new();
    collect_global_arrays_from_items(&krate.items, cx, &mut arrays);
    GlobalInfo { consts, arrays, aliases }
}

fn collect_global_arrays_from_items(
    items: &[P<Item>],
    cx: &RefactorCtxt,
    arrays: &mut HashMap<ArrayKey, ArrayMeta>,
) {
    for item in items {
        match &item.kind {
            // Constraint 2 explicitly excludes `static mut`. A non-mutable
            // static must have an initializer here so this AST item is a
            // definition, not merely a foreign declaration.
            ItemKind::Static(ty, mutability, Some(_)) if *mutability == Mutability::Not => {
                if let Some(length) = one_dimensional_array_length(ty) {
                    arrays.insert(
                        ArrayKey::Item(cx.node_def_id(item.id)),
                        ArrayMeta { length },
                    );
                }
            }
            ItemKind::Const(_, ty, Some(_)) => {
                if let Some(length) = one_dimensional_array_length(ty) {
                    arrays.insert(
                        ArrayKey::Item(cx.node_def_id(item.id)),
                        ArrayMeta { length },
                    );
                }
            }
            ItemKind::Mod(_, rustc_ast::ModKind::Loaded(module_items, ..)) => {
                collect_global_arrays_from_items(module_items, cx, arrays);
            }
            _ => {}
        }
    }
}

fn collect_function_loop_plans(
    items: &[P<Item>],
    global_info: &GlobalInfo,
    cx: &RefactorCtxt,
    target: TargetInfo,
    binding_origins: &LoopBindingOrigins,
    plans: &mut Vec<LoopPlan>,
) {
    for item in items {
        match &item.kind {
            ItemKind::Fn(func) => {
                if let Some(body) = &func.body {
                    let mut analyzer = FunctionLoopAnalyzer {
                        arrays: global_info.arrays.clone(),
                        consts: &global_info.consts,
                        aliases: &global_info.aliases,
                        cx,
                        target,
                        binding_origins,
                        plans,
                    };
                    analyzer.visit_block(body);
                }
            }
            ItemKind::Mod(_, rustc_ast::ModKind::Loaded(module_items, ..)) => {
                collect_function_loop_plans(module_items, global_info, cx, target, binding_origins, plans);
            }
            _ => {}
        }
    }
}

struct FunctionLoopAnalyzer<'data, 'out, 'cx, 'a, 'tcx: 'a> {
    // This map is deliberately maintained in source order. At each loop it
    // contains exactly the global arrays plus arrays declared in the same
    // function and in lexical scope *before* that loop. This implements
    // constraint 2 without relying on spans of the newly generated `for` node.
    arrays: HashMap<ArrayKey, ArrayMeta>,
    consts: &'data HashMap<DefId, IntegerConstMeta>,
    aliases: &'data HashMap<DefId, P<rustc_ast::Ty>>,
    binding_origins: &'data LoopBindingOrigins,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    target: TargetInfo,
    plans: &'out mut Vec<LoopPlan>,
}

impl<'data, 'out, 'cx, 'a, 'tcx: 'a> FunctionLoopAnalyzer<'data, 'out, 'cx, 'a, 'tcx> {
    fn add_local_array(&mut self, local: &Local) {
        let ty = match &local.ty {
            Some(ty) => ty,
            None => return,
        };
        let length = match one_dimensional_array_length(ty) {
            Some(length) => length,
            None => return,
        };
        if !matches!(&local.kind, LocalKind::Init(_) | LocalKind::InitElse(_, _)) {
            return;
        }
        let hir_id = match simple_local_binding_hir(&local.pat, self.cx) {
            Some(hir_id) => hir_id,
            None => return,
        };
        self.arrays
            .insert(ArrayKey::Local(hir_id), ArrayMeta { length });
    }

    fn add_function_local_item_array(&mut self, item: &Item) {
        let length = match &item.kind {
            ItemKind::Static(ty, mutability, Some(_)) if *mutability == Mutability::Not => {
                one_dimensional_array_length(ty)
            }
            ItemKind::Const(_, ty, Some(_)) => one_dimensional_array_length(ty),
            _ => None,
        };
        if let Some(length) = length {
            self.arrays.insert(
                ArrayKey::Item(self.cx.node_def_id(item.id)),
                ArrayMeta { length },
            );
        }
    }
}

impl<'ast, 'data, 'out, 'cx, 'a, 'tcx: 'a> Visitor<'ast>
    for FunctionLoopAnalyzer<'data, 'out, 'cx, 'a, 'tcx>
{
    fn visit_block(&mut self, block: &'ast Block) {
        // A nested lexical block inherits all arrays available at entry, but
        // declarations made inside it must not leak back out.
        let arrays_at_entry = self.arrays.clone();

        for stmt in &block.stmts {
            match &stmt.kind {
                StmtKind::Local(local) => {
                    // Any loop in an initializer / let-else block occurs before
                    // this binding has been introduced, so analyze those first.
                    match &local.kind {
                        LocalKind::Init(init) => self.visit_expr(init),
                        LocalKind::InitElse(init, else_block) => {
                            self.visit_expr(init);
                            self.visit_block(else_block);
                        }
                        LocalKind::Decl => {}
                    }
                    self.add_local_array(local);
                }
                StmtKind::Item(item) => {
                    // Constraint 2 says "before the for loop" even for block
                    // items, whose Rust name scope itself is not source ordered.
                    // Therefore make the array available only after its item
                    // statement has been reached. Nested item bodies are separate
                    // functions/items and are not traversed with this function's
                    // local-array environment.
                    self.add_function_local_item_array(item);
                }
                StmtKind::Expr(expr) | StmtKind::Semi(expr) => self.visit_expr(expr),
                _ => visit::walk_stmt(self, stmt),
            }
        }

        self.arrays = arrays_at_entry;
    }

    fn visit_expr(&mut self, expr: &'ast Expr) {
        if matches!(expr.kind, ExprKind::Closure(..)) {
            // A closure has its own body and cannot use the surrounding
            // function's local arrays as arrays "within the same function".
            return;
        }

        if let Some(plan) = analyze_loop(expr, &self.arrays, self.consts, self.aliases, self.cx, self.target, self.binding_origins) {
            self.plans.push(plan);
        }

        visit::walk_expr(self, expr);
    }

    fn visit_item(&mut self, _item: &'ast Item) {
        // Nested items are separate function/item bodies.
    }
}

fn one_dimensional_array_length(ty: &rustc_ast::Ty) -> Option<u128> {
    let (element_ty, length) = array_type_with_literal_length(ty)?;
    if matches!(&element_ty.kind, TyKind::Array(..)) {
        return None;
    }
    Some(length)
}

fn simple_local_binding_hir(pat: &Pat, cx: &RefactorCtxt) -> Option<HirId> {
    match pat.kind {
        PatKind::Ident(BindingAnnotation(ByRef::No, _), _, None) => {
            cx.hir_map().opt_node_to_hir_id(pat.id)
        }
        _ => None,
    }
}

fn analyze_loop(
    expr: &Expr,
    arrays: &HashMap<ArrayKey, ArrayMeta>,
    consts: &HashMap<DefId, IntegerConstMeta>,
    aliases: &HashMap<DefId, P<rustc_ast::Ty>>,
    cx: &RefactorCtxt,
    target: TargetInfo,
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
    let induction_ty = zero_start_type(start, cx, aliases)?;

    let accesses = collect_matching_array_accesses(
        body,
        induction_hir,
        &induction_name,
        arrays,
        aliases,
        cx,
    );

    for (array_expr, array_meta) in accesses {
        if valid_end_bound(end, array_meta.length, induction_ty, consts, aliases, cx, target) {
            return Some(LoopPlan {
                binding_pat_id: pat.id,
                binding_pat_span: pat.span,
                induction_ty,
                array_expr,
            });
        }
    }

    None
}

fn zero_start_type(
    expr: &Expr, cx: &RefactorCtxt, aliases: &HashMap<DefId, P<rustc_ast::Ty>>,
) -> Option<IntegerType> {
    let expr = strip_parens(expr);

    // Constraint 1 permits exactly `0T` or `0 as T`.  The former is handled
    // by the shared suffixed-literal helper; the latter must contain an
    // unsuffixed zero as its operand.
    if let Some(ty) = suffixed_zero_integer_type(expr) {
        return Some(ty);
    }

    match &expr.kind {
        ExprKind::Cast(inner, ty)
            if unsuffixed_integer_literal_value(strip_parens(inner))
                == Some(IntValue::NonNegative(0)) =>
        {
            underlying_integer_type_from_ty(ty, cx, aliases)
        }
        _ => None,
    }
}

fn collect_matching_array_accesses(
    body: &rustc_ast::Block,
    induction_hir: Option<HirId>,
    induction_name: &str,
    arrays: &HashMap<ArrayKey, ArrayMeta>,
    aliases: &HashMap<DefId, P<rustc_ast::Ty>>,
    cx: &RefactorCtxt,
) -> Vec<(P<Expr>, ArrayMeta)> {
    struct AccessCollector<'arrays, 'cx, 'a, 'tcx: 'a> {
        induction_hir: Option<HirId>,
        induction_name: &'arrays str,
        arrays: &'arrays HashMap<ArrayKey, ArrayMeta>,
        aliases: &'arrays HashMap<DefId, P<rustc_ast::Ty>>,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        found: Vec<(P<Expr>, ArrayMeta)>,
    }

    impl<'ast, 'arrays, 'cx, 'a, 'tcx: 'a> Visitor<'ast>
        for AccessCollector<'arrays, 'cx, 'a, 'tcx>
    {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if matches!(expr.kind, ExprKind::Closure(..)) {
                return;
            }

            if let ExprKind::Index(base, index) = &expr.kind {
                if matches_loop_index(
                    index,
                    self.induction_hir,
                    self.induction_name,
                    self.cx,
                    self.aliases,
                ) {
                    let base_expr = strip_parens(base);
                    if let Some(key) = array_key_for_expr(base_expr, self.cx) {
                        if let Some(meta) = self.arrays.get(&key).copied() {
                            self.found.push((base.clone(), meta));
                        }
                    }
                }
            }

            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {}
    }

    let mut collector = AccessCollector {
        induction_hir,
        induction_name,
        arrays,
        aliases,
        cx,
        found: Vec::new(),
    };
    collector.visit_block(body);
    collector.found
}

fn matches_loop_index(
    expr: &Expr,
    induction_hir: Option<HirId>,
    induction_name: &str,
    cx: &RefactorCtxt,
    aliases: &HashMap<DefId, P<rustc_ast::Ty>>,
) -> bool {
    let expr = strip_parens(expr);
    if expr_matches_loop_binding(expr, induction_hir, induction_name, cx) {
        return true;
    }

    let (inner, ty) = match &expr.kind {
        ExprKind::Cast(inner, ty) => (strip_parens(inner), ty),
        _ => return false,
    };
    underlying_integer_type_from_ty(ty, cx, aliases) == Some(IntegerType::Primitive("usize"))
        && expr_matches_loop_binding(inner, induction_hir, induction_name, cx)
}

fn valid_end_bound(
    expr: &Expr,
    array_length: u128,
    induction_ty: IntegerType,
    consts: &HashMap<DefId, IntegerConstMeta>,
    aliases: &HashMap<DefId, P<rustc_ast::Ty>>,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> bool {
    let length_value = IntValue::NonNegative(array_length);
    if unsuffixed_integer_literal_value(strip_parens(expr)) == Some(length_value) {
        return integer_value_fits_type(length_value, induction_ty, target);
    }

    let mut current = strip_parens(expr);
    let mut casts = Vec::new();
    while let ExprKind::Cast(inner, ty) = &current.kind {
        let cast_ty = match underlying_integer_type_from_ty(ty, cx, aliases) {
            Some(cast_ty) => cast_ty,
            None => return false,
        };
        casts.push(cast_ty);
        if casts.len() > 2 {
            return false;
        }
        current = strip_parens(inner);
    }

    let const_id = match cx.try_resolve_expr_hir(current) {
        Some(Res::Def(_, id)) => id,
        _ => return false,
    };
    let meta = match consts.get(&const_id) {
        Some(meta) if meta.value == length_value => meta,
        _ => return false,
    };

    match casts.as_slice() {
        [outer] if *outer != induction_ty => return false,
        [outer, _inner] if *outer != induction_ty => return false,
        [] | [_] | [_, _] => {}
        _ => return false,
    }

    integer_value_fits_type(meta.value, meta.integer_ty, target)
        && integer_value_fits_type(meta.value, induction_ty, target)
        && casts
            .iter()
            .all(|ty| integer_value_fits_type(meta.value, *ty, target))
}

fn array_key_for_expr(expr: &Expr, cx: &RefactorCtxt) -> Option<ArrayKey> {
    match cx.try_resolve_expr_hir(strip_parens(expr))? {
        Res::Local(id) => Some(ArrayKey::Local(id)),
        Res::Def(_, id) => Some(ArrayKey::Item(id)),
        _ => None,
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
                .find(|plan| {
                    plan.binding_pat_id == pat.id && plan.binding_pat_span == pat.span
                })
                .cloned(),
            _ => None,
        };

        if let Some(plan) = plan {
            if let ExprKind::ForLoop(_, iter, _, _) = &mut expr.kind {
                if let ExprKind::Range(Some(start), Some(end), RangeLimits::HalfOpen) = &mut iter.kind {
                    // The updated transformation always normalizes the range
                    // start to the unsuffixed literal `0`.
                    *start = self.st.parse_expr(self.cx, "0");
                    *end = make_array_len_expr(
                        self.st,
                        self.cx,
                        plan.array_expr,
                        plan.induction_ty,
                    );
                }
            }
        }

        mut_visit::noop_visit_expr(expr, self);
    }
}

fn make_array_len_expr(
    st: &CommandState,
    cx: &RefactorCtxt,
    array_expr: P<Expr>,
    induction_ty: IntegerType,
) -> P<Expr> {
    let source = if induction_ty == IntegerType::Primitive("usize") {
        "__c2rust_array.len()".to_string()
    } else {
        format!(
            "__c2rust_array.len() as {}",
            integer_type_source(induction_ty)
        )
    };
    let mut result = st.parse_expr(cx, &source);

    let method = match &mut result.kind {
        ExprKind::MethodCall(_, receiver, _, _) => Some(receiver),
        ExprKind::Cast(inner, _) => match &mut inner.kind {
            ExprKind::MethodCall(_, receiver, _, _) => Some(receiver),
            _ => None,
        },
        _ => None,
    };
    if let Some(receiver) = method {
        *receiver = array_expr;
    }

    result
}
