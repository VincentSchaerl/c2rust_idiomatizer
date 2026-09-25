use super::TargetInfo;

use c2rust_refactor::ast_manip::visit_nodes;
use c2rust_refactor::command::CommandState;
use c2rust_refactor::matcher::{mut_visit_match_with, MatchCtxt, Subst};
use c2rust_refactor::RefactorCtxt;

use rustc_ast::ptr::P;
use rustc_ast::visit::{self, Visitor};
use rustc_ast::{
    BinOpKind, BindingAnnotation, Block, ByRef, Crate, Expr, ExprKind, Item, ItemKind, Label,
    LitIntType, LitKind, Local, LocalKind, Mutability, Pat, PatKind, Stmt, StmtKind, Ty, TyKind,
    UnOp,
};
use rustc_hir::def::Res;
use rustc_hir::def_id::DefId;
use rustc_hir::HirId;
use rustc_hir_typeck::expr_use_visitor::{Delegate, ExprUseVisitor, PlaceBase};
use rustc_infer::infer::TyCtxtInferExt;
use rustc_middle::hir::place::PlaceWithHirId;
use rustc_middle::mir::FakeReadCause;
use rustc_middle::ty::{self, ParamEnv, TyCtxt};
use rustc_span::{symbol::Symbol, Span};

use std::collections::{HashMap, HashSet};

pub(super) fn base_case(
    st: &CommandState,
    cx: &RefactorCtxt,
    target: TargetInfo,
) {
    // All semantic information is collected before the first rewrite.  The HIR
    // and typeck tables describe the pre-transform AST, so we must not try to
    // derive new semantic facts after replacing a loop.
    let krate_snapshot = {
        let krate = st.krate();
        (*krate).clone()
    };
    let type_env = TypeEnvironment::collect(&krate_snapshot, cx);
    let preceding_stmts = collect_preceding_statements(&krate_snapshot);
    let following_stmts = collect_following_statements(&krate_snapshot);

    let mut mcx = MatchCtxt::new(st, cx);
    let pattern = mcx.parse_stmts(
        r#"
            let $ipat:Pat: $T:Ty = $L:Expr;
            $'label:?Ident: while $cond:Expr {
                $body:MultiStmt;
                $incr:Stmt;
            }
        "#,
    );

    let mut krate = st.krate_mut();
    mut_visit_match_with(mcx, pattern, &mut *krate, |orig, mut mcx| {
        let ipat = match mcx.bindings.get::<_, P<Pat>>("$ipat") {
            Some(pat) => pat.clone(),
            None => return,
        };

        let induction_hir_id = match induction_binding(&ipat, cx) {
            Some(id) => id,
            None => return,
        };

        let induction_ty_ast = match mcx.bindings.get::<_, P<Ty>>("$T") {
            Some(ty) => ty.clone(),
            None => return,
        };
        let induction_ty = match integer_type_from_ty(&induction_ty_ast) {
            Some(ty) => ty,
            None => return,
        };

        let start = match mcx.bindings.get::<_, P<Expr>>("$L") {
            Some(expr) => expr.clone(),
            None => return,
        };
        let start_kind = match classify_start(&start, induction_ty, &type_env, cx) {
            Some(kind) => kind,
            None => return,
        };

        let cond = match mcx.bindings.get::<_, P<Expr>>("$cond") {
            Some(expr) => expr.clone(),
            None => return,
        };
        let condition = match classify_condition(&cond, induction_hir_id, cx) {
            Some(condition) => condition,
            None => return,
        };

        let while_expr = match orig.get(1).and_then(stmt_expr) {
            Some(expr) => expr,
            None => return,
        };
        let (while_body, while_label) = match &while_expr.kind {
            ExprKind::While(_, body, label) => (body, label.as_ref()),
            _ => return,
        };

        // Work with the HIR roots of the loop body instead of requiring an
        // AST -> HIR mapping for the `while` statement itself.  In particular,
        // the final `while` in a block can be lowered as the block's tail
        // expression and then has no mapped AST Stmt NodeId in C2Rust.
        let loop_body_hir_ids = mapped_expr_hir_ids_in_statements(&while_body.stmts, cx);

        let preceding = match preceding_stmts.get(&while_expr.id) {
            Some(stmts) => stmts,
            None => return,
        };
        let following = match following_stmts.get(&while_expr.id) {
            Some(stmts) => stmts,
            None => return,
        };

        let bound_kind = match classify_bound(
            &condition.bound,
            induction_ty,
            while_expr.span,
            &loop_body_hir_ids,
            preceding,
            &type_env,
            cx,
            target,
            condition.direction,
        ) {
            Some(kind) => kind,
            None => return,
        };

        let incr_stmt = match mcx.bindings.get::<_, Stmt>("$incr") {
            Some(stmt) => stmt,
            None => return,
        };
        let incr_expr = match &incr_stmt.kind {
            StmtKind::Semi(expr) | StmtKind::Expr(expr) => expr,
            _ => return,
        };
        let increment = match classify_increment(incr_expr, induction_hir_id, induction_ty, cx) {
            Some(increment) => increment,
            None => return,
        };

        let step = match classify_step(
            increment,
            induction_ty,
            while_expr.span,
            &loop_body_hir_ids,
            preceding,
            &type_env,
            cx,
            target,
        ) {
            Some(step) => step,
            None => return,
        };

        if !direction_matches_step(condition.direction, step.operation, step.value) {
            return;
        }

        // Constraint 6: a continue targeting this while loop would skip the
        // increment in the old loop but not in the for loop.
        if body_has_continue_to_loop(while_body, while_label) {
            return;
        }

        // Constraints 7 and 8. ExprUseVisitor sees explicit borrows as well as
        // type-adjustment/autoref borrows, so it also covers implicit method
        // receiver borrowing. The increment is excluded because it is removed.
        // Constraint 7 applies to the original loop *body*, not to the
        // while condition and not to the increment that will be removed.
        // ExprUseVisitor can report borrow-like uses for expressions in the
        // condition/update, so pass only the body statements before the
        // increment as the region in which shared/reference borrows are
        // forbidden.
        let checked_body_len = while_body.stmts.len().saturating_sub(1);
        let checked_body = &while_body.stmts[..checked_body_len];
        if !induction_variable_usage_is_safe(
            induction_hir_id,
            &loop_body_hir_ids,
            checked_body,
            following,
            cx,
        ) {
            return;
        }

        let start_for_ascending = match &start_kind {
            StartKind::Literal(value) => typed_literal_expr(st, cx, *value, induction_ty),
            StartKind::TValue | StartKind::CastValue => start.clone(),
        };
        let start_for_descending = start.clone();

        let bound_for_ascending = normalize_bound_for_ascending(&condition.bound);
        let bound_for_descending_inclusive = match &bound_kind {
            BoundKind::Literal(value) => typed_literal_expr(st, cx, *value, induction_ty),
            BoundKind::Stable | BoundKind::CastStable => condition.bound.clone(),
        };
        let bound_for_descending_strict = match &bound_kind {
            BoundKind::Literal(value) => {
                let plus_one = match add_one(*value) {
                    Some(value) => value,
                    None => return,
                };
                typed_literal_expr(st, cx, plus_one, induction_ty)
            }
            BoundKind::Stable | BoundKind::CastStable => {
                make_add_one_expr(st, cx, condition.bound.clone())
            }
        };

        let step_expr = make_step_by_argument(st, cx, &step);
        let use_step_by = step.value.abs_magnitude() != 1;

        let range = match condition.direction {
            Direction::AscendingExclusive => make_range_expr(
                st,
                cx,
                start_for_ascending,
                bound_for_ascending,
                false,
                false,
                if use_step_by { Some(step_expr) } else { None },
            ),
            Direction::AscendingInclusive => make_range_expr(
                st,
                cx,
                start_for_ascending,
                bound_for_ascending,
                true,
                false,
                if use_step_by { Some(step_expr) } else { None },
            ),
            Direction::DescendingStrict => make_range_expr(
                st,
                cx,
                bound_for_descending_strict,
                start_for_descending,
                true,
                true,
                if use_step_by { Some(step_expr) } else { None },
            ),
            Direction::DescendingInclusive => make_range_expr(
                st,
                cx,
                bound_for_descending_inclusive,
                start_for_descending,
                true,
                true,
                if use_step_by { Some(step_expr) } else { None },
            ),
        };

        // Reuse the old binding node, but the for binding is immutable. The
        // specification guarantees that the body never writes I.
        let mut for_pat = ipat.clone();
        match &mut for_pat.kind {
            PatKind::Ident(binding, _, None) => {
                *binding = BindingAnnotation(ByRef::No, Mutability::Not);
            }
            _ => return,
        }

        mcx.bindings.add("$for_pat", for_pat);
        mcx.bindings.add("$range_out", range);

        let replacement = mcx.parse_stmts(
            r#"
                $'label: for $for_pat:Pat in $range_out:Expr {
                    $body;
                }
            "#,
        );
        *orig = replacement.subst(st, cx, &mcx.bindings);
    });
}

