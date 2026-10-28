use super::{
    adjusted_reference_mutability, array_type_with_literal_length, collect_integer_type_aliases,
    collect_pattern_binding_names, collect_public_integer_consts, fresh_name, integer_type_from_ty,
    integer_type_range_is_subset, integer_type_source, integer_value_fits_type,
    record_pattern_binding_name, simple_loop_binding, simple_path_name, strip_parens,
    underlying_integer_type_from_ty, unsuffixed_integer_literal_value, IntValue, IntegerConstMeta,
    IntegerType, LoopBindingOrigins, TargetInfo,
};

use c2rust_refactor::ast_manip::visit_nodes;
use c2rust_refactor::command::CommandState;
use c2rust_refactor::matcher::{MatchCtxt, Subst};
use c2rust_refactor::RefactorCtxt;

use rustc_ast::mut_visit::{self, MutVisitor};
use rustc_ast::ptr::P;
use rustc_ast::visit::{self, Visitor};
use rustc_ast::{
    AngleBracketedArg, BindingAnnotation, BorrowKind, ByRef, Crate, Expr, ExprKind, Extern,
    FnRetTy, ForeignItemKind, GenericArg, GenericArgs, IntTy, Item, ItemKind, LitIntType, LitKind,
    Local, LocalKind, Mutability, Pat, PatKind, RangeLimits, Ty, TyKind, UintTy, Unsafe,
};
use rustc_hir::def::Res;
use rustc_hir::def_id::DefId;
use rustc_hir::HirId;
use rustc_middle::ty::{self, ParamEnv};
use rustc_span::{symbol::Symbol, Span};

use std::collections::{HashMap, HashSet};

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
enum ArrayKey {
    Local(HirId),
    Item(DefId),
}

#[derive(Clone)]
struct ArrayMeta {
    element_ty: P<Ty>,
    length: u128,
}

#[derive(Clone)]
struct LoopPlan {
    induction_name: String,
    induction_ty: IntegerType,
    length_use_id: rustc_ast::NodeId,
    pointer_access_ids: HashSet<rustc_ast::NodeId>,
    allowed_array_path_ids: HashSet<rustc_ast::NodeId>,
}

#[derive(Clone)]
struct CallPlan {
    call_id: rustc_ast::NodeId,
    caller_body_id: rustc_ast::NodeId,
    call_span: Span,
    array_index: usize,
    length_index: usize,
    array_key: ArrayKey,
    exception_addr_id: rustc_ast::NodeId,
    array_expr: P<Expr>,
}

#[derive(Clone)]
struct FunctionPlan {
    item_id: rustc_ast::NodeId,
    def_id: DefId,
    foreign_decl_item_ids: HashSet<rustc_ast::NodeId>,
    array_index: usize,
    length_index: usize,
    array_name: String,
    pointer_mutability: Mutability,
    element_ty: P<Ty>,
    length_name: String,
    restored_length_name: String,
    restored_length_use_ids: HashSet<rustc_ast::NodeId>,
    length_ty: P<Ty>,
    length_integer_ty: IntegerType,
    restore_length: bool,
    loops: Vec<LoopPlan>,
    calls: Vec<CallPlan>,
}

#[derive(Default)]
struct GlobalInfo {
    all_aliases: HashMap<DefId, P<Ty>>,
    consts: HashMap<DefId, IntegerConstMeta>,
}

pub(super) fn pass_arrays_as_slices(
    st: &CommandState,
    cx: &RefactorCtxt,
    target: TargetInfo,
    transform_public_functions: bool,
    binding_origins: &LoopBindingOrigins,
) {
    // Phase-3 semantic tables still describe the pre-rewrite crate.  The
    // initial transformation deliberately preserves the parameter, call, and
    // loop-body nodes that this pass resolves through HIR, while the newly
    // created `for` shell is recognized structurally below.
    let snapshot = {
        let krate = st.krate();
        (*krate).clone()
    };

    let global_info = collect_global_info(&snapshot, cx, target);
    let mut arrays = HashMap::new();
    collect_local_arrays(&snapshot, cx, &mut arrays);

    let mut plans = Vec::new();
    visit_nodes(&snapshot, |item: &Item| {
        if let Some(plan) = analyze_function(
            item,
            &snapshot,
            &global_info,
            &arrays,
            cx,
            target,
            transform_public_functions,
            binding_origins,
        ) {
            plans.push(plan);
        }
    });

    if plans.is_empty() {
        return;
    }

    // A single function can theoretically contain several pointer/length pairs.
    // Rewriting two independent signatures in one pass is possible, but an
    // ambiguous pair would make call-site argument removal order-sensitive.
    // Keep only functions for which analysis produced exactly one plan.
    let mut counts = HashMap::<DefId, usize>::new();
    for plan in &plans {
        *counts.entry(plan.def_id).or_insert(0) += 1;
    }
    plans.retain(|plan| counts.get(&plan.def_id) == Some(&1));
    if plans.is_empty() {
        return;
    }

    // Constraint 7.d is independent of whether an earlier callee is itself
    // transformed.  Compute the direct-call raw-address expressions whose
    // corresponding callee parameter is proven non-escaping, then validate
    // every candidate against that stable fact set.
    filter_plans_by_aliasing(&snapshot, &mut plans, cx);
    if plans.is_empty() {
        return;
    }

    let plans_by_item = plans
        .iter()
        .cloned()
        .map(|plan| (plan.item_id, plan))
        .collect::<HashMap<_, _>>();
    let mut calls_by_id = HashMap::new();
    let mut plans_by_foreign_item = HashMap::new();
    for plan in &plans {
        for call in &plan.calls {
            calls_by_id.insert(call.call_id, (plan.clone(), call.clone()));
        }
        for foreign_id in &plan.foreign_decl_item_ids {
            plans_by_foreign_item.insert(*foreign_id, plan.clone());
        }
    }

    let mut krate = st.krate_mut();
    rewrite_planned_functions(
        &mut krate.items,
        &plans_by_item,
        &plans_by_foreign_item,
        st,
        cx,
    );

    let mut call_rewriter = ApplyCallSites {
        st,
        cx,
        calls_by_id,
    };
    call_rewriter.visit_crate(&mut *krate);
}

fn collect_global_info(krate: &Crate, cx: &RefactorCtxt, target: TargetInfo) -> GlobalInfo {
    let all_aliases = collect_integer_type_aliases(krate, cx);
    let consts = collect_public_integer_consts(krate, cx, target, &all_aliases);
    GlobalInfo {
        all_aliases,
        consts,
    }
}

fn collect_local_arrays(
    krate: &Crate,
    cx: &RefactorCtxt,
    arrays: &mut HashMap<ArrayKey, ArrayMeta>,
) {
    visit_nodes(krate, |local: &Local| {
        let ty = match &local.ty {
            Some(ty) => ty,
            None => return,
        };
        let (element_ty, length) = match array_type_with_literal_length(ty) {
            Some(value) => value,
            None => return,
        };
        let init = match &local.kind {
            LocalKind::Init(init) => init,
            _ => return,
        };
        if !array_initializer(init) {
            return;
        }
        let (hir_id, _, _) = match simple_binding(&local.pat, cx) {
            Some(value) => value,
            None => return,
        };
        let key = ArrayKey::Local(hir_id);
        arrays.insert(key, ArrayMeta { element_ty, length });
    });
}

fn array_initializer(expr: &Expr) -> bool {
    matches!(expr.kind, ExprKind::Array(_) | ExprKind::Repeat(..))
}