// -----------------------------------------------------------------------------
// Source-level type information
// -----------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum CIntegerType {
    SChar,
    UChar,
    Short,
    UShort,
    Int,
    UInt,
    Long,
    ULong,
    LongLong,
    ULongLong,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum IntegerType {
    Primitive(&'static str),
    C(CIntegerType),
}

#[derive(Clone, Debug)]
enum TypeInfo {
    Integer(IntegerType),
    Struct(DefId),
    Pointer(Box<TypeInfo>),
    Array(Box<TypeInfo>),
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum BaseOrigin {
    Param,
    Local,
    Const,
    Static,
}

#[derive(Clone, Debug)]
struct BaseMeta {
    type_info: TypeInfo,
    mutable: bool,
    origin: BaseOrigin,
}

#[derive(Clone, Debug, Default)]
struct TypeEnvironment {
    locals: HashMap<HirId, BaseMeta>,
    items: HashMap<DefId, BaseMeta>,
    struct_fields: HashMap<DefId, HashMap<String, TypeInfo>>,

    // A `static` or `const` appearing as a statement inside a block is a
    // function-local item.  This distinction matters for stable array/struct
    // values: constraints 4.b.ix / 4.b.xiv permit a mutable container defined
    // inside the function when the no-reference/no-modification conditions are
    // proven, while a mutable global item is not accepted.
    local_item_ids: HashSet<DefId>,

    // Syntactic/rustc-adjustment evidence used by the stability proof.  We do
    // not equate every ExprUseVisitor `borrow` event with "a reference was
    // created": rustc also reports borrow-like internal uses for ordinary
    // reads.  These maps record the reference-creation forms that the
    // specification actually forbids.
    forbidden_reference_spans: HashMap<BaseKey, Vec<Span>>,
    mutation_spans: HashMap<BaseKey, Vec<Span>>,

    // For `ref`/`ref mut` patterns we deliberately keep a conservative
    // function-wide guard. If such syntax occurs before or during a candidate
    // loop, mutable stable values are rejected unless a more precise
    // pattern-to-base analysis is added later.
    by_ref_pattern_spans: Vec<Span>,
}

impl TypeEnvironment {
    fn collect(krate: &Crate, cx: &RefactorCtxt) -> Self {
        let mut env = TypeEnvironment::default();

        // Mark item statements (most importantly function-local `static`s).
        // Module/global items are not block statements and therefore are not
        // inserted here.
        visit_nodes(krate, |block: &Block| {
            for stmt in &block.stmts {
                if let StmtKind::Item(item) = &stmt.kind {
                    env.local_item_ids.insert(cx.node_def_id(item.id));
                }
            }
        });

        visit_nodes(krate, |item: &Item| match &item.kind {
            ItemKind::Struct(variant_data, _) => {
                let mut fields = HashMap::new();
                for field in variant_data.fields() {
                    if let Some(ident) = field.ident {
                        if let Some(type_info) = type_info_from_ty(&field.ty, cx) {
                            fields.insert(ident.name.to_string(), type_info);
                        }
                    }
                }
                env.struct_fields.insert(cx.node_def_id(item.id), fields);
            }
            ItemKind::Static(ty, mutability, _) => {
                if let Some(type_info) = type_info_from_ty(ty, cx) {
                    env.items.insert(
                        cx.node_def_id(item.id),
                        BaseMeta {
                            type_info,
                            mutable: *mutability == Mutability::Mut,
                            origin: BaseOrigin::Static,
                        },
                    );
                }
            }
            ItemKind::Const(_, ty, _) => {
                if let Some(type_info) = type_info_from_ty(ty, cx) {
                    env.items.insert(
                        cx.node_def_id(item.id),
                        BaseMeta {
                            type_info,
                            mutable: false,
                            origin: BaseOrigin::Const,
                        },
                    );
                }
            }
            ItemKind::Fn(func) => {
                for param in &func.sig.decl.inputs {
                    let (hir_id, mutable) = match simple_binding_hir_id(&param.pat, cx) {
                        Some(value) => value,
                        None => continue,
                    };
                    if let Some(type_info) = type_info_from_ty(&param.ty, cx) {
                        env.locals.insert(
                            hir_id,
                            BaseMeta {
                                type_info,
                                mutable,
                                origin: BaseOrigin::Param,
                            },
                        );
                    }
                }
            }
            _ => {}
        });

        visit_nodes(krate, |local: &Local| {
            let (hir_id, mutable) = match simple_binding_hir_id(&local.pat, cx) {
                Some(value) => value,
                None => return,
            };
            let type_info = local
                .ty
                .as_ref()
                .and_then(|ty| type_info_from_ty(ty, cx))
                .or_else(|| cx.opt_node_type(local.pat.id).and_then(type_info_from_rustc_ty));
            if let Some(type_info) = type_info {
                env.locals.insert(
                    hir_id,
                    BaseMeta {
                        type_info,
                        mutable,
                        origin: BaseOrigin::Local,
                    },
                );
            }
        });

        collect_stability_syntax_facts(krate, cx, &mut env);
        env
    }

    fn base_meta(&self, base: BaseKey) -> Option<&BaseMeta> {
        match base {
            BaseKey::Local(id) => self.locals.get(&id),
            BaseKey::Item(id) => self.items.get(&id),
        }
    }

    fn item_is_defined_inside_function(&self, id: DefId) -> bool {
        self.local_item_ids.contains(&id)
    }

    fn has_forbidden_reference_before_or_during(
        &self,
        base: BaseKey,
        while_span: Span,
    ) -> bool {
        if self
            .forbidden_reference_spans
            .get(&base)
            .map(|spans| spans.iter().any(|span| span.lo() < while_span.hi()))
            .unwrap_or(false)
        {
            return true;
        }

        // `ref` / `ref mut` patterns are kept as a conservative
        // function-wide guard because associating every binding pattern back
        // to its exact base would require a separate pattern-to-place analysis.
        self.by_ref_pattern_spans
            .iter()
            .any(|span| span.lo() < while_span.hi())
    }

    fn base_modified_during_loop(&self, base: BaseKey, while_span: Span) -> bool {
        self.mutation_spans
            .get(&base)
            .map(|spans| {
                spans.iter().any(|span| {
                    while_span.lo() <= span.lo() && span.hi() <= while_span.hi()
                })
            })
            .unwrap_or(false)
    }

    fn type_info_for_expr(&self, expr: &Expr, cx: &RefactorCtxt) -> Option<TypeInfo> {
        // Prefer the source-level environment because it preserves C integer
        // aliases when they are still present.  Fall back to rustc's typeck
        // result for expressions whose defining item is not represented by an
        // AST Item in this crate, most importantly `extern { static ... }`
        // declarations produced by C2Rust.
        let structural = match &expr.kind {
            ExprKind::Path(..) => match cx.try_resolve_expr_hir(expr) {
                Some(Res::Local(id)) => self.locals.get(&id).map(|meta| meta.type_info.clone()),
                Some(Res::Def(_, id)) => self.items.get(&id).map(|meta| meta.type_info.clone()),
                _ => None,
            },
            ExprKind::Unary(UnOp::Deref, inner) => match self.type_info_for_expr(inner, cx) {
                Some(TypeInfo::Pointer(inner)) => Some(*inner),
                _ => None,
            },
            ExprKind::Index(base, _) => match self.type_info_for_expr(base, cx) {
                Some(TypeInfo::Array(element)) => Some(*element),
                _ => None,
            },
            ExprKind::Field(base, ident) => {
                let struct_id = match self.type_info_for_expr(base, cx) {
                    Some(TypeInfo::Struct(id)) => id,
                    _ => return cx.opt_node_type(expr.id).and_then(type_info_from_rustc_ty),
                };
                self.struct_fields
                    .get(&struct_id)
                    .and_then(|fields| fields.get(&ident.name.to_string()))
                    .cloned()
            }
            ExprKind::MethodCall(segment, receiver, _, _)
                if segment.ident.name.as_str() == "offset" =>
            {
                self.type_info_for_expr(receiver, cx)
            }
            ExprKind::Paren(inner) => self.type_info_for_expr(inner, cx),
            _ => None,
        };

        structural.or_else(|| cx.opt_node_type(expr.id).and_then(type_info_from_rustc_ty))
    }
}

fn simple_binding_hir_id(pat: &Pat, cx: &RefactorCtxt) -> Option<(HirId, bool)> {
    let mutable = match pat.kind {
        PatKind::Ident(BindingAnnotation(ByRef::No, mutability), _, None) => {
            mutability == Mutability::Mut
        }
        _ => return None,
    };
    Some((cx.hir_map().opt_node_to_hir_id(pat.id)?, mutable))
}

fn induction_binding(pat: &Pat, cx: &RefactorCtxt) -> Option<HirId> {
    match pat.kind {
        PatKind::Ident(BindingAnnotation(ByRef::No, Mutability::Mut), _, None) => {
            cx.hir_map().opt_node_to_hir_id(pat.id)
        }
        _ => None,
    }
}

fn type_info_from_ty(ty: &Ty, cx: &RefactorCtxt) -> Option<TypeInfo> {
    if let Some(integer) = integer_type_from_ty(ty) {
        return Some(TypeInfo::Integer(integer));
    }

    match &ty.kind {
        TyKind::Path(None, _) => cx.try_resolve_ty(ty).map(TypeInfo::Struct),
        TyKind::Ptr(mut_ty) => {
            let inner = type_info_from_ty(&mut_ty.ty, cx)?;
            Some(TypeInfo::Pointer(Box::new(inner)))
        }
        TyKind::Array(element, _) => {
            let inner = type_info_from_ty(element, cx)?;
            Some(TypeInfo::Array(Box::new(inner)))
        }
        _ => None,
    }
}

fn type_info_from_rustc_ty(ty: ty::Ty<'_>) -> Option<TypeInfo> {
    let integer = match ty.kind() {
        ty::TyKind::Int(ty::IntTy::I8) => Some(IntegerType::Primitive("i8")),
        ty::TyKind::Int(ty::IntTy::I16) => Some(IntegerType::Primitive("i16")),
        ty::TyKind::Int(ty::IntTy::I32) => Some(IntegerType::Primitive("i32")),
        ty::TyKind::Int(ty::IntTy::I64) => Some(IntegerType::Primitive("i64")),
        ty::TyKind::Int(ty::IntTy::I128) => Some(IntegerType::Primitive("i128")),
        ty::TyKind::Int(ty::IntTy::Isize) => Some(IntegerType::Primitive("isize")),
        ty::TyKind::Uint(ty::UintTy::U8) => Some(IntegerType::Primitive("u8")),
        ty::TyKind::Uint(ty::UintTy::U16) => Some(IntegerType::Primitive("u16")),
        ty::TyKind::Uint(ty::UintTy::U32) => Some(IntegerType::Primitive("u32")),
        ty::TyKind::Uint(ty::UintTy::U64) => Some(IntegerType::Primitive("u64")),
        ty::TyKind::Uint(ty::UintTy::U128) => Some(IntegerType::Primitive("u128")),
        ty::TyKind::Uint(ty::UintTy::Usize) => Some(IntegerType::Primitive("usize")),
        _ => None,
    };
    if let Some(integer) = integer {
        return Some(TypeInfo::Integer(integer));
    }

    match ty.kind() {
        ty::TyKind::RawPtr(mut_ty) => {
            let inner = type_info_from_rustc_ty(mut_ty.ty)?;
            Some(TypeInfo::Pointer(Box::new(inner)))
        }
        ty::TyKind::Array(element, _) => {
            let inner = type_info_from_rustc_ty(*element)?;
            Some(TypeInfo::Array(Box::new(inner)))
        }
        ty::TyKind::Adt(def, _) => Some(TypeInfo::Struct(def.did())),
        _ => None,
    }
}

fn integer_type_from_ty(ty: &Ty) -> Option<IntegerType> {
    let path = match &ty.kind {
        TyKind::Path(None, path) => path,
        _ => return None,
    };

    if !path.is_global() && path.segments.len() == 1 {
        return match path.segments[0].ident.name.as_str() {
            "i8" => Some(IntegerType::Primitive("i8")),
            "i16" => Some(IntegerType::Primitive("i16")),
            "i32" => Some(IntegerType::Primitive("i32")),
            "i64" => Some(IntegerType::Primitive("i64")),
            "i128" => Some(IntegerType::Primitive("i128")),
            "isize" => Some(IntegerType::Primitive("isize")),
            "u8" => Some(IntegerType::Primitive("u8")),
            "u16" => Some(IntegerType::Primitive("u16")),
            "u32" => Some(IntegerType::Primitive("u32")),
            "u64" => Some(IntegerType::Primitive("u64")),
            "u128" => Some(IntegerType::Primitive("u128")),
            "usize" => Some(IntegerType::Primitive("usize")),
            _ => None,
        };
    }

    if path.is_global()
        && path.segments.len() == 4
        && path.segments[1].ident.name.as_str() == "core"
        && path.segments[2].ident.name.as_str() == "ffi"
    {
        return match path.segments[3].ident.name.as_str() {
            "c_schar" => Some(IntegerType::C(CIntegerType::SChar)),
            "c_uchar" => Some(IntegerType::C(CIntegerType::UChar)),
            "c_short" => Some(IntegerType::C(CIntegerType::Short)),
            "c_ushort" => Some(IntegerType::C(CIntegerType::UShort)),
            "c_int" => Some(IntegerType::C(CIntegerType::Int)),
            "c_uint" => Some(IntegerType::C(CIntegerType::UInt)),
            "c_long" => Some(IntegerType::C(CIntegerType::Long)),
            "c_ulong" => Some(IntegerType::C(CIntegerType::ULong)),
            "c_longlong" => Some(IntegerType::C(CIntegerType::LongLong)),
            "c_ulonglong" => Some(IntegerType::C(CIntegerType::ULongLong)),
            _ => None,
        };
    }

    None
}

// -----------------------------------------------------------------------------
// T-values and stable values
// -----------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
enum BaseKey {
    Local(HirId),
    Item(DefId),
}

#[derive(Clone, Debug, PartialEq, Eq)]
enum PlaceProjection {
    Index(IntValue),
    Field(String),
}

#[derive(Clone, Debug, PartialEq, Eq)]
struct PlaceKey {
    base: BaseKey,
    projections: Vec<PlaceProjection>,
}

#[derive(Clone, Debug)]
struct StableValue {
    value_type: IntegerType,
    place: PlaceKey,
}

fn place_key(expr: &Expr, cx: &RefactorCtxt, target: TargetInfo) -> Option<PlaceKey> {
    match &expr.kind {
        ExprKind::Path(..) => {
            let base = match cx.try_resolve_expr_hir(expr)? {
                Res::Local(id) => BaseKey::Local(id),
                Res::Def(_, id) => BaseKey::Item(id),
                _ => return None,
            };
            Some(PlaceKey {
                base,
                projections: Vec::new(),
            })
        }
        ExprKind::Index(base, index) => {
            let mut key = place_key(base, cx, target)?;
            let index = normalized_integer_literal_expression_value(index, target)?;
            key.projections.push(PlaceProjection::Index(index));
            Some(key)
        }
        ExprKind::Field(base, ident) => {
            let mut key = place_key(base, cx, target)?;
            key.projections
                .push(PlaceProjection::Field(ident.name.to_string()));
            Some(key)
        }
        _ => None,
    }
}

fn root_base(expr: &Expr, cx: &RefactorCtxt) -> Option<BaseKey> {
    match &expr.kind {
        ExprKind::Path(..) => match cx.try_resolve_expr_hir(expr)? {
            Res::Local(id) => Some(BaseKey::Local(id)),
            Res::Def(_, id) => Some(BaseKey::Item(id)),
            _ => None,
        },
        ExprKind::Index(base, _) | ExprKind::Field(base, _) | ExprKind::Paren(base) => {
            root_base(base, cx)
        }
        _ => None,
    }
}


fn rustc_ty_is_reference(ty: ty::Ty<'_>) -> bool {
    matches!(ty.kind(), ty::TyKind::Ref(..))
}

fn expression_introduces_autoref(expr: &Expr, cx: &RefactorCtxt) -> bool {
    let original = match cx.opt_node_type(expr.id) {
        Some(ty) => ty,
        None => return false,
    };
    let adjusted = match cx.opt_adjusted_node_type(expr.id) {
        Some(ty) => ty,
        None => return false,
    };

    !rustc_ty_is_reference(original) && rustc_ty_is_reference(adjusted)
}

fn record_base_span(
    map: &mut HashMap<BaseKey, Vec<Span>>,
    base: BaseKey,
    span: Span,
) {
    map.entry(base).or_default().push(span);
}

fn collect_stability_syntax_facts(
    krate: &Crate,
    cx: &RefactorCtxt,
    env: &mut TypeEnvironment,
) {
    // Explicit `&`, `&mut`, `&raw const`, `&raw mut`, rustc-introduced
    // autoref adjustments, and direct writes. Closure/async captures are
    // checked separately from rustc's capture information so that only
    // captures by reference are forbidden; captures by value are allowed.
    visit_nodes(krate, |expr: &Expr| {
        match &expr.kind {
            ExprKind::AddrOf(_, _, operand) => {
                if let Some(base) = root_base(operand, cx) {
                    record_base_span(
                        &mut env.forbidden_reference_spans,
                        base,
                        expr.span,
                    );
                }
            }
            ExprKind::Assign(lhs, _, _) | ExprKind::AssignOp(_, lhs, _) => {
                if let Some(base) = root_base(lhs, cx) {
                    record_base_span(&mut env.mutation_spans, base, expr.span);
                }
            }
            _ => {}
        }

        if expression_introduces_autoref(expr, cx) {
            if let Some(base) = root_base(expr, cx) {
                record_base_span(
                    &mut env.forbidden_reference_spans,
                    base,
                    expr.span,
                );
            }
        }
    });

    // `ref` / `ref mut` binding patterns are another explicitly forbidden
    // reference-creation form. `visit_nodes` in the pinned C2Rust version does
    // not implement VisitNode for `Pat`, so traverse patterns with rustc_ast's
    // normal Visitor instead.
    struct ByRefPatternCollector<'a> {
        spans: &'a mut Vec<Span>,
    }

    impl<'ast, 'a> Visitor<'ast> for ByRefPatternCollector<'a> {
        fn visit_pat(&mut self, pat: &'ast Pat) {
            if matches!(
                pat.kind,
                PatKind::Ident(BindingAnnotation(ByRef::Yes, _), _, _)
            ) {
                self.spans.push(pat.span);
            }
            visit::walk_pat(self, pat);
        }
    }

    let mut collector = ByRefPatternCollector {
        spans: &mut env.by_ref_pattern_spans,
    };
    collector.visit_crate(krate);
}

fn is_t_value(expr: &Expr, expected: IntegerType, env: &TypeEnvironment, cx: &RefactorCtxt) -> bool {
    let actual = match env.type_info_for_expr(expr, cx) {
        Some(TypeInfo::Integer(integer)) => integer,
        _ => return false,
    };
    if actual != expected {
        return false;
    }

    match &expr.kind {
        ExprKind::Path(..) => true,
        ExprKind::Unary(UnOp::Deref, _) => true,
        ExprKind::Index(..) => true,
        ExprKind::Field(..) => true,
        _ => false,
    }
}

fn stable_value(
    expr: &Expr,
    while_span: Span,
    loop_body_hir_ids: &[HirId],
    env: &TypeEnvironment,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> Option<StableValue> {
    let value_type = match env.type_info_for_expr(expr, cx)? {
        TypeInfo::Integer(integer) => integer,
        _ => return None,
    };
    let place = place_key(expr, cx, target)?;
    let base_meta = env.base_meta(place.base)?;

    let is_plain_value = place.projections.is_empty();
    let is_array_element = !place.projections.is_empty()
        && place
            .projections
            .iter()
            .all(|projection| matches!(projection, PlaceProjection::Index(_)));
    let is_struct_field = !place.projections.is_empty()
        && place
            .projections
            .iter()
            .all(|projection| matches!(projection, PlaceProjection::Field(_)));

    if is_plain_value {
        if !matches!(base_meta.type_info, TypeInfo::Integer(_)) {
            return None;
        }
    } else if is_array_element {
        if !matches!(base_meta.type_info, TypeInfo::Array(_)) {
            return None;
        }
    } else if is_struct_field {
        if !matches!(base_meta.type_info, TypeInfo::Struct(_)) {
            return None;
        }
    } else {
        // Mixed projections (for example s.a[0]) are deliberately rejected:
        // they are not one of the base-case stable-value shapes enumerated in
        // the specification.
        return None;
    }

    // Whether the candidate needs the dynamic "no reference / not modified"
    // proof. Immutable/const cases are accepted directly;
    // mutable locals/parameters and mutable function-local array/struct
    // statics are permitted only through the corresponding dynamic clauses.
    let needs_dynamic_proof = match place.base {
        BaseKey::Item(id) => match base_meta.origin {
            BaseOrigin::Const => false,
            BaseOrigin::Static if !base_meta.mutable => false,
            BaseOrigin::Static
                if base_meta.mutable
                    && env.item_is_defined_inside_function(id)
                    && (is_array_element || is_struct_field) =>
            {
                // Constraints 4.b.ix / 4.b.xiv.
                true
            }
            _ => return None,
        },
        BaseKey::Local(_) => match base_meta.origin {
            BaseOrigin::Param | BaseOrigin::Local => base_meta.mutable,
            _ => return None,
        },
    };

    if needs_dynamic_proof {
        // Do not use ExprUseVisitor's generic `borrow` event as the definition
        // of "a reference was created": it is also emitted for some ordinary
        // reads.  Instead use the source/adjustment facts collected above,
        // which correspond to the forbidden forms in the specification.
        if env.has_forbidden_reference_before_or_during(place.base, while_span) {
            return None;
        }

        // This AST-level check also works for function-local `static mut`
        // containers, whose base is an Item rather than a Local.
        if env.base_modified_during_loop(place.base, while_span) {
            return None;
        }

        // Keep rustc's use analysis as an additional proof for ordinary
        // locals/parameters. It catches writes that are not obvious from the
        // surface AST and, via closure_min_captures, checks the fourth
        // forbidden reference-creation form: capture by reference by a
        // closure, async closure, or async block. Captures by value are not
        // rejected; there is no separate thread-sharing condition anymore.
        if let BaseKey::Local(id) = place.base {
            let facts = local_stability_facts(id, while_span, loop_body_hir_ids, cx)?;
            if facts.captured_by_reference_before_or_during || facts.modified_during_loop {
                return None;
            }
        }
    }

    Some(StableValue { value_type, place })
}

// -----------------------------------------------------------------------------
// Candidate classification
// -----------------------------------------------------------------------------

#[derive(Clone, Copy, Debug)]
enum StartKind {
    Literal(IntValue),
    TValue,
    CastValue,
}

fn classify_start(
    expr: &Expr,
    induction_ty: IntegerType,
    env: &TypeEnvironment,
    cx: &RefactorCtxt,
) -> Option<StartKind> {
    if let Some(value) = unsuffixed_integer_literal_value(expr) {
        return Some(StartKind::Literal(value));
    }

    if is_t_value(expr, induction_ty, env, cx) {
        return Some(StartKind::TValue);
    }

    if let ExprKind::Cast(inner, cast_ty) = &expr.kind {
        if integer_type_from_ty(cast_ty) != Some(induction_ty) {
            return None;
        }
        let source_ty = match env.type_info_for_expr(inner, cx)? {
            TypeInfo::Integer(integer) => integer,
            _ => return None,
        };
        if source_ty == induction_ty || !is_t_value(inner, source_ty, env, cx) {
            return None;
        }
        return Some(StartKind::CastValue);
    }

    None
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Direction {
    AscendingExclusive,
    AscendingInclusive,
    DescendingStrict,
    DescendingInclusive,
}

#[derive(Clone, Debug)]
struct ConditionInfo {
    direction: Direction,
    bound: P<Expr>,
}

fn classify_condition(cond: &Expr, induction_hir_id: HirId, cx: &RefactorCtxt) -> Option<ConditionInfo> {
    let (op, left, right) = match &cond.kind {
        ExprKind::Binary(op, left, right) => (op.node, left, right),
        _ => return None,
    };

    let left_is_i = resolves_to_local(left, induction_hir_id, cx);
    let right_is_i = resolves_to_local(right, induction_hir_id, cx);
    if left_is_i == right_is_i {
        return None;
    }

    let (direction, bound) = if left_is_i {
        let direction = match op {
            BinOpKind::Lt => Direction::AscendingExclusive,
            BinOpKind::Le => Direction::AscendingInclusive,
            BinOpKind::Gt => Direction::DescendingStrict,
            BinOpKind::Ge => Direction::DescendingInclusive,
            _ => return None,
        };
        (direction, right.clone())
    } else {
        let direction = match op {
            BinOpKind::Gt => Direction::AscendingExclusive,
            BinOpKind::Ge => Direction::AscendingInclusive,
            BinOpKind::Lt => Direction::DescendingStrict,
            BinOpKind::Le => Direction::DescendingInclusive,
            _ => return None,
        };
        (direction, left.clone())
    };

    Some(ConditionInfo { direction, bound })
}

#[derive(Clone, Copy, Debug)]
enum BoundKind {
    Literal(IntValue),
    Stable,
    CastStable,
}

fn classify_bound(
    bound: &Expr,
    induction_ty: IntegerType,
    while_span: Span,
    loop_body_hir_ids: &[HirId],
    preceding: &[Stmt],
    env: &TypeEnvironment,
    cx: &RefactorCtxt,
    target: TargetInfo,
    direction: Direction,
) -> Option<BoundKind> {
    if let Some(value) = bound_literal_value(bound) {
        if !integer_value_fits_type(value, induction_ty, target) {
            return None;
        }
        if direction == Direction::DescendingStrict {
            let plus_one = add_one(value)?;
            if !integer_value_fits_type(plus_one, induction_ty, target) {
                return None;
            }
        }
        return Some(BoundKind::Literal(value));
    }

    if let Some(stable) = stable_value(bound, while_span, loop_body_hir_ids, env, cx, target) {
        if stable.value_type == induction_ty {
            if direction == Direction::DescendingStrict {
                let assigned = last_direct_literal_assignment(&stable.place, preceding, cx, target)?;
                if !integer_value_fits_type(assigned, induction_ty, target) {
                    return None;
                }
                let plus_one = add_one(assigned)?;
                if !integer_value_fits_type(plus_one, induction_ty, target) {
                    return None;
                }
            }
            return Some(BoundKind::Stable);
        }
    }

    if let ExprKind::Cast(inner, cast_ty) = &bound.kind {
        if integer_type_from_ty(cast_ty) != Some(induction_ty) {
            return None;
        }
        let stable = stable_value(inner, while_span, loop_body_hir_ids, env, cx, target)?;
        if stable.value_type == induction_ty {
            return None;
        }

        if direction == Direction::DescendingStrict {
            let assigned = last_direct_literal_assignment(&stable.place, preceding, cx, target)?;
            if !integer_value_fits_type(assigned, stable.value_type, target) {
                return None;
            }
            let cast_value = cast_integer_value(assigned, induction_ty, target);
            let plus_one = add_one(cast_value)?;
            if !integer_value_fits_type(plus_one, induction_ty, target) {
                return None;
            }
        }
        return Some(BoundKind::CastStable);
    }

    None
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum StepOperation {
    Add,
    Sub,
}

#[derive(Clone, Debug)]
struct IncrementInfo {
    operation: StepOperation,
    step_expr: P<Expr>,
    cast_to_induction_type: bool,
}

fn classify_increment(
    expr: &Expr,
    induction_hir_id: HirId,
    induction_ty: IntegerType,
    cx: &RefactorCtxt,
) -> Option<IncrementInfo> {
    match &expr.kind {
        ExprKind::AssignOp(op, lhs, rhs) => {
            if !resolves_to_local(lhs, induction_hir_id, cx) {
                return None;
            }
            let operation = match op.node {
                BinOpKind::Add => StepOperation::Add,
                BinOpKind::Sub => StepOperation::Sub,
                _ => return None,
            };
            let (step_expr, cast) = strip_step_cast(rhs, induction_ty)?;
            Some(IncrementInfo {
                operation,
                step_expr,
                cast_to_induction_type: cast,
            })
        }
        ExprKind::Assign(lhs, rhs, _) => {
            if !resolves_to_local(lhs, induction_hir_id, cx) {
                return None;
            }

            if let ExprKind::Binary(op, left, right) = &rhs.kind {
                match op.node {
                    BinOpKind::Add if resolves_to_local(left, induction_hir_id, cx) => {
                        let (step_expr, cast) = strip_step_cast(right, induction_ty)?;
                        return Some(IncrementInfo {
                            operation: StepOperation::Add,
                            step_expr,
                            cast_to_induction_type: cast,
                        });
                    }
                    BinOpKind::Sub if resolves_to_local(left, induction_hir_id, cx) => {
                        let (step_expr, cast) = strip_step_cast(right, induction_ty)?;
                        return Some(IncrementInfo {
                            operation: StepOperation::Sub,
                            step_expr,
                            cast_to_induction_type: cast,
                        });
                    }
                    BinOpKind::Add if resolves_to_local(right, induction_hir_id, cx) => {
                        let (step_expr, cast) = strip_step_cast(left, induction_ty)?;
                        return Some(IncrementInfo {
                            operation: StepOperation::Add,
                            step_expr,
                            cast_to_induction_type: cast,
                        });
                    }
                    _ => {}
                }
            }

            if let ExprKind::MethodCall(segment, receiver, args, _) = &rhs.kind {
                if args.len() != 1 {
                    return None;
                }
                let method = segment.ident.name.as_str();

                if resolves_to_local(receiver, induction_hir_id, cx) {
                    let operation = match method {
                        "wrapping_add" => StepOperation::Add,
                        "wrapping_sub" => StepOperation::Sub,
                        _ => return None,
                    };
                    let (step_expr, cast) = strip_step_cast(&args[0], induction_ty)?;
                    return Some(IncrementInfo {
                        operation,
                        step_expr,
                        cast_to_induction_type: cast,
                    });
                }

                // The only reversed wrapping form allowed by the specification
                // is (STEP as T).wrapping_add(I).
                if method == "wrapping_add" && resolves_to_local(&args[0], induction_hir_id, cx) {
                    let receiver = strip_parentheses(receiver);
                    if let ExprKind::Cast(step, cast_ty) = &receiver.kind {
                        if integer_type_from_ty(cast_ty) == Some(induction_ty) {
                            return Some(IncrementInfo {
                                operation: StepOperation::Add,
                                step_expr: step.clone(),
                                cast_to_induction_type: true,
                            });
                        }
                    }
                }
            }

            None
        }
        _ => None,
    }
}

fn strip_step_cast(expr: &P<Expr>, induction_ty: IntegerType) -> Option<(P<Expr>, bool)> {
    if let ExprKind::Cast(inner, cast_ty) = &expr.kind {
        if integer_type_from_ty(cast_ty) == Some(induction_ty) {
            return Some((inner.clone(), true));
        }
        // A cast to some other type is not one of the permitted STEP forms.
        return None;
    }
    Some((expr.clone(), false))
}

#[derive(Clone, Debug)]
struct StepInfo {
    operation: StepOperation,
    source_expr: P<Expr>,
    source_type: Option<IntegerType>,
    value: IntValue,
}

fn classify_step(
    increment: IncrementInfo,
    induction_ty: IntegerType,
    while_span: Span,
    loop_body_hir_ids: &[HirId],
    preceding: &[Stmt],
    env: &TypeEnvironment,
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> Option<StepInfo> {
    let (value, source_type) = if let Some(value) = unsuffixed_integer_literal_value(&increment.step_expr) {
        (value, None)
    } else {
        let stable = stable_value(&increment.step_expr, while_span, loop_body_hir_ids, env, cx, target)?;
        let assigned = last_direct_literal_assignment(&stable.place, preceding, cx, target)?;
        if !integer_value_fits_type(assigned, stable.value_type, target) {
            return None;
        }
        (assigned, Some(stable.value_type))
    };

    if value.is_zero() || value.abs_magnitude() > unsigned_max_for_bits(target.pointer_width) {
        return None;
    }

    if increment.cast_to_induction_type
        && !integer_value_fits_type(value, induction_ty, target)
    {
        // For a casted STEP, the cast is required to preserve the mathematical
        // value, not merely to compile.
        return None;
    }

    if let Some(source_type) = source_type {
        if value.is_negative()
            && !integer_value_fits_type(
                IntValue::NonNegative(value.abs_magnitude()),
                source_type,
                target,
            )
        {
            // The generated STEP' is -STEP. Reject the minimum signed value,
            // for example, because negating it would overflow W.
            return None;
        }
    }

    Some(StepInfo {
        operation: increment.operation,
        source_expr: increment.step_expr,
        source_type,
        value,
    })
}

fn direction_matches_step(direction: Direction, operation: StepOperation, value: IntValue) -> bool {
    let positive = value.is_positive();
    let negative = value.is_negative();
    match (direction, operation) {
        (Direction::AscendingExclusive | Direction::AscendingInclusive, StepOperation::Add) => {
            positive
        }
        (Direction::AscendingExclusive | Direction::AscendingInclusive, StepOperation::Sub) => {
            negative
        }
        (Direction::DescendingStrict | Direction::DescendingInclusive, StepOperation::Add) => {
            negative
        }
        (Direction::DescendingStrict | Direction::DescendingInclusive, StepOperation::Sub) => {
            positive
        }
    }
}

fn resolves_to_local(expr: &Expr, expected: HirId, cx: &RefactorCtxt) -> bool {
    matches!(expr.kind, ExprKind::Path(..))
        && matches!(cx.try_resolve_expr_hir(expr), Some(Res::Local(id)) if id == expected)
}

// -----------------------------------------------------------------------------
// Direct-statement literal history
// -----------------------------------------------------------------------------

fn collect_preceding_statements(krate: &Crate) -> HashMap<rustc_ast::NodeId, Vec<Stmt>> {
    let mut result = HashMap::new();
    visit_nodes(krate, |block: &Block| {
        for (index, stmt) in block.stmts.iter().enumerate() {
            if let Some(expr) = stmt_expr(stmt) {
                if matches!(expr.kind, ExprKind::While(..)) {
                    result.insert(expr.id, block.stmts[..index].to_vec());
                }
            }
        }
    });
    result
}

fn collect_following_statements(krate: &Crate) -> HashMap<rustc_ast::NodeId, Vec<Stmt>> {
    let mut result = HashMap::new();
    visit_nodes(krate, |block: &Block| {
        for (index, stmt) in block.stmts.iter().enumerate() {
            if let Some(expr) = stmt_expr(stmt) {
                if matches!(expr.kind, ExprKind::While(..)) {
                    result.insert(expr.id, block.stmts[index + 1..].to_vec());
                }
            }
        }
    });
    result
}

fn mapped_expr_hir_ids_in_statements(
    statements: &[Stmt],
    cx: &RefactorCtxt,
) -> Vec<HirId> {
    struct ExprIdCollector {
        ids: Vec<rustc_ast::NodeId>,
    }

    impl<'ast> Visitor<'ast> for ExprIdCollector {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            self.ids.push(expr.id);
            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // A nested item cannot capture locals from the surrounding body.
        }
    }

    let mut collector = ExprIdCollector { ids: Vec::new() };
    for stmt in statements {
        collector.visit_stmt(stmt);
    }

    collector
        .ids
        .into_iter()
        .filter_map(|id| cx.hir_map().opt_node_to_hir_id(id))
        .collect()
}

fn statements_access_local(
    statements: &[Stmt],
    target: HirId,
    cx: &RefactorCtxt,
) -> bool {
    struct Finder<'cx, 'a, 'tcx: 'a> {
        target: HirId,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        found: bool,
    }

    impl<'cx, 'a, 'tcx: 'a, 'ast> Visitor<'ast> for Finder<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }

            if matches!(expr.kind, ExprKind::Path(..))
                && matches!(
                    self.cx.try_resolve_expr_hir(expr),
                    Some(Res::Local(id)) if id == self.target
                )
            {
                self.found = true;
                return;
            }

            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // A nested item cannot capture this local binding.
        }
    }

    let mut finder = Finder {
        target,
        cx,
        found: false,
    };
    for stmt in statements {
        finder.visit_stmt(stmt);
        if finder.found {
            return true;
        }
    }
    false
}

fn last_direct_literal_assignment(
    place: &PlaceKey,
    preceding: &[Stmt],
    cx: &RefactorCtxt,
    target: TargetInfo,
) -> Option<IntValue> {
    for stmt in preceding.iter().rev() {
        if let Some(value) = direct_literal_write(stmt, place, cx, target) {
            return Some(value);
        }
        if statement_modifies_place(stmt, place, cx, target) {
            // The latest modification is not the required direct literal
            // initialization/assignment, or another modification occurs after
            // such an assignment. Either way, constraint 4/5 is not proven.
            return None;
        }
    }
    None
}

fn direct_literal_write(stmt: &Stmt, place: &PlaceKey, cx: &RefactorCtxt, target: TargetInfo) -> Option<IntValue> {
    match &stmt.kind {
        StmtKind::Local(local) if place.projections.is_empty() => {
            let base = match place.base {
                BaseKey::Local(id) => id,
                _ => return None,
            };
            let (local_id, _) = simple_binding_hir_id(&local.pat, cx)?;
            if local_id != base {
                return None;
            }
            normalized_unsuffixed_integer_literal_value(local_initializer(local)?, target)
        }
        StmtKind::Item(item) if place.projections.is_empty() => {
            let base = match place.base {
                BaseKey::Item(id) => id,
                _ => return None,
            };
            if cx.node_def_id(item.id) != base {
                return None;
            }
            let init = match &item.kind {
                ItemKind::Static(_, _, init) => init.as_ref()?,
                ItemKind::Const(_, _, init) => init.as_ref()?,
                _ => return None,
            };
            normalized_unsuffixed_integer_literal_value(init, target)
        }
        StmtKind::Expr(expr) | StmtKind::Semi(expr) => {
            let (lhs, rhs) = match &expr.kind {
                ExprKind::Assign(lhs, rhs, _) => (lhs, rhs),
                _ => return None,
            };
            if place_key(lhs, cx, target).as_ref() != Some(place) {
                return None;
            }
            normalized_unsuffixed_integer_literal_value(rhs, target)
        }
        _ => None,
    }
}

fn local_initializer(local: &Local) -> Option<&P<Expr>> {
    match &local.kind {
        LocalKind::Init(init) | LocalKind::InitElse(init, _) => Some(init),
        LocalKind::Decl => None,
    }
}

fn statement_modifies_place(stmt: &Stmt, target: &PlaceKey, cx: &RefactorCtxt, target_info: TargetInfo) -> bool {
    struct Finder<'cx, 'a, 'tcx: 'a> {
        target: &'cx PlaceKey,
        cx: &'cx RefactorCtxt<'a, 'tcx>,
        found: bool,
        target_info: TargetInfo,
    }

    impl<'cx, 'a, 'tcx: 'a, 'ast> Visitor<'ast> for Finder<'cx, 'a, 'tcx> {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }
            match &expr.kind {
                ExprKind::Assign(lhs, _, _) | ExprKind::AssignOp(_, lhs, _) => {
                    if assignment_may_modify_place(lhs, self.target, self.cx, self.target_info) {
                        self.found = true;
                        return;
                    }
                }
                _ => {}
            }
            visit::walk_expr(self, expr);
        }

        fn visit_local(&mut self, local: &'ast Local) {
            if self.found {
                return;
            }
            if let BaseKey::Local(id) = self.target.base {
                if simple_binding_hir_id(&local.pat, self.cx)
                    .map(|(local_id, _)| local_id == id)
                    .unwrap_or(false)
                {
                    self.found = true;
                    return;
                }
            }
            visit::walk_local(self, local);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items cannot capture a local. Local static/const item
            // initialization is handled explicitly by direct_literal_write.
        }
    }

    let mut finder = Finder {
        target,
        cx,
        found: false,
        target_info,
    };
    finder.visit_stmt(stmt);
    finder.found
}