fn analyze_function(
    item: &Item,
    krate: &Crate,
    global_info: &GlobalInfo,
    arrays: &HashMap<ArrayKey, ArrayMeta>,
    cx: &RefactorCtxt,
    target: TargetInfo,
    transform_public_functions: bool,
    binding_origins: &LoopBindingOrigins,
) -> Option<FunctionPlan> {
    let func = match &item.kind {
        ItemKind::Fn(func) => func,
        _ => return None,
    };
    let body = func.body.as_ref()?;
    if !function_is_unsafe_extern_c(func) {
        return None;
    }

    let is_public = item_contains_pub(item);
    if is_public && !transform_public_functions {
        return None;
    }

    let def_id = cx.node_def_id(item.id);
    let (related_def_ids, foreign_decl_item_ids) = if is_public {
        collect_public_function_linkage_group(item, func, krate, cx)?
    } else {
        let mut ids = HashSet::new();
        ids.insert(def_id);
        (ids, HashSet::new())
    };

    // Constraint 1: after the signature changes, every in-crate use of the
    // definition and of every related foreign declaration must be a direct call.
    if !functions_are_used_only_as_direct_callees(krate, &related_def_ids, cx) {
        return None;
    }

    let mut pair_plans = Vec::new();
    for (array_index, array_param) in func.sig.decl.inputs.iter().enumerate() {
        let (array_hir, array_name, _) = match simple_binding(&array_param.pat, cx) {
            Some(value) => value,
            None => continue,
        };
        let (pointer_mutability, element_ty) = match &array_param.ty.kind {
            TyKind::Ptr(mut_ty) => (mut_ty.mutbl, mut_ty.ty.clone()),
            _ => continue,
        };

        for (length_index, length_param) in func.sig.decl.inputs.iter().enumerate() {
            if length_index == array_index {
                continue;
            }
            let (length_hir, length_name, _) = match simple_binding(&length_param.pat, cx) {
                Some(value) => value,
                None => continue,
            };
            let length_integer_ty = match underlying_integer_type_from_ty(
                &length_param.ty,
                cx,
                &global_info.all_aliases,
            ) {
                Some(value) => value,
                None => continue,
            };

            if !local_is_read_only_and_unborrowed(body, length_hir, cx)
                || local_has_forbidden_reference_syntax(body, length_hir, cx)
            {
                continue;
            }

            let mut loops = Vec::new();
            visit_nodes(&**body, |expr: &Expr| {
                if let Some(loop_plan) = analyze_loop(
                    expr,
                    array_hir,
                    length_hir,
                    length_integer_ty,
                    pointer_mutability,
                    &element_ty,
                    cx,
                    target,
                    binding_origins,
                    &global_info.all_aliases,
                ) {
                    loops.push(loop_plan);
                }
            });
            if loops.is_empty() {
                continue;
            }

            // Constraints 2 and 6: every use of the pointer parameter must be
            // consumed by one of the validated pointer-indexing forms inside a
            // matching loop.
            let allowed = loops
                .iter()
                .flat_map(|lp| lp.allowed_array_path_ids.iter().copied())
                .collect::<HashSet<_>>();
            let all_array_uses = collect_local_path_uses(&**body, array_hir, cx);
            if all_array_uses != allowed {
                continue;
            }

            let exempt_length_uses = loops
                .iter()
                .map(|lp| lp.length_use_id)
                .collect::<HashSet<_>>();
            let all_length_uses = collect_local_path_uses(&**body, length_hir, cx);
            let restored_length_use_ids = all_length_uses
                .difference(&exempt_length_uses)
                .copied()
                .collect::<HashSet<_>>();
            let restore_length = !restored_length_use_ids.is_empty();
            let restored_length_name = if restore_length {
                fresh_restored_length_name(func, body, length_index, length_hir, &length_name, cx)
            } else {
                length_name.clone()
            };

            let calls = match analyze_all_calls(
                krate,
                &related_def_ids,
                array_index,
                length_index,
                pointer_mutability,
                &element_ty,
                length_integer_ty,
                global_info,
                arrays,
                cx,
                target,
            ) {
                Some(calls) => calls,
                None => continue,
            };

            pair_plans.push(FunctionPlan {
                item_id: item.id,
                def_id,
                foreign_decl_item_ids: foreign_decl_item_ids.clone(),
                array_index,
                length_index,
                array_name: array_name.clone(),
                pointer_mutability,
                element_ty: element_ty.clone(),
                length_name: length_name.clone(),
                restored_length_name,
                restored_length_use_ids,
                length_ty: length_param.ty.clone(),
                length_integer_ty,
                restore_length,
                loops,
                calls,
            });
        }
    }

    if pair_plans.len() == 1 {
        pair_plans.pop()
    } else {
        None
    }
}

fn function_is_unsafe_extern_c(func: &rustc_ast::Fn) -> bool {
    if !matches!(func.sig.header.unsafety, Unsafe::Yes(_)) {
        return false;
    }
    match func.sig.header.ext {
        Extern::Explicit(ref lit, _) => lit.symbol.as_str() == "C",
        _ => false,
    }
}

fn item_contains_pub(item: &Item) -> bool {
    !matches!(&item.vis.kind, rustc_ast::VisibilityKind::Inherited)
}

fn collect_public_function_linkage_group(
    item: &Item,
    func: &rustc_ast::Fn,
    krate: &Crate,
    cx: &RefactorCtxt,
) -> Option<(HashSet<DefId>, HashSet<rustc_ast::NodeId>)> {
    let mut related_def_ids = HashSet::new();
    related_def_ids.insert(cx.node_def_id(item.id));
    let mut foreign_item_ids = HashSet::new();

    // C2Rust definitions normally use #[no_mangle].  export_name/link_name are
    // handled as well.  If the definition has no source-level fixed external
    // symbol name, no foreign declaration can be related to it by this proof.
    let definition_symbol = fixed_definition_external_symbol(item);
    let mut valid = true;

    if let Some(definition_symbol) = definition_symbol {
        visit_nodes(krate, |candidate: &Item| {
            if !valid {
                return;
            }
            let foreign_mod = match &candidate.kind {
                ItemKind::ForeignMod(foreign_mod) if foreign_mod_is_c(foreign_mod) => foreign_mod,
                _ => return,
            };
            for foreign_item in &foreign_mod.items {
                let foreign_fn = match &foreign_item.kind {
                    ForeignItemKind::Fn(foreign_fn) => foreign_fn,
                    _ => continue,
                };
                if foreign_external_symbol(foreign_item) != definition_symbol {
                    continue;
                }
                if !function_signatures_equivalent(func, foreign_fn, cx) {
                    valid = false;
                    return;
                }
                related_def_ids.insert(cx.node_def_id(foreign_item.id));
                foreign_item_ids.insert(foreign_item.id);
            }
        });
    }

    valid.then_some((related_def_ids, foreign_item_ids))
}

fn foreign_mod_is_c(foreign_mod: &rustc_ast::ForeignMod) -> bool {
    match &foreign_mod.abi {
        None => true,
        Some(abi) => abi.symbol.as_str() == "C",
    }
}

fn fixed_definition_external_symbol(item: &Item) -> Option<Symbol> {
    let export_name = Symbol::intern("export_name");
    if let Some(symbol) = item
        .attrs
        .iter()
        .find(|attr| attr.has_name(export_name))
        .and_then(|attr| attr.value_str())
    {
        return Some(symbol);
    }

    let no_mangle = Symbol::intern("no_mangle");
    item.attrs
        .iter()
        .any(|attr| attr.has_name(no_mangle))
        .then_some(item.ident.name)
}

fn foreign_external_symbol(item: &rustc_ast::ForeignItem) -> Symbol {
    let link_name = Symbol::intern("link_name");
    item.attrs
        .iter()
        .find(|attr| attr.has_name(link_name))
        .and_then(|attr| attr.value_str())
        .unwrap_or(item.ident.name)
}

fn function_signatures_equivalent(
    definition: &rustc_ast::Fn,
    declaration: &rustc_ast::Fn,
    cx: &RefactorCtxt,
) -> bool {
    if definition.sig.decl.inputs.len() != declaration.sig.decl.inputs.len() {
        return false;
    }
    if !definition
        .sig
        .decl
        .inputs
        .iter()
        .zip(&declaration.sig.decl.inputs)
        .all(|(left, right)| same_type(&left.ty, &right.ty, cx))
    {
        return false;
    }
    return_types_equivalent(
        &definition.sig.decl.output,
        &declaration.sig.decl.output,
        cx,
    )
}

fn return_types_equivalent(left: &FnRetTy, right: &FnRetTy, cx: &RefactorCtxt) -> bool {
    match (left, right) {
        (FnRetTy::Default(_), FnRetTy::Default(_)) => true,
        (FnRetTy::Ty(left), FnRetTy::Ty(right)) => same_type(left, right, cx),
        (FnRetTy::Default(_), FnRetTy::Ty(ty)) | (FnRetTy::Ty(ty), FnRetTy::Default(_)) => {
            matches!(&ty.kind, TyKind::Tup(types) if types.is_empty())
        }
    }
}

fn fresh_restored_length_name(
    func: &rustc_ast::Fn,
    body: &rustc_ast::Block,
    removed_param_index: usize,
    removed_param_hir: HirId,
    requested: &str,
    cx: &RefactorCtxt,
) -> String {
    let mut names = HashSet::<String>::new();

    for (index, param) in func.sig.decl.inputs.iter().enumerate() {
        if index != removed_param_index {
            collect_pattern_binding_names(&param.pat, &mut names);
        }
    }

    struct BindingCollector<'names, 'cx, 'a, 'tcx: 'a> {
        names: &'names mut HashSet<String>,
        removed_param_hir: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
    }
    impl<'ast, 'names, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for BindingCollector<'names, 'cx, 'a, 'tcx> {
        fn visit_pat(&mut self, pat: &'ast Pat) {
            record_pattern_binding_name(pat, self.names);
            visit::walk_pat(self, pat);
        }

        fn visit_expr(&mut self, expr: &'ast Expr) {
            if !expr_resolves_to_local(expr, self.removed_param_hir, self.cx) {
                if let Some(name) = simple_path_name(expr) {
                    self.names.insert(name.to_string());
                }
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, item: &'ast Item) {
            self.names.insert(item.ident.name.to_string());
            // Nested items are separate scopes; their internal bindings cannot
            // collide with the restored local in the enclosing function.
        }
    }

    let mut collector = BindingCollector {
        names: &mut names,
        removed_param_hir,
        cx,
    };
    collector.visit_block(body);

    fresh_name(requested, &names)
}

fn analyze_loop(
    expr: &Expr,
    array_hir: HirId,
    length_hir: HirId,
    length_ty: IntegerType,
    pointer_mutability: Mutability,
    element_ty: &P<Ty>,
    cx: &RefactorCtxt,
    target: TargetInfo,
    binding_origins: &LoopBindingOrigins,
    aliases: &HashMap<DefId, P<Ty>>,
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
    let length_use_id = match_length_bound(
        end,
        length_hir,
        length_ty,
        induction_ty,
        cx,
        target,
        aliases,
    )?;

    let mut checker = BodyPointerUseChecker {
        array_hir,
        induction_hir,
        induction_name: &induction_name,
        element_ty,
        cx,
        pointer_access_ids: HashSet::new(),
        allowed_array_path_ids: HashSet::new(),
        mutable_cast_access_ids: HashSet::new(),
        invalid: false,
    };
    checker.visit_block(body);
    if checker.invalid || checker.pointer_access_ids.is_empty() {
        return None;
    }

    if pointer_mutability == Mutability::Not && !checker.mutable_cast_access_ids.is_empty() {
        let mut detector = MutableAccessDetector {
            tracked: &checker.mutable_cast_access_ids,
            cx,
            bad: false,
        };
        detector.visit_block(body);
        if detector.bad {
            return None;
        }
    }

    Some(LoopPlan {
        induction_name: induction_name.clone(),
        induction_ty,
        length_use_id,
        pointer_access_ids: checker.pointer_access_ids,
        allowed_array_path_ids: checker.allowed_array_path_ids,
    })
}

fn zero_start_type(
    expr: &Expr,
    cx: &RefactorCtxt,
    aliases: &HashMap<DefId, P<Ty>>,
) -> Option<IntegerType> {
    let expr = strip_parens(expr);
    match &expr.kind {
        ExprKind::Lit(lit) => match lit.kind {
            LitKind::Int(0, LitIntType::Signed(ty)) => Some(match ty {
                IntTy::I8 => IntegerType::Primitive("i8"),
                IntTy::I16 => IntegerType::Primitive("i16"),
                IntTy::I32 => IntegerType::Primitive("i32"),
                IntTy::I64 => IntegerType::Primitive("i64"),
                IntTy::I128 => IntegerType::Primitive("i128"),
                IntTy::Isize => IntegerType::Primitive("isize"),
            }),
            LitKind::Int(0, LitIntType::Unsigned(ty)) => Some(match ty {
                UintTy::U8 => IntegerType::Primitive("u8"),
                UintTy::U16 => IntegerType::Primitive("u16"),
                UintTy::U32 => IntegerType::Primitive("u32"),
                UintTy::U64 => IntegerType::Primitive("u64"),
                UintTy::U128 => IntegerType::Primitive("u128"),
                UintTy::Usize => IntegerType::Primitive("usize"),
            }),
            _ => None,
        },
        ExprKind::Cast(inner, ty)
            if unsuffixed_integer_literal_value(strip_parens(inner))
                == Some(IntValue::NonNegative(0)) =>
        {
            underlying_integer_type_from_ty(ty, cx, aliases)
        }
        _ => None,
    }
}

fn match_length_bound(
    expr: &Expr,
    length_hir: HirId,
    length_ty: IntegerType,
    induction_ty: IntegerType,
    cx: &RefactorCtxt,
    target: TargetInfo,
    aliases: &HashMap<DefId, P<Ty>>,
) -> Option<rustc_ast::NodeId> {
    let expr = strip_parens(expr);
    if expr_resolves_to_local(expr, length_hir, cx) {
        return (length_ty == induction_ty).then_some(expr.id);
    }

    let (inner, cast_ty) = match &expr.kind {
        ExprKind::Cast(inner, cast_ty) => (strip_parens(inner), cast_ty),
        _ => return None,
    };
    if !expr_resolves_to_local(inner, length_hir, cx) {
        return None;
    }
    let cast_ty = underlying_integer_type_from_ty(cast_ty, cx, aliases)?;
    if cast_ty != induction_ty || length_ty == induction_ty {
        return None;
    }
    if !integer_type_range_is_subset(length_ty, induction_ty, target) {
        return None;
    }
    Some(inner.id)
}

struct BodyPointerUseChecker<'cx, 'a, 'tcx: 'a> {
    array_hir: HirId,
    induction_hir: Option<HirId>,
    induction_name: &'cx str,
    element_ty: &'cx P<Ty>,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    pointer_access_ids: HashSet<rustc_ast::NodeId>,
    allowed_array_path_ids: HashSet<rustc_ast::NodeId>,
    mutable_cast_access_ids: HashSet<rustc_ast::NodeId>,
    invalid: bool,
}

#[derive(Clone, Copy)]
struct PointerAccessMatch {
    array_path_id: rustc_ast::NodeId,
    through_mut_cast: bool,
}

impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for BodyPointerUseChecker<'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &'ast Expr) {
        if self.invalid {
            return;
        }
        if let Some(matched) = match_pointer_access(
            expr,
            self.array_hir,
            self.induction_hir,
            self.induction_name,
            self.element_ty,
            self.cx,
        ) {
            self.pointer_access_ids.insert(expr.id);
            self.allowed_array_path_ids.insert(matched.array_path_id);
            if matched.through_mut_cast {
                self.mutable_cast_access_ids.insert(expr.id);
            }
            return;
        }

        if expr_resolves_to_local(expr, self.array_hir, self.cx) {
            self.invalid = true;
            return;
        }
        visit::walk_expr(self, expr);
    }
}

fn match_pointer_access(
    expr: &Expr,
    array_hir: HirId,
    induction_hir: Option<HirId>,
    induction_name: &str,
    element_ty: &P<Ty>,
    cx: &RefactorCtxt,
) -> Option<PointerAccessMatch> {
    let inner = match &strip_parens(expr).kind {
        ExprKind::Unary(rustc_ast::UnOp::Deref, inner) => strip_parens(inner),
        _ => return None,
    };
    let (segment, receiver, args) = match &inner.kind {
        ExprKind::MethodCall(segment, receiver, args, _)
            if segment.ident.name.as_str() == "offset" =>
        {
            (segment, strip_parens(receiver), args)
        }
        _ => return None,
    };
    let _ = segment;
    if args.len() != 1 || !matches_induction_as_isize(&args[0], induction_hir, induction_name, cx) {
        return None;
    }

    match &receiver.kind {
        ExprKind::Path(..) if expr_resolves_to_local(receiver, array_hir, cx) => {
            Some(PointerAccessMatch {
                array_path_id: receiver.id,
                through_mut_cast: false,
            })
        }
        ExprKind::Cast(base, cast_ty) => {
            let base = strip_parens(base);
            if !expr_resolves_to_local(base, array_hir, cx) {
                return None;
            }
            let mut_ty = match &cast_ty.kind {
                TyKind::Ptr(mut_ty) => mut_ty,
                _ => return None,
            };
            if !same_type(&mut_ty.ty, element_ty, cx) {
                return None;
            }
            Some(PointerAccessMatch {
                array_path_id: base.id,
                through_mut_cast: mut_ty.mutbl == Mutability::Mut,
            })
        }
        _ => None,
    }
}

fn matches_induction_as_isize(
    expr: &Expr,
    induction_hir: Option<HirId>,
    induction_name: &str,
    cx: &RefactorCtxt,
) -> bool {
    let expr = strip_parens(expr);
    let (inner, ty) = match &expr.kind {
        ExprKind::Cast(inner, ty) => (strip_parens(inner), ty),
        _ => return false,
    };
    if integer_type_from_ty(ty) != Some(IntegerType::Primitive("isize")) {
        return false;
    }
    if let Some(hir) = induction_hir {
        if expr_resolves_to_local(inner, hir, cx) {
            return true;
        }
    }
    simple_path_name(inner)
        .map(|name| name == induction_name)
        .unwrap_or(false)
}

struct MutableAccessDetector<'cx, 'a, 'tcx: 'a> {
    tracked: &'cx HashSet<rustc_ast::NodeId>,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    bad: bool,
}

impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for MutableAccessDetector<'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &'ast Expr) {
        if self.bad {
            return;
        }
        match &expr.kind {
            ExprKind::Assign(lhs, _, _) | ExprKind::AssignOp(_, lhs, _) => {
                if subtree_contains_any_expr(lhs, self.tracked) {
                    self.bad = true;
                    return;
                }
            }
            ExprKind::AddrOf(_, Mutability::Mut, operand) => {
                if subtree_contains_any_expr(operand, self.tracked) {
                    self.bad = true;
                    return;
                }
            }
            _ => {}
        }
        if self.tracked.contains(&expr.id)
            && adjusted_reference_mutability(expr, self.cx) == Some(Mutability::Mut)
        {
            self.bad = true;
            return;
        }
        visit::walk_expr(self, expr);
    }
}