fn assignment_may_modify_place(lhs: &Expr, target: &PlaceKey, cx: &RefactorCtxt, target_info: TargetInfo) -> bool {
    if root_base(lhs, cx) != Some(target.base) {
        return false;
    }

    let written = match place_key(lhs, cx, target_info) {
        Some(place) => place,
        // A write through the same base whose projection cannot be resolved
        // precisely might alias the target place.
        None => return true,
    };

    projections_are_prefix(&written.projections, &target.projections)
        || projections_are_prefix(&target.projections, &written.projections)
}

fn projections_are_prefix(prefix: &[PlaceProjection], whole: &[PlaceProjection]) -> bool {
    prefix.len() <= whole.len() && prefix.iter().zip(whole).all(|(left, right)| left == right)
}

// -----------------------------------------------------------------------------
// rustc use/borrow analysis
// -----------------------------------------------------------------------------

fn hir_node_is_descendant_of(tcx: TyCtxt<'_>, id: HirId, ancestor: HirId) -> bool {
    let hir_map = tcx.hir();
    let mut current = id;
    loop {
        if current == ancestor {
            return true;
        }
        let parent = hir_map.get_parent_node(current);
        if parent == current {
            return false;
        }
        current = parent;
    }
}

#[derive(Default)]
struct StabilityFacts {
    captured_by_reference_before_or_during: bool,
    modified_during_loop: bool,
}

struct StabilityDelegate<'tcx> {
    tcx: TyCtxt<'tcx>,
    tracked: HirId,
    loop_body_hir_ids: Vec<HirId>,
    facts: StabilityFacts,
}

impl<'tcx> StabilityDelegate<'tcx> {
    fn tracked(&self, place: &PlaceWithHirId<'tcx>) -> bool {
        matches!(place.place.base, PlaceBase::Local(id) if id == self.tracked)
    }

    fn inside_loop_body(&self, id: HirId) -> bool {
        self.loop_body_hir_ids
            .iter()
            .copied()
            .any(|root| hir_node_is_descendant_of(self.tcx, id, root))
    }
}

impl<'tcx> Delegate<'tcx> for StabilityDelegate<'tcx> {
    fn consume(&mut self, _place: &PlaceWithHirId<'tcx>, _diag_expr_id: HirId) {}

    fn borrow(
        &mut self,
        place: &PlaceWithHirId<'tcx>,
        _diag_expr_id: HirId,
        borrow_kind: ty::BorrowKind,
    ) {
        if !self.tracked(place) {
            return;
        }
        if self.inside_loop_body(place.hir_id) && borrow_kind == ty::BorrowKind::MutBorrow {
            self.facts.modified_during_loop = true;
        }
    }

    fn mutate(&mut self, place: &PlaceWithHirId<'tcx>, _diag_expr_id: HirId) {
        if self.tracked(place) && self.inside_loop_body(place.hir_id) {
            self.facts.modified_during_loop = true;
        }
    }

    fn fake_read(
        &mut self,
        _place: &PlaceWithHirId<'tcx>,
        _cause: FakeReadCause,
        _diag_expr_id: HirId,
    ) {
    }
}