fn subtree_contains_any_expr(expr: &Expr, ids: &HashSet<rustc_ast::NodeId>) -> bool {
    struct Finder<'a> {
        ids: &'a HashSet<rustc_ast::NodeId>,
        found: bool,
    }
    impl<'ast, 'a> Visitor<'ast> for Finder<'a> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found || self.ids.contains(&expr.id) {
                self.found = true;
                return;
            }
            visit::walk_expr(self, expr);
        }
    }
    let mut finder = Finder { ids, found: false };
    finder.visit_expr(expr);
    finder.found
}

fn collect_local_path_uses(
    body: &rustc_ast::Block,
    tracked: HirId,
    cx: &RefactorCtxt,
) -> HashSet<rustc_ast::NodeId> {
    let mut result = HashSet::new();
    visit_nodes(body, |expr: &Expr| {
        if expr_resolves_to_local(expr, tracked, cx) {
            result.insert(expr.id);
        }
    });
    result
}

fn local_has_forbidden_reference_syntax(
    body: &rustc_ast::Block,
    tracked: HirId,
    cx: &RefactorCtxt,
) -> bool {
    struct Checker<'cx, 'a, 'tcx: 'a> {
        tracked: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        bad: bool,
    }

    impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for Checker<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.bad {
                return;
            }
            if let ExprKind::AddrOf(_, _, operand) = &expr.kind {
                if expr_resolves_to_local(strip_parens(operand), self.tracked, self.cx) {
                    self.bad = true;
                    return;
                }
            }
            if expr_resolves_to_local(expr, self.tracked, self.cx)
                && adjusted_reference_mutability(expr, self.cx).is_some()
            {
                self.bad = true;
                return;
            }
            visit::walk_expr(self, expr);
        }

        fn visit_local(&mut self, local: &'ast Local) {
            if self.bad {
                return;
            }
            if let LocalKind::Init(init) = &local.kind {
                if expr_resolves_to_local(strip_parens(init), self.tracked, self.cx)
                    && pattern_reference_mutability(&local.pat).is_some()
                {
                    self.bad = true;
                    return;
                }
            }
            visit::walk_local(self, local);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items are distinct scopes and cannot capture this local.
        }
    }

    let mut checker = Checker {
        tracked,
        cx,
        bad: false,
    };
    checker.visit_block(body);
    checker.bad
}

fn local_is_read_only_and_unborrowed(
    body: &rustc_ast::Block,
    tracked: HirId,
    cx: &RefactorCtxt,
) -> bool {
    // Constraint 3 is about writes to the parameter after it has been bound.
    // ExprUseVisitor reports the initial binding of a `mut` parameter as a
    // mutation as well, so using it function-wide produces a false positive
    // for ordinary C2Rust parameters such as `mut length: i32`. Detect actual
    // source-level writes instead. Because U1 is a primitive integer, every
    // indirect mutation first requires a reference/raw pointer, which is
    // rejected separately by local_has_forbidden_reference_syntax().
    struct WriteChecker<'cx, 'a, 'tcx: 'a> {
        tracked: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        bad: bool,
    }

    impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for WriteChecker<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.bad {
                return;
            }

            match &expr.kind {
                ExprKind::Assign(lhs, _, _) | ExprKind::AssignOp(_, lhs, _) => {
                    let mut finder = LocalUseFinder {
                        tracked: self.tracked,
                        cx: self.cx,
                        found: false,
                    };
                    finder.visit_expr(lhs);
                    if finder.found {
                        self.bad = true;
                        return;
                    }
                }
                _ => {}
            }

            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items are separate scopes and cannot capture this local.
        }
    }

    struct LocalUseFinder<'cx, 'a, 'tcx: 'a> {
        tracked: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        found: bool,
    }

    impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for LocalUseFinder<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }
            if expr_resolves_to_local(expr, self.tracked, self.cx) {
                self.found = true;
                return;
            }
            visit::walk_expr(self, expr);
        }
    }

    let mut write_checker = WriteChecker {
        tracked,
        cx,
        bad: false,
    };
    write_checker.visit_block(body);
    if write_checker.bad {
        return false;
    }

    // A closure/async capture by reference creates a reference even without an
    // explicit `&length` expression in the source, so retain rustc's capture
    // information for that part of constraint 3.
    let tcx = cx.ty_ctxt();
    let hir_map = cx.hir_map();
    let parent = hir_map.get_parent_item(tracked);
    let body_id = match hir_map.maybe_body_owned_by(parent) {
        Some(id) => id,
        None => return false,
    };
    let tables = tcx.typeck_body(body_id);

    !tables
        .closure_min_captures
        .values()
        .filter_map(|captures| captures.get(&tracked))
        .flatten()
        .any(|capture| matches!(capture.info.capture_kind, ty::UpvarCapture::ByRef(_)))
}

fn functions_are_used_only_as_direct_callees(
    krate: &Crate,
    target_fns: &HashSet<DefId>,
    cx: &RefactorCtxt,
) -> bool {
    let mut direct_callee_ids = HashSet::new();
    visit_nodes(krate, |expr: &Expr| {
        if let ExprKind::Call(callee, _) = &expr.kind {
            let callee = strip_parens(callee);
            if matches!(cx.try_resolve_expr_hir(callee), Some(Res::Def(_, id)) if target_fns.contains(&id))
            {
                direct_callee_ids.insert(callee.id);
            }
        }
    });

    let mut valid = true;
    visit_nodes(krate, |expr: &Expr| {
        if !valid || !matches!(expr.kind, ExprKind::Path(..)) {
            return;
        }
        if matches!(cx.try_resolve_expr_hir(expr), Some(Res::Def(_, id)) if target_fns.contains(&id))
            && !direct_callee_ids.contains(&expr.id)
        {
            valid = false;
        }
    });
    valid
}