fn local_stability_facts(
    tracked: HirId,
    while_span: Span,
    loop_body_hir_ids: &[HirId],
    cx: &RefactorCtxt,
) -> Option<StabilityFacts> {
    let tcx = cx.ty_ctxt();
    let hir_map = cx.hir_map();
    let parent = hir_map.get_parent_item(tracked);
    let body_id = hir_map.maybe_body_owned_by(parent)?;
    let body = hir_map.body(body_id);
    let tables = tcx.typeck_body(body_id);
    let infcx = tcx.infer_ctxt().build();

    // `closure_min_captures` records the actual capture mode inferred by
    // rustc. This is the relevant distinction for the stable-value rule:
    // ByRef creates a reference to the tracked local, whereas ByValue does not.
    // Use the capture-kind span for the temporal "before or during" boundary.
    let captured_by_reference_before_or_during = tables
        .closure_min_captures
        .values()
        .filter_map(|captures| captures.get(&tracked))
        .flatten()
        .any(|capture| {
            matches!(capture.info.capture_kind, ty::UpvarCapture::ByRef(_))
                && capture.get_capture_kind_span(tcx).lo() < while_span.hi()
        });

    let mut delegate = StabilityDelegate {
        tcx,
        tracked,
        loop_body_hir_ids: loop_body_hir_ids.to_vec(),
        facts: StabilityFacts {
            captured_by_reference_before_or_during,
            modified_during_loop: false,
        },
    };
    ExprUseVisitor::new(&mut delegate, &infcx, parent, ParamEnv::empty(), tables)
        .consume_body(body);
    Some(delegate.facts)
}

#[derive(Default)]
struct InductionFacts {
    writes_inside_loop: usize,
    illegal_borrow_inside: bool,
}

struct InductionDelegate<'tcx> {
    tcx: TyCtxt<'tcx>,
    tracked: HirId,
    // HIR expression roots belonging to the complete original while body,
    // including the final increment.  Using these roots avoids depending on
    // whether the while statement itself has an AST -> HIR mapping.
    loop_body_hir_ids: Vec<HirId>,
    // HIR expression roots belonging to the original while body, excluding
    // the final increment statement. Constraint 7 forbids reference/raw
    // pointer uses of I only in this region.
    checked_body_hir_ids: Vec<HirId>,
    facts: InductionFacts,
}

impl<'tcx> InductionDelegate<'tcx> {
    fn tracked(&self, place: &PlaceWithHirId<'tcx>) -> bool {
        matches!(place.place.base, PlaceBase::Local(id) if id == self.tracked)
    }

    fn inside_loop_body(&self, id: HirId) -> bool {
        self.loop_body_hir_ids
            .iter()
            .copied()
            .any(|root| hir_node_is_descendant_of(self.tcx, id, root))
    }

    fn inside_checked_body(&self, id: HirId) -> bool {
        self.checked_body_hir_ids
            .iter()
            .copied()
            .any(|root| hir_node_is_descendant_of(self.tcx, id, root))
    }
}