fn analyze_all_calls(
    krate: &Crate,
    target_fns: &HashSet<DefId>,
    array_index: usize,
    length_index: usize,
    pointer_mutability: Mutability,
    element_ty: &P<Ty>,
    length_ty: IntegerType,
    global_info: &GlobalInfo,
    arrays: &HashMap<ArrayKey, ArrayMeta>,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> Option<Vec<CallPlan>> {
    let mut all_target_call_ids = HashSet::new();
    visit_nodes(krate, |expr: &Expr| {
        if let ExprKind::Call(callee, _) = &expr.kind {
            if matches!(cx.try_resolve_expr_hir(strip_parens(callee)), Some(Res::Def(_, id)) if target_fns.contains(&id))
            {
                all_target_call_ids.insert(expr.id);
            }
        }
    });

    let mut calls = Vec::new();
    let mut seen_call_ids = HashSet::new();
    let mut valid = true;

    visit_nodes(krate, |item: &Item| {
        if !valid {
            return;
        }
        let func = match &item.kind {
            ItemKind::Fn(func) => func,
            _ => return,
        };
        let body = match &func.body {
            Some(body) => body,
            None => return,
        };

        visit_nodes(&**body, |expr: &Expr| {
            if !valid {
                return;
            }
            let (callee, args) = match &expr.kind {
                ExprKind::Call(callee, args) => (callee, args),
                _ => return,
            };
            if !matches!(cx.try_resolve_expr_hir(strip_parens(callee)), Some(Res::Def(_, id)) if target_fns.contains(&id))
            {
                return;
            }
            seen_call_ids.insert(expr.id);
            if array_index >= args.len() || length_index >= args.len() {
                valid = false;
                return;
            }

            let array_arg = match match_array_argument(
                &args[array_index],
                pointer_mutability,
                element_ty,
                arrays,
                cx,
            ) {
                Some(value) => value,
                None => {
                    valid = false;
                    return;
                }
            };
            let array_meta = match arrays.get(&array_arg.key) {
                Some(value) => value,
                None => {
                    valid = false;
                    return;
                }
            };
            if !valid_length_argument(
                &args[length_index],
                array_meta,
                element_ty,
                length_ty,
                global_info,
                cx,
                target,
            ) {
                valid = false;
                return;
            }
            // Constraint 7.d is checked after the structural call-site
            // conditions.  Its earlier-call exception is derived independently
            // from callee non-escape analysis.
            calls.push(CallPlan {
                call_id: expr.id,
                caller_body_id: body.id,
                call_span: expr.span,
                array_index,
                length_index,
                array_key: array_arg.key,
                exception_addr_id: array_arg.exception_addr_id,
                array_expr: array_arg.operand.clone(),
            });
        });
    });

    if seen_call_ids != all_target_call_ids {
        return None;
    }
    valid.then_some(calls)
}

struct ArrayArgumentMatch {
    key: ArrayKey,
    exception_addr_id: rustc_ast::NodeId,
    operand: P<Expr>,
}

fn match_array_argument(
    expr: &Expr,
    pointer_mutability: Mutability,
    element_ty: &P<Ty>,
    arrays: &HashMap<ArrayKey, ArrayMeta>,
    cx: &RefactorCtxt,
) -> Option<ArrayArgumentMatch> {
    let mut expr = strip_parens(expr);
    let mut casts = Vec::<(Mutability, P<Ty>)>::new();
    while let ExprKind::Cast(inner, ty) = &expr.kind {
        let ptr = match &ty.kind {
            TyKind::Ptr(ptr) => ptr,
            _ => return None,
        };
        casts.push((ptr.mutbl, ptr.ty.clone()));
        expr = strip_parens(inner);
    }
    if casts.is_empty() || casts.len() > 2 {
        return None;
    }
    // Casts are collected outside-in, so casts[0] is the explicit final
    // pointer type. A *mut U0 expression may be passed to a *const U0 parameter
    // by Rust's built-in raw-pointer coercion (constraint 7.b.iii), while the
    // reverse direction is not valid.
    if pointer_mutability == Mutability::Mut && casts[0].0 != Mutability::Mut {
        return None;
    }
    if casts.iter().any(|(_, ty)| !same_type(ty, element_ty, cx)) {
        return None;
    }

    let (borrow_kind, raw_mutability, operand_ast) = match &expr.kind {
        ExprKind::AddrOf(kind, mutability, operand) => (*kind, *mutability, operand.clone()),
        _ => return None,
    };
    if borrow_kind != BorrowKind::Raw {
        return None;
    }
    let operand = strip_parens(&operand_ast);
    let key = array_key_for_expr(operand, cx)?;
    let meta = arrays.get(&key)?;
    if !same_type(&meta.element_ty, element_ty, cx) {
        return None;
    }

    let cast_shape_ok = match (raw_mutability, casts.as_slice()) {
        (Mutability::Not, [(Mutability::Not, _)]) => true,
        (Mutability::Mut, [(Mutability::Mut, _)]) => true,
        (Mutability::Mut, [(Mutability::Mut, _), (Mutability::Not, _)]) => true,
        (Mutability::Mut, [(Mutability::Not, _), (Mutability::Mut, _)]) => true,
        _ => false,
    };
    if !cast_shape_ok {
        return None;
    }

    Some(ArrayArgumentMatch {
        key,
        exception_addr_id: expr.id,
        operand: operand_ast,
    })
}

fn valid_length_argument(
    expr: &Expr,
    array: &ArrayMeta,
    element_ty: &P<Ty>,
    length_ty: IntegerType,
    global_info: &GlobalInfo,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> bool {
    if unsuffixed_integer_literal_value(strip_parens(expr))
        == Some(IntValue::NonNegative(array.length))
    {
        return integer_value_fits_type(IntValue::NonNegative(array.length), length_ty, target);
    }

    if valid_length_const_argument(expr, array.length, length_ty, global_info, cx, target) {
        return true;
    }

    valid_sizeof_length_argument(expr, array, element_ty, length_ty, global_info, cx, target)
}

fn valid_length_const_argument(
    expr: &Expr,
    array_len: u128,
    length_ty: IntegerType,
    global_info: &GlobalInfo,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> bool {
    let mut expr = strip_parens(expr);
    let mut casts = Vec::new();
    while let ExprKind::Cast(inner, ty) = &expr.kind {
        let integer = match underlying_integer_type_from_ty(ty, cx, &global_info.all_aliases) {
            Some(value) => value,
            None => return false,
        };
        casts.push(integer);
        expr = strip_parens(inner);
        if casts.len() > 2 {
            return false;
        }
    }
    let const_id = match cx.try_resolve_expr_hir(expr) {
        Some(Res::Def(_, id)) => id,
        _ => return false,
    };
    let meta = match global_info.consts.get(&const_id) {
        Some(value) => value,
        None => return false,
    };
    if meta.value != IntValue::NonNegative(array_len) {
        return false;
    }

    match casts.as_slice() {
        [] => {
            if meta.integer_ty != length_ty {
                return false;
            }
        }
        [outer] => {
            if *outer != length_ty {
                return false;
            }
        }
        [outer, _inner] => {
            if *outer != length_ty {
                return false;
            }
        }
        _ => return false,
    }

    integer_value_fits_type(meta.value, meta.integer_ty, target)
        && integer_value_fits_type(meta.value, length_ty, target)
        && casts
            .iter()
            .all(|ty| integer_value_fits_type(meta.value, *ty, target))
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum SizeCastKind {
    Usize,
    SizeT,
}

fn valid_sizeof_length_argument(
    expr: &Expr,
    array: &ArrayMeta,
    element_ty: &P<Ty>,
    length_ty: IntegerType,
    global_info: &GlobalInfo,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> bool {
    let mut expr = strip_parens(expr);
    let mut outer_cast = None;
    if let ExprKind::Cast(inner, ty) = &expr.kind {
        outer_cast = underlying_integer_type_from_ty(ty, cx, &global_info.all_aliases);
        if outer_cast.is_none() {
            return false;
        }
        expr = strip_parens(inner);
    }

    let (receiver, args) = match &expr.kind {
        ExprKind::MethodCall(segment, receiver, args, _)
            if segment.ident.name.as_str() == "wrapping_div" && args.len() == 1 =>
        {
            (strip_parens(receiver), args)
        }
        _ => return false,
    };

    let (numerator, numerator_cast) = match match_casted_sizeof(receiver, cx, global_info) {
        Some(value) => value,
        None => return false,
    };
    let (denominator, denominator_cast) =
        match match_casted_sizeof(strip_parens(&args[0]), cx, global_info) {
            Some(value) => value,
            None => return false,
        };
    if numerator_cast != denominator_cast {
        return false;
    }

    let (array_element, array_len) = match &numerator.kind {
        TyKind::Array(element, len) => {
            let len = match unsuffixed_integer_literal_value(&len.value) {
                Some(IntValue::NonNegative(value)) => value,
                _ => return false,
            };
            (element, len)
        }
        _ => return false,
    };
    if array_len != array.length
        || !same_type(array_element, element_ty, cx)
        || !same_type(&denominator, element_ty, cx)
    {
        return false;
    }

    // The specification requires size_of::<U0>() != 0.  We prove that only
    // for source types for which non-zero size follows without layout queries;
    // unknown/ADT layouts therefore fail closed.
    if !type_is_known_non_zero_sized(element_ty, cx, &global_info.all_aliases) {
        return false;
    }
    let len_value = IntValue::NonNegative(array.length);
    if !integer_value_fits_type(len_value, length_ty, target) {
        return false;
    }

    match outer_cast {
        // Constraint 7.c.iii: both size_of results are cast specifically to
        // usize, then the quotient is cast to U1.
        Some(ty) => {
            ty == length_ty
                && numerator_cast == SizeCastKind::Usize
                && denominator_cast == SizeCastKind::Usize
        }
        // Constraint 7.c.iv: no outer cast and U5 is exactly usize or size_t.
        // Both spellings have underlying type usize, so U1 must be usize.
        None => length_ty == IntegerType::Primitive("usize"),
    }
}

fn match_casted_sizeof(
    expr: &Expr,
    cx: &RefactorCtxt,
    global_info: &GlobalInfo,
) -> Option<(P<Ty>, SizeCastKind)> {
    let (call, cast_ty) = match &expr.kind {
        ExprKind::Cast(call, cast_ty) => (strip_parens(call), cast_ty),
        _ => return None,
    };

    let spelling = simple_ty_path_name(cast_ty)?;
    let cast_kind = match spelling {
        "usize" => SizeCastKind::Usize,
        "size_t" => SizeCastKind::SizeT,
        _ => return None,
    };
    if underlying_integer_type_from_ty(cast_ty, cx, &global_info.all_aliases)?
        != IntegerType::Primitive("usize")
    {
        return None;
    }

    let ty = size_of_generic_type(call)?;
    Some((ty, cast_kind))
}

fn size_of_generic_type(expr: &Expr) -> Option<P<Ty>> {
    let (callee, args) = match &expr.kind {
        ExprKind::Call(callee, args) if args.is_empty() => (strip_parens(callee), args),
        _ => return None,
    };
    let _ = args;
    let path = match &callee.kind {
        ExprKind::Path(None, path) => path,
        _ => return None,
    };
    if !path.is_global()
        || path.segments.len() != 4
        || path.segments[1].ident.name.as_str() != "core"
        || path.segments[2].ident.name.as_str() != "mem"
        || path.segments[3].ident.name.as_str() != "size_of"
    {
        return None;
    }
    let generic_args = path.segments[3].args.as_ref()?;
    let angle = match &**generic_args {
        GenericArgs::AngleBracketed(angle) => angle,
        _ => return None,
    };
    if angle.args.len() != 1 {
        return None;
    }
    match &angle.args[0] {
        AngleBracketedArg::Arg(GenericArg::Type(ty)) => Some(ty.clone()),
        _ => None,
    }
}

fn type_is_known_non_zero_sized(
    ty: &Ty,
    cx: &RefactorCtxt,
    aliases: &HashMap<DefId, P<Ty>>,
) -> bool {
    // Every primitive integer type accepted by the loop passes has a strictly
    // positive bit width, so size_of::<T>() is known to be non-zero without
    // consulting HIR/typeck for the AST type node.  In particular this avoids
    // rejecting C2Rust-generated size_of::<i32>() expressions on toolchains
    // where RefactorCtxt::opt_node_type does not expose a type for Ty NodeIds.
    if underlying_integer_type_from_ty(ty, cx, aliases).is_some() {
        return true;
    }

    // Raw/reference pointers also have a non-zero target pointer width.
    if matches!(&ty.kind, TyKind::Ptr(_) | TyKind::Rptr(..)) {
        return true;
    }

    // For all remaining source types, keep the conservative rustc-layout
    // proof.  Unknown layouts fail closed.
    let rust_ty = match cx.opt_node_type(ty.id) {
        Some(ty) => ty,
        None => return false,
    };
    cx.ty_ctxt()
        .layout_of(ParamEnv::empty().and(rust_ty))
        .map(|layout| layout.size.bytes() != 0)
        .unwrap_or(false)
}

fn filter_plans_by_aliasing(krate: &Crate, plans: &mut Vec<FunctionPlan>, cx: &RefactorCtxt) {
    let non_escaping_call_addr_ids = collect_non_escaping_call_addr_ids(krate, cx);
    plans.retain(|plan| {
        function_plan_aliasing_is_safe(plan, krate, &non_escaping_call_addr_ids, cx)
    });
}

fn collect_non_escaping_call_addr_ids(
    krate: &Crate,
    cx: &RefactorCtxt,
) -> HashMap<rustc_ast::NodeId, Span> {
    let mut non_escaping_params = HashSet::<(DefId, usize)>::new();

    visit_nodes(krate, |item: &Item| {
        let func = match &item.kind {
            ItemKind::Fn(func) => func,
            _ => return,
        };
        let body = match &func.body {
            Some(body) => body,
            None => return,
        };
        let def_id = cx.node_def_id(item.id);
        for (index, param) in func.sig.decl.inputs.iter().enumerate() {
            if !matches!(&param.ty.kind, TyKind::Ptr(_)) {
                continue;
            }
            let (tracked, _, _) = match simple_binding(&param.pat, cx) {
                Some(binding) => binding,
                None => continue,
            };
            if raw_pointer_parameter_does_not_escape(body, tracked, cx) {
                non_escaping_params.insert((def_id, index));
            }
        }
    });

    let foreign_to_definition = collect_foreign_definition_map(krate, cx);
    let mut result = HashMap::new();
    visit_nodes(krate, |expr: &Expr| {
        let (callee, args) = match &expr.kind {
            ExprKind::Call(callee, args) => (strip_parens(callee), args),
            _ => return,
        };
        let resolved = match cx.try_resolve_expr_hir(callee) {
            Some(Res::Def(_, id)) => id,
            _ => return,
        };
        let definition = foreign_to_definition
            .get(&resolved)
            .copied()
            .unwrap_or(resolved);

        for (index, arg) in args.iter().enumerate() {
            if !non_escaping_params.contains(&(definition, index)) {
                continue;
            }
            if let Some(addr_id) = direct_raw_address_argument_id(arg) {
                result.insert(addr_id, expr.span);
            }
        }
    });
    result
}

fn collect_foreign_definition_map(krate: &Crate, cx: &RefactorCtxt) -> HashMap<DefId, DefId> {
    let mut result = HashMap::new();

    visit_nodes(krate, |item: &Item| {
        let foreign_mod = match &item.kind {
            ItemKind::ForeignMod(foreign_mod) if foreign_mod_is_c(foreign_mod) => foreign_mod,
            _ => return,
        };
        for foreign_item in &foreign_mod.items {
            let foreign_fn = match &foreign_item.kind {
                ForeignItemKind::Fn(func) => func,
                _ => continue,
            };
            let foreign_symbol = foreign_external_symbol(foreign_item);
            let mut matching_definition = None;
            let mut ambiguous = false;
            visit_nodes(krate, |candidate: &Item| {
                if ambiguous {
                    return;
                }
                let candidate_fn = match &candidate.kind {
                    ItemKind::Fn(func) => func,
                    _ => return,
                };
                if !item_contains_pub(candidate)
                    || !function_is_unsafe_extern_c(candidate_fn)
                    || fixed_definition_external_symbol(candidate) != Some(foreign_symbol)
                    || !function_signatures_equivalent(candidate_fn, foreign_fn, cx)
                {
                    return;
                }
                let id = cx.node_def_id(candidate.id);
                if matching_definition.replace(id).is_some() {
                    ambiguous = true;
                }
            });
            if !ambiguous {
                if let Some(definition) = matching_definition {
                    result.insert(cx.node_def_id(foreign_item.id), definition);
                }
            }
        }
    });

    result
}

fn direct_raw_address_argument_id(expr: &Expr) -> Option<rustc_ast::NodeId> {
    let mut expr = strip_parens(expr);
    while let ExprKind::Cast(inner, ty) = &expr.kind {
        if !matches!(&ty.kind, TyKind::Ptr(_)) {
            return None;
        }
        expr = strip_parens(inner);
    }
    match &expr.kind {
        ExprKind::AddrOf(BorrowKind::Raw, _, _) => Some(expr.id),
        _ => None,
    }
}

fn raw_pointer_parameter_does_not_escape(
    body: &rustc_ast::Block,
    tracked: HirId,
    cx: &RefactorCtxt,
) -> bool {
    struct Checker<'cx, 'a, 'tcx: 'a> {
        tracked: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        bad: bool,
    }

    impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for Checker<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.bad {
                return;
            }

            // Creating any reference/raw pointer derived from the parameter is
            // conservatively rejected: such a derived value could outlive the
            // immediate pointed-to-value access.
            if let ExprKind::AddrOf(_, _, operand) = &expr.kind {
                if subtree_contains_local(operand, self.tracked, self.cx) {
                    self.bad = true;
                    return;
                }
            }

            // Capturing the pointer in a closure is explicitly forbidden by
            // constraint 7.d.iii, even if the closure body only dereferences it.
            if matches!(expr.kind, ExprKind::Closure(..) | ExprKind::Async(..))
                && subtree_contains_local(expr, self.tracked, self.cx)
            {
                self.bad = true;
                return;
            }

            // A dereference consumes a pointer expression solely to access the
            // pointed-to value.  Permit casts and raw-pointer derivation methods
            // on the way to that dereference, provided their arguments do not
            // themselves use/copy the tracked parameter.
            if let ExprKind::Unary(rustc_ast::UnOp::Deref, pointer) = &strip_parens(expr).kind {
                if pointer_expression_is_derived_from_local(pointer, self.tracked, self.cx) {
                    return;
                }
            }

            if expr_resolves_to_local(expr, self.tracked, self.cx) {
                self.bad = true;
                return;
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items cannot capture a function parameter.
        }
    }

    let mut checker = Checker {
        tracked,
        cx,
        bad: false,
    };
    checker.visit_block(body);
    !checker.bad
}

fn pointer_expression_is_derived_from_local(
    expr: &Expr,
    tracked: HirId,
    cx: &RefactorCtxt,
) -> bool {
    let expr = strip_parens(expr);
    if expr_resolves_to_local(expr, tracked, cx) {
        return true;
    }
    match &expr.kind {
        ExprKind::Cast(inner, ty) if matches!(&ty.kind, TyKind::Ptr(_)) => {
            pointer_expression_is_derived_from_local(inner, tracked, cx)
        }
        ExprKind::MethodCall(segment, receiver, args, _) => {
            let method = segment.ident.name.as_str();
            let derivation_method = matches!(
                method,
                "offset"
                    | "wrapping_offset"
                    | "add"
                    | "wrapping_add"
                    | "sub"
                    | "wrapping_sub"
                    | "byte_offset"
                    | "wrapping_byte_offset"
                    | "byte_add"
                    | "wrapping_byte_add"
                    | "byte_sub"
                    | "wrapping_byte_sub"
                    | "cast"
                    | "with_addr"
                    | "map_addr"
            );
            derivation_method
                && pointer_expression_is_derived_from_local(receiver, tracked, cx)
                && args
                    .iter()
                    .all(|arg| !subtree_contains_local(arg, tracked, cx))
        }
        _ => false,
    }
}

fn subtree_contains_local(expr: &Expr, tracked: HirId, cx: &RefactorCtxt) -> bool {
    struct Finder<'cx, 'a, 'tcx: 'a> {
        tracked: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        found: bool,
    }
    impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for Finder<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }
            if expr_resolves_to_local(expr, self.tracked, self.cx) {
                self.found = true;
                return;
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {}
    }
    let mut finder = Finder {
        tracked,
        cx,
        found: false,
    };
    finder.visit_expr(expr);
    finder.found
}

fn function_plan_aliasing_is_safe(
    plan: &FunctionPlan,
    krate: &Crate,
    non_escaping_call_addr_ids: &HashMap<rustc_ast::NodeId, Span>,
    cx: &RefactorCtxt,
) -> bool {
    plan.calls.iter().all(|call| {
        let mut result = None;
        visit_nodes(krate, |block: &rustc_ast::Block| {
            if result.is_some() || block.id != call.caller_body_id {
                return;
            }
            result = Some(call_array_borrow_is_safe(
                block,
                call.array_key,
                call.call_span,
                call.exception_addr_id,
                plan.pointer_mutability,
                non_escaping_call_addr_ids,
                cx,
            ));
        });
        result.unwrap_or(false)
    })
}

fn call_array_borrow_is_safe(
    body: &rustc_ast::Block,
    key: ArrayKey,
    call_span: Span,
    exception_addr_id: rustc_ast::NodeId,
    pointer_mutability: Mutability,
    non_escaping_call_addr_ids: &HashMap<rustc_ast::NodeId, Span>,
    cx: &RefactorCtxt,
) -> bool {
    let mut checker = ReferenceBeforeCallChecker {
        key,
        call_span,
        exception_addr_id,
        pointer_mutability,
        non_escaping_call_addr_ids,
        cx,
        bad: false,
    };
    checker.visit_block(body);
    if checker.bad {
        return false;
    }

    if let ArrayKey::Local(local) = key {
        if closure_capture_before_call_is_forbidden(local, call_span, pointer_mutability, cx) {
            return false;
        }
    }
    true
}

struct ReferenceBeforeCallChecker<'cx, 'a, 'tcx: 'a> {
    key: ArrayKey,
    call_span: Span,
    exception_addr_id: rustc_ast::NodeId,
    pointer_mutability: Mutability,
    non_escaping_call_addr_ids: &'cx HashMap<rustc_ast::NodeId, Span>,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    bad: bool,
}

impl<'ast, 'cx, 'a, 'tcx: 'a> Visitor<'ast> for ReferenceBeforeCallChecker<'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &'ast Expr) {
        if self.bad || expr.span.lo() >= self.call_span.hi() {
            return;
        }

        if let ExprKind::AddrOf(kind, mutability, operand) = &expr.kind {
            let is_current_parameter_0 = expr.id == self.exception_addr_id;
            let is_completed_non_escaping_parameter_0 = self
                .non_escaping_call_addr_ids
                .get(&expr.id)
                .map(|earlier_call_span| earlier_call_span.hi() <= self.call_span.lo())
                .unwrap_or(false);

            if !is_current_parameter_0
                && !is_completed_non_escaping_parameter_0
                && root_array_key(operand, self.cx) == Some(self.key)
            {
                let forbidden = if self.pointer_mutability == Mutability::Mut {
                    true
                } else {
                    *kind == BorrowKind::Raw || *mutability == Mutability::Mut
                };
                if forbidden {
                    self.bad = true;
                    return;
                }
            }
        }

        if root_array_key(expr, self.cx) == Some(self.key) {
            if let Some(mutability) = adjusted_reference_mutability(expr, self.cx) {
                if self.pointer_mutability == Mutability::Mut || mutability == Mutability::Mut {
                    self.bad = true;
                    return;
                }
            }
        }

        visit::walk_expr(self, expr);
    }

    fn visit_local(&mut self, local: &'ast Local) {
        if self.bad {
            return;
        }
        let init = match &local.kind {
            LocalKind::Init(init) => init,
            _ => {
                visit::walk_local(self, local);
                return;
            }
        };
        if init.span.lo() < self.call_span.hi() && root_array_key(init, self.cx) == Some(self.key) {
            if let Some(mutability) = pattern_reference_mutability(&local.pat) {
                if self.pointer_mutability == Mutability::Mut || mutability == Mutability::Mut {
                    self.bad = true;
                    return;
                }
            }
        }
        visit::walk_local(self, local);
    }

    fn visit_item(&mut self, _item: &'ast Item) {
        // A nested item is a distinct function/item scope, not part of the
        // calling function for constraint 7.d.
    }
}

fn pattern_reference_mutability(pat: &Pat) -> Option<Mutability> {
    struct Finder {
        found: Option<Mutability>,
    }
    impl<'ast> Visitor<'ast> for Finder {
        fn visit_pat(&mut self, pat: &'ast Pat) {
            if let PatKind::Ident(BindingAnnotation(ByRef::Yes, mutability), _, _) = pat.kind {
                if mutability == Mutability::Mut {
                    self.found = Some(Mutability::Mut);
                    return;
                }
                if self.found.is_none() {
                    self.found = Some(Mutability::Not);
                }
            }
            visit::walk_pat(self, pat);
        }
    }
    let mut finder = Finder { found: None };
    finder.visit_pat(pat);
    finder.found
}

fn closure_capture_before_call_is_forbidden(
    tracked: HirId,
    call_span: Span,
    pointer_mutability: Mutability,
    cx: &RefactorCtxt,
) -> bool {
    let tcx = cx.ty_ctxt();
    let hir_map = cx.hir_map();
    let parent = hir_map.get_parent_item(tracked);
    let body_id = match hir_map.maybe_body_owned_by(parent) {
        Some(id) => id,
        None => return true,
    };
    let tables = tcx.typeck_body(body_id);
    tables
        .closure_min_captures
        .values()
        .filter_map(|captures| captures.get(&tracked))
        .flatten()
        .any(|capture| {
            if capture.get_capture_kind_span(tcx).lo() >= call_span.hi() {
                return false;
            }
            match capture.info.capture_kind {
                ty::UpvarCapture::ByValue => false,
                ty::UpvarCapture::ByRef(kind) => {
                    pointer_mutability == Mutability::Mut || kind == ty::BorrowKind::MutBorrow
                }
            }
        })
}

fn root_array_key(expr: &Expr, cx: &RefactorCtxt) -> Option<ArrayKey> {
    let expr = strip_parens(expr);
    if let Some(key) = array_key_for_expr(expr, cx) {
        return Some(key);
    }
    match &expr.kind {
        ExprKind::Index(base, _) | ExprKind::Field(base, _) => root_array_key(base, cx),
        ExprKind::Unary(rustc_ast::UnOp::Deref, base) => root_array_key(base, cx),
        _ => None,
    }
}

fn array_key_for_expr(expr: &Expr, cx: &RefactorCtxt) -> Option<ArrayKey> {
    match cx.try_resolve_expr_hir(strip_parens(expr))? {
        Res::Local(id) => Some(ArrayKey::Local(id)),
        Res::Def(_, id) => Some(ArrayKey::Item(id)),
        _ => None,
    }
}

fn same_type(a: &Ty, b: &Ty, cx: &RefactorCtxt) -> bool {
    if let (Some(a_ty), Some(b_ty)) = (cx.opt_node_type(a.id), cx.opt_node_type(b.id)) {
        if a_ty == b_ty {
            return true;
        }
    }
    type_syntax_key(a, cx) == type_syntax_key(b, cx)
}

fn type_syntax_key(ty: &Ty, cx: &RefactorCtxt) -> Option<String> {
    match &ty.kind {
        TyKind::Path(None, path) => {
            if let Some(id) = cx.try_resolve_ty(ty) {
                return Some(format!("def:{:?}", id));
            }
            Some(
                path.segments
                    .iter()
                    .map(|segment| segment.ident.name.to_string())
                    .collect::<Vec<_>>()
                    .join("::"),
            )
        }
        TyKind::Ptr(mut_ty) => Some(format!(
            "*{}{}",
            if mut_ty.mutbl == Mutability::Mut {
                "mut"
            } else {
                "const"
            },
            type_syntax_key(&mut_ty.ty, cx)?
        )),
        TyKind::Rptr(_, mut_ty) => Some(format!(
            "&{}{}",
            if mut_ty.mutbl == Mutability::Mut {
                "mut"
            } else {
                ""
            },
            type_syntax_key(&mut_ty.ty, cx)?
        )),
        _ => None,
    }
}

fn simple_ty_path_name(ty: &Ty) -> Option<&str> {
    match &ty.kind {
        TyKind::Path(None, path) if !path.is_global() && path.segments.len() == 1 => {
            Some(path.segments[0].ident.name.as_str())
        }
        _ => None,
    }
}

fn simple_binding(pat: &Pat, cx: &RefactorCtxt) -> Option<(HirId, String, bool)> {
    match pat.kind {
        PatKind::Ident(BindingAnnotation(ByRef::No, mutability), ident, None) => Some((
            cx.hir_map().opt_node_to_hir_id(pat.id)?,
            ident.name.to_string(),
            mutability == Mutability::Mut,
        )),
        _ => None,
    }
}

fn expr_resolves_to_local(expr: &Expr, tracked: HirId, cx: &RefactorCtxt) -> bool {
    matches!(cx.try_resolve_expr_hir(strip_parens(expr)), Some(Res::Local(id)) if id == tracked)
}

struct ApplyCallSites<'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    calls_by_id: HashMap<rustc_ast::NodeId, (FunctionPlan, CallPlan)>,
}

impl<'cx, 'a, 'tcx: 'a> MutVisitor for ApplyCallSites<'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        if let Some((plan, call)) = self.calls_by_id.get(&expr.id).cloned() {
            if let ExprKind::Call(_, args) = &mut expr.kind {
                let mut mcx = MatchCtxt::new(self.st, self.cx);
                mcx.bindings.add("$array", call.array_expr.clone());
                let template = if plan.pointer_mutability == Mutability::Mut {
                    mcx.parse_expr("&mut $array:Expr")
                } else {
                    mcx.parse_expr("&$array:Expr")
                };
                args[call.array_index] = template.subst(self.st, self.cx, &mcx.bindings);
                args.remove(call.length_index);
            }
        }

        mut_visit::noop_visit_expr(expr, self);
    }
}

fn rewrite_planned_functions(
    items: &mut [P<Item>],
    plans_by_item: &HashMap<rustc_ast::NodeId, FunctionPlan>,
    plans_by_foreign_item: &HashMap<rustc_ast::NodeId, FunctionPlan>,
    st: &CommandState,
    cx: &RefactorCtxt,
) {
    for item in items {
        if let ItemKind::Mod(_, rustc_ast::ModKind::Loaded(module_items, ..)) = &mut item.kind {
            rewrite_planned_functions(module_items, plans_by_item, plans_by_foreign_item, st, cx);
            continue;
        }

        if let ItemKind::ForeignMod(foreign_mod) = &mut item.kind {
            for foreign_item in &mut foreign_mod.items {
                let plan = match plans_by_foreign_item.get(&foreign_item.id).cloned() {
                    Some(plan) => plan,
                    None => continue,
                };
                let func = match &mut foreign_item.kind {
                    ForeignItemKind::Fn(func) => func,
                    _ => continue,
                };
                rewrite_function_signature(func, &plan, st, cx, false);
            }
            continue;
        }

        let plan = match plans_by_item.get(&item.id).cloned() {
            Some(plan) => plan,
            None => continue,
        };
        let func = match &mut item.kind {
            ItemKind::Fn(func) => func,
            _ => continue,
        };

        if let Some(body) = &mut func.body {
            if plan.restore_length && plan.restored_length_name != plan.length_name {
                let mut length_rewriter = RestoredLengthUseRewriter {
                    st,
                    cx,
                    ids: &plan.restored_length_use_ids,
                    replacement_name: &plan.restored_length_name,
                };
                length_rewriter.visit_block(body);
            }

            let mut body_rewriter = FunctionBodyRewriter {
                st,
                cx,
                plan: &plan,
            };
            body_rewriter.visit_block(body);
            if plan.restore_length {
                insert_length_restore(body, &plan, st, cx);
            }
        }

        rewrite_function_signature(func, &plan, st, cx, true);
    }
}