impl<'tcx> Delegate<'tcx> for InductionDelegate<'tcx> {
    fn consume(&mut self, _place: &PlaceWithHirId<'tcx>, _diag_expr_id: HirId) {
        // Plain reads of I are permitted in the condition and in the loop body.
        // Constraint 8 (no access after the while) is checked separately at
        // the AST level below.
    }

    fn borrow(
        &mut self,
        place: &PlaceWithHirId<'tcx>,
        _diag_expr_id: HirId,
        borrow_kind: ty::BorrowKind,
    ) {
        if !self.tracked(place) || !self.inside_loop_body(place.hir_id) {
            return;
        }

        // ExprUseVisitor can represent the permitted update as a mutable
        // borrow. Count that as a write over the whole body. Shared/immutable
        // borrows are forbidden only in the body before the increment.
        if borrow_kind == ty::BorrowKind::MutBorrow {
            self.facts.writes_inside_loop += 1;
        } else if self.inside_checked_body(place.hir_id) {
            self.facts.illegal_borrow_inside = true;
        }
    }

    fn mutate(&mut self, place: &PlaceWithHirId<'tcx>, _diag_expr_id: HirId) {
        if self.tracked(place) && self.inside_loop_body(place.hir_id) {
            self.facts.writes_inside_loop += 1;
        }
    }

    fn fake_read(
        &mut self,
        _place: &PlaceWithHirId<'tcx>,
        _cause: FakeReadCause,
        _diag_expr_id: HirId,
    ) {
    }
}

fn induction_variable_usage_is_safe(
    tracked: HirId,
    loop_body_hir_ids: &[HirId],
    checked_body: &[Stmt],
    following: &[Stmt],
    cx: &RefactorCtxt,
) -> bool {
    // Constraint 8: after the while, the old induction binding must never be
    // accessed again. Resolve paths to the exact HirId so a later shadowing
    // binding with the same source name does not cause a false rejection.
    if statements_access_local(following, tracked, cx) {
        return false;
    }

    let tcx = cx.ty_ctxt();
    let hir_map = cx.hir_map();
    let parent = hir_map.get_parent_item(tracked);
    let body_id = match hir_map.maybe_body_owned_by(parent) {
        Some(body_id) => body_id,
        None => return false,
    };
    let body = hir_map.body(body_id);
    let tables = tcx.typeck_body(body_id);
    let infcx = tcx.infer_ctxt().build();

    let checked_body_hir_ids = mapped_expr_hir_ids_in_statements(checked_body, cx);

    let mut delegate = InductionDelegate {
        tcx,
        tracked,
        loop_body_hir_ids: loop_body_hir_ids.to_vec(),
        checked_body_hir_ids,
        facts: InductionFacts::default(),
    };
    ExprUseVisitor::new(&mut delegate, &infcx, parent, ParamEnv::empty(), tables)
        .consume_body(body);

    // The matcher/classify_increment step has already established that the
    // final statement of the while body is one of the permitted updates of I.
    // Therefore exactly one write in the whole body means that this update is
    // the only write.
    delegate.facts.writes_inside_loop == 1 && !delegate.facts.illegal_borrow_inside
}

// -----------------------------------------------------------------------------
// continue analysis
// -----------------------------------------------------------------------------

fn body_has_continue_to_loop(body: &Block, target_label: Option<&Label>) -> bool {
    struct ContinueVisitor {
        target_label: Option<Symbol>,
        nested_loop_depth: usize,
        shadowing_label_depth: usize,
        found: bool,
    }

    impl<'ast> Visitor<'ast> for ContinueVisitor {
        fn visit_expr(&mut self, expr: &'ast Expr) {
            if self.found {
                return;
            }

            if let ExprKind::Continue(label) = &expr.kind {
                match label {
                    None if self.nested_loop_depth == 0 => self.found = true,
                    Some(label)
                        if self.target_label == Some(label.ident.name)
                            && self.shadowing_label_depth == 0 =>
                    {
                        self.found = true
                    }
                    _ => {}
                }
                return;
            }

            if matches!(expr.kind, ExprKind::Closure(..)) {
                // A continue inside a closure cannot target a loop outside that
                // closure. Do not descend into the nested body.
                return;
            }

            let nested_loop_label = match &expr.kind {
                ExprKind::While(_, _, label)
                | ExprKind::ForLoop(_, _, _, label)
                | ExprKind::Loop(_, label) => {
                    Some(label.as_ref().map(|label| label.ident.name))
                }
                _ => None,
            };

            if let Some(nested_label) = nested_loop_label {
                self.nested_loop_depth += 1;
                let shadows = self.target_label.is_some() && nested_label == self.target_label;
                if shadows {
                    self.shadowing_label_depth += 1;
                }
                visit::walk_expr(self, expr);
                if shadows {
                    self.shadowing_label_depth -= 1;
                }
                self.nested_loop_depth -= 1;
                return;
            }

            visit::walk_expr(self, expr);
        }

        fn visit_item(&mut self, _item: &'ast Item) {
            // Nested items cannot continue an enclosing loop.
        }
    }

    let mut visitor = ContinueVisitor {
        target_label: target_label.map(|label| label.ident.name),
        nested_loop_depth: 0,
        shadowing_label_depth: 0,
        found: false,
    };
    visitor.visit_block(body);
    visitor.found
}

// -----------------------------------------------------------------------------
// Integer literal arithmetic and ranges
// -----------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
enum IntValue {
    NonNegative(u128),
    Negative(u128),
}

impl IntValue {
    fn is_zero(self) -> bool {
        matches!(self, IntValue::NonNegative(0))
    }

    fn is_positive(self) -> bool {
        matches!(self, IntValue::NonNegative(value) if value > 0)
    }

    fn is_negative(self) -> bool {
        matches!(self, IntValue::Negative(value) if value > 0)
    }

    fn abs_magnitude(self) -> u128 {
        match self {
            IntValue::NonNegative(value) | IntValue::Negative(value) => value,
        }
    }

    fn decimal_string(self) -> String {
        match self {
            IntValue::NonNegative(value) => value.to_string(),
            IntValue::Negative(value) => format!("-{}", value),
        }
    }
}

fn integer_literal_expression_value(expr: &Expr) -> Option<IntValue> {
    match &expr.kind {
        ExprKind::Lit(lit) => match lit.kind {
            LitKind::Int(value, _) => Some(IntValue::NonNegative(value)),
            _ => None,
        },
        ExprKind::Unary(UnOp::Neg, inner) => match &inner.kind {
            ExprKind::Lit(lit) => match lit.kind {
                LitKind::Int(value, _) => Some(IntValue::Negative(value)),
                _ => None,
            },
            _ => None,
        },
        _ => None,
    }
}

fn unsuffixed_integer_literal_value(expr: &Expr) -> Option<IntValue> {
    match &expr.kind {
        ExprKind::Lit(lit) => match lit.kind {
            LitKind::Int(value, LitIntType::Unsuffixed) if lit.token_lit.suffix.is_none() => {
                Some(IntValue::NonNegative(value))
            }
            _ => None,
        },
        ExprKind::Unary(UnOp::Neg, inner) => match &inner.kind {
            ExprKind::Lit(lit) => match lit.kind {
                LitKind::Int(value, LitIntType::Unsuffixed) if lit.token_lit.suffix.is_none() => {
                    Some(IntValue::Negative(value))
                }
                _ => None,
            },
            _ => None,
        },
        _ => None,
    }
}

fn normalized_integer_literal_expression_value(
    expr: &Expr,
    target: TargetInfo,
) -> Option<IntValue> {
    if let Some(value) = integer_literal_expression_value(expr) {
        return Some(value);
    }

    // C2Rust must cast array indices to usize and may leave intermediate
    // integer casts in place.  Treat such a cast chain as the original literal
    // expression only when every cast preserves its mathematical value.  This
    // keeps the projection key exact (e.g. it will not confuse
    // `300 as u8 as usize` with the literal index 300).
    if let ExprKind::Cast(inner, cast_ty) = &expr.kind {
        let value = normalized_integer_literal_expression_value(inner, target)?;
        let cast_ty = integer_type_from_ty(cast_ty)?;
        if integer_value_fits_type(value, cast_ty, target) {
            return Some(value);
        }
    }

    None
}

fn normalized_unsuffixed_integer_literal_value(
    expr: &Expr,
    target: TargetInfo,
) -> Option<IntValue> {
    if let Some(value) = unsuffixed_integer_literal_value(expr) {
        return Some(value);
    }

    // The C source contains a direct unsuffixed literal assignment, but C2Rust
    // can materialize it as a chain such as `-1 as i32 as i16`.  Accept only
    // value-preserving integer casts around an innermost unsuffixed literal.
    // This does not accept arbitrary constant expressions or lossy casts.
    if let ExprKind::Cast(inner, cast_ty) = &expr.kind {
        let value = normalized_unsuffixed_integer_literal_value(inner, target)?;
        let cast_ty = integer_type_from_ty(cast_ty)?;
        if integer_value_fits_type(value, cast_ty, target) {
            return Some(value);
        }
    }

    // Be robust to the alternative AST parenthesization `-(1 as T)` while
    // still requiring the only non-cast leaf to be an unsuffixed literal.
    if let ExprKind::Unary(UnOp::Neg, inner) = &expr.kind {
        if let IntValue::NonNegative(magnitude) =
            normalized_unsuffixed_integer_literal_value(inner, target)?
        {
            return Some(IntValue::Negative(magnitude));
        }
    }

    None
}