fn rewrite_function_signature(
    func: &mut rustc_ast::Fn,
    plan: &FunctionPlan,
    st: &CommandState,
    cx: &RefactorCtxt,
    normalize_binding: bool,
) {
    let slice_ty = make_slice_type(st, cx, plan.element_ty.clone(), plan.pointer_mutability);
    if let Some(param) = func.sig.decl.inputs.get_mut(plan.array_index) {
        param.ty = slice_ty;
        if normalize_binding {
            if let PatKind::Ident(binding, _, None) = &mut param.pat.kind {
                *binding = BindingAnnotation(ByRef::No, Mutability::Not);
            }
        }
    }
    func.sig.decl.inputs.remove(plan.length_index);
}

struct RestoredLengthUseRewriter<'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    ids: &'cx HashSet<rustc_ast::NodeId>,
    replacement_name: &'cx str,
}

impl<'cx, 'a, 'tcx: 'a> MutVisitor for RestoredLengthUseRewriter<'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        if self.ids.contains(&expr.id) {
            *expr = self.st.parse_expr(self.cx, self.replacement_name);
            return;
        }
        mut_visit::noop_visit_expr(expr, self);
    }
}

struct FunctionBodyRewriter<'plan, 'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    plan: &'plan FunctionPlan,
}

impl<'plan, 'cx, 'a, 'tcx: 'a> MutVisitor for FunctionBodyRewriter<'plan, 'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        // The `for` shell was created by `initial_transformation()` in the same
        // Phase-3 command. Newly parsed/substituted AST nodes do not necessarily
        // have distinct/stable NodeIds before rustc rebuilds the crate. Therefore
        // do not identify a loop by the NodeId of the `for` expression itself.
        //
        // The original pointer-access expressions in the loop body are preserved
        // by the initial transformation, however, and their NodeIds still refer
        // to the pre-rewrite HIR/typeck information. Every matching loop contains
        // at least one such access by constraint 6, so those preserved body nodes
        // form a stable anchor for selecting the corresponding LoopPlan.
        let loop_plan_index = match &expr.kind {
            ExprKind::ForLoop(_, _, body, _) => self
                .plan
                .loops
                .iter()
                .position(|lp| block_contains_any_expr(body, &lp.pointer_access_ids)),
            _ => None,
        };

        if let Some(index) = loop_plan_index {
            let loop_plan = &self.plan.loops[index];
            rewrite_loop(expr, self.plan, loop_plan, self.st, self.cx);
            return;
        }
        mut_visit::noop_visit_expr(expr, self);
    }
}

fn block_contains_any_expr(block: &rustc_ast::Block, ids: &HashSet<rustc_ast::NodeId>) -> bool {
    struct Finder<'ids> {
        ids: &'ids HashSet<rustc_ast::NodeId>,
        found: bool,
    }

    impl<'ast, 'ids> Visitor<'ast> for Finder<'ids> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }
            if self.ids.contains(&expr.id) {
                self.found = true;
                return;
            }
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items are separate scopes and cannot be part of this loop
            // body's pointer access for the current function parameter.
        }
    }

    let mut finder = Finder { ids, found: false };
    finder.visit_block(block);
    finder.found
}

fn rewrite_loop(
    expr: &mut P<Expr>,
    plan: &FunctionPlan,
    loop_plan: &LoopPlan,
    st: &CommandState,
    cx: &RefactorCtxt,
) {
    let (iter, body) = match &mut expr.kind {
        ExprKind::ForLoop(_, iter, body, _) => (iter, body),
        _ => return,
    };
    let end = if loop_plan.induction_ty == IntegerType::Primitive("usize") {
        format!("{}.len()", plan.array_name)
    } else {
        format!(
            "{}.len() as {}",
            plan.array_name,
            integer_type_source(loop_plan.induction_ty)
        )
    };
    *iter = st.parse_expr(cx, &format!("0..{}", end));

    let mut replacer = PointerAccessRewriter {
        st,
        cx,
        ids: &loop_plan.pointer_access_ids,
        array_name: &plan.array_name,
        induction_name: &loop_plan.induction_name,
        induction_ty: loop_plan.induction_ty,
    };
    replacer.visit_block(body);
}

struct PointerAccessRewriter<'cx, 'a, 'tcx: 'a> {
    st: &'cx CommandState,
    cx: &'cx RefactorCtxt<'a, 'tcx>,
    ids: &'cx HashSet<rustc_ast::NodeId>,
    array_name: &'cx str,
    induction_name: &'cx str,
    induction_ty: IntegerType,
}

impl<'cx, 'a, 'tcx: 'a> MutVisitor for PointerAccessRewriter<'cx, 'a, 'tcx> {
    fn visit_expr(&mut self, expr: &mut P<Expr>) {
        if self.ids.contains(&expr.id) {
            let source = if self.induction_ty == IntegerType::Primitive("usize") {
                format!("{}[{}]", self.array_name, self.induction_name)
            } else {
                format!("{}[{} as usize]", self.array_name, self.induction_name)
            };
            *expr = self.st.parse_expr(self.cx, &source);
            return;
        }
        mut_visit::noop_visit_expr(expr, self);
    }
}

fn insert_length_restore(
    body: &mut P<rustc_ast::Block>,
    plan: &FunctionPlan,
    st: &CommandState,
    cx: &RefactorCtxt,
) {
    let mut mcx = MatchCtxt::new(st, cx);
    mcx.bindings.add("$T", plan.length_ty.clone());
    let source = if plan.length_integer_ty == IntegerType::Primitive("usize") {
        format!(
            "let {}: $T:Ty = {}.len();",
            plan.restored_length_name, plan.array_name
        )
    } else {
        format!(
            "let {}: $T:Ty = {}.len() as $T:Ty;",
            plan.restored_length_name, plan.array_name
        )
    };
    let template = mcx.parse_stmts(&source);
    let mut stmts = template.subst(st, cx, &mcx.bindings);
    for stmt in stmts.drain(..).rev() {
        body.stmts.insert(0, stmt);
    }
}

fn make_slice_type(
    st: &CommandState,
    cx: &RefactorCtxt,
    element_ty: P<Ty>,
    mutability: Mutability,
) -> P<Ty> {
    let mut mcx = MatchCtxt::new(st, cx);
    mcx.bindings.add("$T", element_ty);
    let source = if mutability == Mutability::Mut {
        "let __c2rust_slice_tmp: &mut [$T:Ty];"
    } else {
        "let __c2rust_slice_tmp: &[$T:Ty];"
    };
    let template = mcx.parse_stmts(source);
    let rewritten = template.subst(st, cx, &mcx.bindings);
    match &rewritten[0].kind {
        rustc_ast::StmtKind::Local(local) => local.ty.clone().expect("temporary slice type"),
        _ => unreachable!("temporary slice declaration did not parse as a local"),
    }
}