fn bound_literal_value(expr: &Expr) -> Option<IntValue> {
    if let Some(value) = unsuffixed_integer_literal_value(expr) {
        return Some(value);
    }
    match &expr.kind {
        ExprKind::Paren(inner) => unsuffixed_integer_literal_value(inner),
        _ => None,
    }
}

fn normalize_bound_for_ascending(expr: &P<Expr>) -> P<Expr> {
    match &expr.kind {
        ExprKind::Paren(inner) if unsuffixed_integer_literal_value(inner).is_some() => inner.clone(),
        _ => expr.clone(),
    }
}

fn strip_parentheses(mut expr: &Expr) -> &Expr {
    while let ExprKind::Paren(inner) = &expr.kind {
        expr = inner;
    }
    expr
}

fn add_one(value: IntValue) -> Option<IntValue> {
    match value {
        IntValue::NonNegative(value) => value.checked_add(1).map(IntValue::NonNegative),
        IntValue::Negative(1) => Some(IntValue::NonNegative(0)),
        IntValue::Negative(value) if value > 1 => Some(IntValue::Negative(value - 1)),
        IntValue::Negative(_) => None,
    }
}

fn integer_type_width_and_signedness(ty: IntegerType, target: TargetInfo) -> (u32, bool) {
    match ty {
        IntegerType::Primitive(name) => match name {
            "i8" => (8, true),
            "i16" => (16, true),
            "i32" => (32, true),
            "i64" => (64, true),
            "i128" => (128, true),
            "isize" => (target.pointer_width, true),
            "u8" => (8, false),
            "u16" => (16, false),
            "u32" => (32, false),
            "u64" => (64, false),
            "u128" => (128, false),
            "usize" => (target.pointer_width, false),
            _ => unreachable!("unsupported primitive integer type {}", name),
        },
        IntegerType::C(c_type) => {
            let signed = matches!(
                c_type,
                CIntegerType::SChar
                    | CIntegerType::Short
                    | CIntegerType::Int
                    | CIntegerType::Long
                    | CIntegerType::LongLong
            );
            let width = match c_type {
                CIntegerType::SChar | CIntegerType::UChar => target.c_char_width,
                CIntegerType::Short | CIntegerType::UShort => target.c_short_width,
                CIntegerType::Int | CIntegerType::UInt => target.c_int_width,
                CIntegerType::Long | CIntegerType::ULong => target.c_long_width,
                CIntegerType::LongLong | CIntegerType::ULongLong => target.c_longlong_width,
            };
            (width, signed)
        }
    }
}

fn integer_value_fits_type(value: IntValue, ty: IntegerType, target: TargetInfo) -> bool {
    let (bits, signed) = integer_type_width_and_signedness(ty, target);
    match value {
        IntValue::NonNegative(value) => {
            if signed {
                value <= signed_max_for_bits(bits)
            } else {
                value <= unsigned_max_for_bits(bits)
            }
        }
        IntValue::Negative(magnitude) => signed && magnitude <= signed_min_magnitude_for_bits(bits),
    }
}

fn cast_integer_value(value: IntValue, ty: IntegerType, target: TargetInfo) -> IntValue {
    let (bits, signed) = integer_type_width_and_signedness(ty, target);
    let raw = match value {
        IntValue::NonNegative(value) => truncate_to_bits(value, bits),
        IntValue::Negative(magnitude) => truncate_to_bits(0u128.wrapping_sub(magnitude), bits),
    };

    if !signed {
        return IntValue::NonNegative(raw);
    }

    let sign_bit = 1u128 << (bits - 1);
    if raw & sign_bit == 0 {
        IntValue::NonNegative(raw)
    } else {
        let magnitude = if bits == 128 {
            (!raw).wrapping_add(1)
        } else {
            (1u128 << bits) - raw
        };
        IntValue::Negative(magnitude)
    }
}

fn truncate_to_bits(value: u128, bits: u32) -> u128 {
    if bits == 128 {
        value
    } else {
        value & ((1u128 << bits) - 1)
    }
}

fn signed_max_for_bits(bits: u32) -> u128 {
    match bits {
        8 => i8::MAX as u128,
        16 => i16::MAX as u128,
        32 => i32::MAX as u128,
        64 => i64::MAX as u128,
        128 => i128::MAX as u128,
        _ => (1u128 << (bits - 1)) - 1,
    }
}

fn signed_min_magnitude_for_bits(bits: u32) -> u128 {
    1u128 << (bits - 1)
}

fn unsigned_max_for_bits(bits: u32) -> u128 {
    if bits == 128 {
        u128::MAX
    } else {
        (1u128 << bits) - 1
    }
}

fn typed_literal_expr(
    st: &CommandState,
    cx: &RefactorCtxt,
    value: IntValue,
    ty: IntegerType,
) -> P<Expr> {
    match ty {
        IntegerType::Primitive(name) => {
            st.parse_expr(cx, &format!("{}{}", value.decimal_string(), name))
        }
        IntegerType::C(c_type) => st.parse_expr(
            cx,
            &format!(
                "({} as {})",
                value.decimal_string(),
                c_integer_type_path(c_type)
            ),
        ),
    }
}

fn c_integer_type_path(ty: CIntegerType) -> &'static str {
    match ty {
        CIntegerType::SChar => "::core::ffi::c_schar",
        CIntegerType::UChar => "::core::ffi::c_uchar",
        CIntegerType::Short => "::core::ffi::c_short",
        CIntegerType::UShort => "::core::ffi::c_ushort",
        CIntegerType::Int => "::core::ffi::c_int",
        CIntegerType::UInt => "::core::ffi::c_uint",
        CIntegerType::Long => "::core::ffi::c_long",
        CIntegerType::ULong => "::core::ffi::c_ulong",
        CIntegerType::LongLong => "::core::ffi::c_longlong",
        CIntegerType::ULongLong => "::core::ffi::c_ulonglong",
    }
}

fn make_add_one_expr(st: &CommandState, cx: &RefactorCtxt, expr: P<Expr>) -> P<Expr> {
    let mut mcx = MatchCtxt::new(st, cx);
    mcx.bindings.add("$x", expr);
    let template = mcx.parse_expr("$x:Expr + 1");
    template.subst(st, cx, &mcx.bindings)
}

fn make_step_by_argument(st: &CommandState, cx: &RefactorCtxt, step: &StepInfo) -> P<Expr> {
    if step.source_type.is_none() {
        return st.parse_expr(cx, &step.value.abs_magnitude().to_string());
    }

    let mut value = step.source_expr.clone();
    if step.value.is_negative() {
        let mut mcx = MatchCtxt::new(st, cx);
        mcx.bindings.add("$x", value);
        let template = mcx.parse_expr("-$x:Expr");
        value = template.subst(st, cx, &mcx.bindings);
    }

    if step.source_type != Some(IntegerType::Primitive("usize")) {
        let mut mcx = MatchCtxt::new(st, cx);
        mcx.bindings.add("$x", value);
        let template = mcx.parse_expr("($x:Expr) as usize");
        value = template.subst(st, cx, &mcx.bindings);
    }

    value
}

fn make_range_expr(
    st: &CommandState,
    cx: &RefactorCtxt,
    start: P<Expr>,
    end: P<Expr>,
    inclusive: bool,
    reversed: bool,
    step: Option<P<Expr>>,
) -> P<Expr> {
    let mut mcx = MatchCtxt::new(st, cx);
    mcx.bindings.add("$start_out", start);
    mcx.bindings.add("$end_out", end);
    let has_step = step.is_some();
    if let Some(step) = step {
        mcx.bindings.add("$step_out", step);
    }

    let source = match (inclusive, reversed, has_step) {
        (false, false, false) => "$start_out:Expr .. $end_out:Expr",
        (false, false, true) => "($start_out:Expr .. $end_out:Expr).step_by($step_out:Expr)",
        (true, false, false) => "$start_out:Expr ..= $end_out:Expr",
        (true, false, true) => "($start_out:Expr ..= $end_out:Expr).step_by($step_out:Expr)",
        (true, true, false) => "($start_out:Expr ..= $end_out:Expr).rev()",
        (true, true, true) => {
            "($start_out:Expr ..= $end_out:Expr).rev().step_by($step_out:Expr)"
        }
        (false, true, _) => unreachable!("reversed base-case ranges are always inclusive"),
    };

    let template = mcx.parse_expr(source);
    template.subst(st, cx, &mcx.bindings)
}


fn stmt_expr(stmt: &Stmt) -> Option<&P<Expr>> {
    match &stmt.kind {
        StmtKind::Expr(expr) | StmtKind::Semi(expr) => Some(expr),
        _ => None,
    }
}
