# c2rust_idiomatizer

A plugin for [`c2rust-refactor`](https://github.com/immunant/c2rust).

## Installation

On Debian:

```bash
export C2RUST_IDIOMATIZER_DIR="$HOME/projects/c2rust_idiomatizer"

sudo apt update
sudo apt install -y bear build-essential clang cmake curl git libclang-dev libssl-dev llvm llvm-dev pkg-config

curl https://sh.rustup.rs -sSf | sh
source "$HOME/.cargo/env"

curl https://astral.sh/uv/install.sh -LsSf | sh
source "$HOME/.local/bin/env"

git clone https://github.com/VincentSchaerl/c2rust_idiomatizer "$C2RUST_IDIOMATIZER_DIR"
cd "$C2RUST_IDIOMATIZER_DIR"
git submodule update --init
cargo build --release

cargo build \
    --manifest-path "$C2RUST_IDIOMATIZER_DIR/lib/c2rust/Cargo.toml" \
    --release \
    -p c2rust \
    --bin c2rust-transpile
cargo build \
    --manifest-path "$C2RUST_IDIOMATIZER_DIR/lib/c2rust/Cargo.toml" \
    --release \
    -p c2rust-refactor \
    --bin c2rust-refactor

test/run.sh
```

## Usage

`c2rust_idiomatizer` provides three commands:
- `replace_integer_types`
- `remove_redundant_integer_casts`
- `idiomatize_loops`

### replace_integer_types

Replaces C FFI integer types, e.g. `::core::ffi::c_int`, with the corresponding Rust integer types, e.g. `i32`.

Options:

```text
char-width          Specifies the bit width of the C char type.
short-width         Specifies the bit width of the C short type.
int-width           Specifies the bit width of the C int type.
long-width          Specifies the bit width of the C long type.
longlong-width      Specifies the bit width of the C long long type.
```

These options are useful for cross-compilation. Otherwise, I recommend omitting them, as the bit widths are determined automatically based on the compilation target, i.e. usually the host platform.

### remove_redundant_integer_casts

Removes redundant integer casts, e.g. `let x: i32 = 0 as i32;`.

Options:

```text
char-width          Specifies the bit width of the C char type.
short-width         Specifies the bit width of the C short type.
int-width           Specifies the bit width of the C int type.
long-width          Specifies the bit width of the C long type.
longlong-width      Specifies the bit width of the C long long type.
```

These options are useful for cross-compilation. Otherwise, I recommend omitting them, as the bit widths are determined automatically based on the compilation target, i.e. usually the host platform.

### idiomatize_loops

Transforms range-based `while` loops (which were likely `for` loops in the original C code) into `for` loops and then makes them more idiomatic.

It does this in multiple steps:
1. `Initial transformation`:  
   Transforms `while` loops that iterate over a range into `for` loops.
2. `iterate-arrays-idiomatically`:  
   Detects `for` loops that iterate over an array and replaces their upper bound with the array's length, e.g. `for i in 0i32..10` → `for i in 0..array.len() as i32`.
3. `pass-arrays-as-slices`:  
   Detects functions that take a raw pointer to an array and its length as separate parameters and contain a `for` loop that iterates over the array using the provided length as its upper bound. These functions are transformed to accept a slice reference instead of the pointer and length parameters, and the loop's upper bound is changed to `slice.len()`.
4. `index-type-to-usize`:  
   Detects `for` loops that iterate over an array or slice using `array.len()` or `slice.len()` as their upper bound and changes the index type to `usize`, e.g. `for i in 0i32..array.len() as i32` → `for i in 0..array.len()`.
5. `index-based-to-iterator-based`:  
   Detects `for` loops that iterate over an array or slice using `array.len()` or `slice.len()` as their upper bound and an index of type `usize`, and changes them from index-based to iterator-based loops, e.g. `for i in 0..array.len()` → `for element in array.iter()`.

Options:

```text
iterate-arrays-idiomatically        Enables or disables this step.
pass-arrays-as-slices               Enables or disables this step.
transform-public-functions          Enables or disables pass-arrays-as-slices for public functions.
index-type-to-usize                 Enables or disables this step.
index-based-to-iterator-based       Enables or disables this step.
```

Note that:
- `idiomatize_loops` only works on code previously refactored with `remove_redundant_integer_casts`.
- The steps `iterate-arrays-idiomatically` and `pass-arrays-as-slices` only work on `for` loops previously transformed by the first step `Initial transformation`, as `c2rust-transpile` translates all `for` loops to `while` loops.
- The step `index-type-to-usize` only works on `for` loops previously transformed by either the step `iterate-arrays-idiomatically` or the step `pass-arrays-as-slices`.
- The step `index-based-to-iterator-based` only works on `for` loops previously transformed by the step `index-type-to-usize`.
- Because the step `pass-arrays-as-slices` changes function signatures, `transform-public-functions=false` should be used for libraries.
- Because the step `pass-arrays-as-slices` needs to examine all function calls to ensure correct transformations, all relevant source code must be contained in a single crate, as `c2rust-refactor` only processes one crate at a time. Therefore, when transpiling a C binary to Rust with `c2rust-transpile`, use `--binary <BINARY> --thin-binaries`. For libraries, only use `--emit-build-files` and subsequently set `transform-public-functions=false`.

### Example Usage

I recommend invoking `c2rust-refactor` separately for each `c2rust_idiomatizer` command rather than combining multiple commands separated by `\;` in a single invocation, as the latter tends to cause bugs.

Due to the rewriting behavior of `c2rust-refactor`, the `c2rust_idiomatizer` commands often leave many blank lines and other whitespace characters. Therefore, I recommend running `cargo fmt` after refactoring.

Example usage for binaries:

```bash
export C2RUST_IDIOMATIZER_DIR="$HOME/projects/c2rust_idiomatizer"
export C2RUST_DIR="$C2RUST_IDIOMATIZER_DIR/lib/c2rust/target/release"
export PLUGIN_DIR="$C2RUST_IDIOMATIZER_DIR/target/release"
export PROJECT_DIR="$HOME/projects/PROJECT"
export PROJECT_TRANSPILED_DIR="$HOME/projects/PROJECT_TRANSPILED"
export PROJECT_REFACTORED_DIR="$HOME/projects/PROJECT_REFACTORED"

make -C "$PROJECT_DIR" clean
bear --output "$PROJECT_DIR/compile_commands.json" -- make -C "$PROJECT_DIR"
"$C2RUST_DIR/c2rust-transpile" \
  --binary main \
  --thin-binaries \
  --output-dir "$PROJECT_TRANSPILED_DIR" \
  "$PROJECT_DIR/compile_commands.json"
cargo fmt --manifest-path "$PROJECT_TRANSPILED_DIR/Cargo.toml"

cp -a "$PROJECT_TRANSPILED_DIR/." "$PROJECT_REFACTORED_DIR/"
cd "$PROJECT_REFACTORED_DIR"
"$C2RUST_DIR/c2rust-refactor" \
  --cargo \
  --plugin-dir "$PLUGIN_DIR" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  replace_integer_types
"$C2RUST_DIR/c2rust-refactor" \
  --cargo \
  --plugin-dir "$PLUGIN_DIR" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  remove_redundant_integer_casts
"$C2RUST_DIR/c2rust-refactor" \
  --cargo \
  --plugin-dir "$PLUGIN_DIR" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  idiomatize_loops
cargo fmt --manifest-path "$PROJECT_REFACTORED_DIR/Cargo.toml"
```

Example usage for libraries:

```bash
export C2RUST_IDIOMATIZER_DIR="$HOME/projects/c2rust_idiomatizer"
export C2RUST_DIR="$C2RUST_IDIOMATIZER_DIR/lib/c2rust/target/release"
export PLUGIN_DIR="$C2RUST_IDIOMATIZER_DIR/target/release"
export PROJECT_DIR="$HOME/projects/PROJECT"
export PROJECT_TRANSPILED_DIR="$HOME/projects/PROJECT_TRANSPILED"
export PROJECT_REFACTORED_DIR="$HOME/projects/PROJECT_REFACTORED"

make -C "$PROJECT_DIR" clean
bear --output "$PROJECT_DIR/compile_commands.json" -- make -C "$PROJECT_DIR"
"$C2RUST_DIR/c2rust-transpile" \
  --emit-build-files \
  --output-dir "$PROJECT_TRANSPILED_DIR" \
  "$PROJECT_DIR/compile_commands.json"
cargo fmt --manifest-path "$PROJECT_TRANSPILED_DIR/Cargo.toml"

cp -a "$PROJECT_TRANSPILED_DIR/." "$PROJECT_REFACTORED_DIR/"
cd "$PROJECT_REFACTORED_DIR"
"$C2RUST_DIR/c2rust-refactor" \
  --cargo \
  --plugin-dir "$PLUGIN_DIR" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  replace_integer_types
"$C2RUST_DIR/c2rust-refactor" \
  --cargo \
  --plugin-dir "$PLUGIN_DIR" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  remove_redundant_integer_casts
"$C2RUST_DIR/c2rust-refactor" \
  --cargo \
  --plugin-dir "$PLUGIN_DIR" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  idiomatize_loops transform-public-functions=false
cargo fmt --manifest-path "$PROJECT_REFACTORED_DIR/Cargo.toml"
```

## Command Details

The following sections describe the transformation rules for each command.

Note that the rules are defined at the source code level; the corresponding rules at the AST level can be found in the implementation, i.e. in the source code of `c2rust_idiomatizer`.

### replace_integer_types

`replace_integer_types` performs all of the following integer type replacements:

```text
::core::ffi::c_schar        →     i<bit width of char>
::core::ffi::c_uchar        →     u<bit width of char>
::core::ffi::c_short        →     i<bit width of short>
::core::ffi::c_ushort       →     u<bit width of short>
::core::ffi::c_int          →     i<bit width of int>
::core::ffi::c_uint         →     u<bit width of int>
::core::ffi::c_long         →     i<bit width of long>
::core::ffi::c_ulong        →     u<bit width of long>
::core::ffi::c_longlong     →     i<bit width of longlong>
::core::ffi::c_ulonglong    →     u<bit width of longlong>
```

By default, the bit widths are 8 for `char`, 16 for `short` and 64 for `long long`. The bit widths of `int` and `long` depend on the compilation target defined by `rustc`:

The bit width of `int` is parsed from `cx.session().target.c_int_width`. It will be 32 in most cases.

The bit width of `long` is 32 if `cx.session().target.os` = `windows` or `cx.session().target.env` = `msvc`, else it is equal to `cx.session().target.pointer_width`.

The user can also specify the bit widths of `char`, `short`, `int`, `long` and `long long`, e.g. for cross-compilation, with the options `char-width=<WIDTH>`, `short-width=<WIDTH>`, `int-width=<WIDTH>`, `long-width=<WIDTH>` and `longlong-width=<WIDTH>`, which changes the results of the integer replacements accordingly. The following conditions must hold:

1. `char-width`, `short-width`, `int-width`, `long-width`, `longlong-width` ∈ {8, 16, 32, 64, 128}
2. `char-width` ≥ 8, `short-width` ≥ 16, `int-width` ≥ 16, `long-width` ≥ 32, `longlong-width` ≥ 64
3. `char-width` ≤ `short-width` ≤ `int-width` ≤ `long-width` ≤ `longlong-width`

Furthermore, if both of the following type aliases are present:

```rust
pub type int8_t = __int8_t;
pub type __int8_t = i8;
```

then all occurrences of `int8_t` are replaced with `i8`, and both type aliases are removed.

The same paired type-alias replacement and removal is performed analogously for the following types:

```rust
uint8_t     →     u8
int16_t     →     i16
uint16_t    →     u16
int32_t     →     i32
uint32_t    →     u32
int64_t     →     i64
uint64_t    →     u64
intptr_t    →     isize
uintptr_t   →     usize
size_t      →     usize
```

For `intptr_t`, `uintptr_t`, and `size_t`, `c2rust-transpile` generates only a single type alias. Therefore, if the corresponding alias is present:

```rust
pub type intptr_t = isize;
pub type uintptr_t = usize;
pub type size_t = usize;
```

all occurrences of the alias are replaced with the corresponding Rust type, and the alias itself is removed:

```rust
intptr_t    →     isize
uintptr_t   →     usize
size_t      →     usize
```

### remove_redundant_integer_casts

Every cast

```rust
L as T
```

is removed and replaced with

```rust
L
```

if all of the following conditions are satisfied:

1. `L` is an *unsuffixed integer literal expression*. An *unsuffixed integer literal expression* is an unsuffixed integer literal, optionally preceded by the unary negation operator `-`.
2. `T` is a *primitive integer type*. A *primitive integer type* is a type belonging to the set {`i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `i128`, `u128`, `isize`, `usize`, `::core::ffi::c_schar`, `::core::ffi::c_uchar`, `::core::ffi::c_short`, `::core::ffi::c_ushort`, `::core::ffi::c_int`, `::core::ffi::c_uint`, `::core::ffi::c_long`, `::core::ffi::c_ulong`, `::core::ffi::c_longlong`, `::core::ffi::c_ulonglong`} or a type alias that resolves (possibly through multiple levels of aliasing) to a type of that subset.
3. The value of `L` lies within the value range of `T`. The value range of `isize` and `usize` is determined using `cx.session().target.pointer_width`. The value ranges of the C types are determined via the corresponding Rust types into which `replace_integer_types` would transform them. Here as well, the default values can be overridden using `char-width=<WIDTH>`, `short-width=<WIDTH>`, `int-width=<WIDTH>`, `long-width=<WIDTH>`, and `longlong-width=<WIDTH>`.
4. The value of `L` lies within the value range of the expected type of the entire cast expression. `remove_redundant_integer_casts` covers the following cases:
    1. Cast to `U`: `L as T as U` or `(L as T) as U`.
    2. Initialization of a constant item, static item, or variable, either globally or inside a function: The type of the constant item, static item, or variable is `U`.
    3. Assignment to a static item or variable, or comparison operation with the value of a static item or variable: The type of the static item or variable is `U`.
    4. Assignment to a memory location, or comparison operation with the value stored at a memory location, referenced by a raw pointer: The pointer type is `*const U` or `*mut U`.
    5. Initialization of an array, either globally or inside a function: The element type of the array is `U`.
    6. Assignment to an array element, or comparison operation with the value of an array element: The element type is `U`.
    7. Initialization of a struct, either globally or inside a function: The type of the field is `U`.
    8. Assignment to a struct field, or comparison operation with the value of a struct field: The type of the field is `U`.
    9. Assignment to a field of a struct dereferenced through a raw pointer, or comparison operation with the value of a field of a struct dereferenced through a raw pointer: The type of the field is `U`.
    10. Comparison operation with the value returned by a function: The return type of the function is `U`.
    11. Passing a value to a function: The parameter type is `U`.
    12. Returning a value from a function: The return type of the function is `U`.
    13. Array or slice indexing operation: `L as T` is the index expression of an indexing operation on an array or slice, including an array or slice accessed through a reference. In this case, let `U` be `usize`.

    In every case, `U` is a *primitive integer type* and the value of `L` lies within the value range of `U`.

The transformation is applied repeatedly until a fixed point is reached, i.e. until no further casts can be removed.

### idiomatize_loops

#### Initial transformation

Every `while` loop of the form

```rust
assignment_or_initialization(I, T, L)
while I relational_operator_0 N {
    body
    I decrementing_or_incrementing;
}
```

or

```rust
assignment_or_initialization(I, T, L)
while N relational_operator_1 I {
    body
    I decrementing_or_incrementing;
}
```

with `I` being an arbitrary variable name, is rewritten into a `for` loop of the form

```rust
for I in expression {
    body
}
```

if all of the following conditions are satisfied:

1. For `assignment_or_initialization(I, T, L)` and `I`, one of the following conditions must hold:
    1. `assignment_or_initialization(I, T, L)` = `let mut I: T = L;` and `I` is not accessed after the `while` loop.
    2. `assignment_or_initialization(I, T, L)` = `I = L;` and all of the following conditions hold:
        1. `I` is a variable of type `T` (not a static item) defined inside the function.
        2. *No reference or raw pointer to `I` is created before or during the `while` loop*. *No reference or raw pointer to `x` is created before/during/within `Y`* means that no occurrence of `x` before/during/within `Y` is
            1. the operand of an explicit reference or raw-address expression (`&x`, `&mut x`, `&raw const x`, or `&raw mut x`),
            2. borrowed through a `ref` or `ref mut` binding pattern,
            3. subject to a `rustc` type adjustment that introduces an automatic borrow/autoref, or
            4. captured by reference by a closure, `async` closure, or `async` block.
        3. Either `I` is not accessed after the `while` loop or the first access to `I` after the `while` loop assigns an *unsuffixed integer literal expression* to `I` and this assignment occurs as a direct statement in the same block as the `while` loop and is therefore not inside a conditional or repeated control-flow construct, such as an `if` statement or a loop.
2. `T` is a *primitive integer type*.
3. For `L`, one of the following conditions must hold:
    1. `L` is an *unsuffixed integer literal expression*.
    2. `L` is a *`T`-value*. An *`X`-value* is one of the following:
        1. a constant item, static item or variable of type `X`
        2. a dereferenced raw pointer of type `*mut X` or `*const X`
        3. an array element of type `X`
        4. a struct field of type `X`
        5. a field of type `X` of a struct dereferenced through a raw pointer
        6. a value returned by a function whose return type is `X`
    3. `L` = `_L as T`, where `_L` is a *`U0`-value* and `U0` is a *primitive integer type*.
4. `relational_operator_0` or `relational_operator_1`, respectively, is an element of {`<`, `<=`, `>`, `>=`}.
5. For `N`, one of the following conditions must hold:
    1. `N` is an *unsuffixed integer literal expression* or `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*.
    2. `N` is a *stable `T`-value*. A *stable `X`-value* is one of the following:
        1. a global constant item or global immutable static item or global immutable variable of type `X`
        2. a variable of type `X` passed to the function by value, where the variable is immutable
        3. a variable of type `X` passed to the function by value, where *no reference or raw pointer to it is created before or during the `while` loop*
        4. a constant item of type `X` defined inside the function
        5. a static item or variable of type `X` defined inside the function, where the static item or variable is immutable
        6. a variable of type `X` defined inside the function, where *no reference or raw pointer to it is created before or during the `while` loop*
        7. a *stably indexed array element* of type `X`, where the array is global and is constant or immutable  
        A *stably indexed array element* is an array element denoted by an indexing expression in which every index expression is one of the following:
            1. an *unsuffixed integer literal expression*
            2. a *stable `usize`-value*
            3. an expression of the form `INDEX as usize`, where `INDEX` is a *stable `INDEX_TYPE`-value* and `INDEX_TYPE` is a *primitive integer type*
        8. a *stably indexed array element* of type `X`, where the array is defined inside the function and is constant or immutable
        9. a *stably indexed array element* of type `X`, where the array is defined inside the function, is not static, and *no reference or raw pointer to the array or one of its elements is created before or during the `while` loop*
        10. a struct field of type `X`, where the struct instance is global and is constant or immutable
        11. a struct field of type `X`, where the struct instance is passed to the function by value and is immutable
        12. a struct field of type `X`, where the struct instance is passed to the function by value and *no reference or raw pointer to the struct instance or one of its fields is created before or during the `while` loop*
        13. a struct field of type `X`, where the struct instance is defined inside the function and is constant or immutable
        14. a struct field of type `X`, where the struct instance is defined inside the function, is not static, and *no reference or raw pointer to the struct instance or one of its fields is created before or during the `while` loop*

        In addition, a *stable `X`-value* is not modified, directly or indirectly, during the entire `while` loop. If the *stable `X`-value* is a *stably indexed array element* or a struct field, the containing array or struct instance is also not modified during the entire `while` loop.

    3. `N` = `_N1 as T` or `N` = `(_N1 as T)` or `N` = `_N1 as U1 as T` or `N` = `(_N1 as U1 as T)`, where `_N1` is a *stable `T`-value* and `U1`, if it exists in the expression, is a *primitive integer type*.
    4. `N` = `_N2 as T` or `N` = `(_N2 as T)` or `N` = `_N2 as U2 as T` or `N` = `(_N2 as U2 as T)`, where `_N2` is a *stable `U3`-value*, `U3` is a *primitive integer type*, `U3` ≠ `T`, and `U2`, if it exists in the expression, is a *primitive integer type*.

    For the next condition, *statically determinable `X`-values with value `x`* must be defined: A *statically determinable `X`-value with value `x`* is a *stable `X`-value*, for which one of the following conditions holds:

    1. The *stable `X`-value* is a global constant item, global immutable static item or global immutable variable and is initialized with an *unsuffixed integer literal expression*.  
    In this case, `x` is the value of that *unsuffixed integer literal expression*.
    2. The *stable `X`-value* is a *stably indexed array element* contained in a global constant array or global immutable array or a struct field contained in a global constant struct instance or global immutable struct instance. If the *stable `X`-value* is a *stably indexed array element*, every index expression used to denote that array element and the corresponding initializer of that array element are *unsuffixed integer literal expressions*. If the *stable `X`-value* is a struct field, the corresponding initializer of that struct field is also an *unsuffixed integer literal expression*.  
    In this case, `x` is the value of the *unsuffixed integer literal expression* that is the corresponding initializer.
    3. Within the same function before the `while` loop, there is an initialization or assignment from which the value of the *stable `X`-value* is given by an *unsuffixed integer literal expression*. If the *stable `X`-value* is a *stably indexed array element*, every index expression used to denote that array element is an *unsuffixed integer literal expression*. If it is an initialization and the *stable `X`-value* is a *stably indexed array element* or a struct field, the corresponding initializer of that array element or struct field is an *unsuffixed integer literal expression*. If it is an initialization and the *stable `X`-value* is not a *stably indexed array element* or a struct field, the initialization initializes the *stable `X`-value* also with an *unsuffixed integer literal expression*. If it is an assignment, the assignment also assigns an *unsuffixed integer literal expression* directly to the *stable `X`-value*. Between this initialization or assignment and the `while` loop, the *stable `X`-value* is not modified again, neither directly nor indirectly. The initialization or assignment occurs as a direct statement in the same block as the `while` loop and is therefore not inside a conditional or repeated control-flow construct, such as an `if` statement or a loop.  
    In this case, `x` is the value of the *unsuffixed integer literal expression* that determines the value of the *stable `X`-value* in that initialization or assignment.

    In addition, if `relational_operator_0` = `>` or `relational_operator_1` = `<`, respectively, one of the following conditions must hold:

    1. `N` is an *unsuffixed integer literal expression* or `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*. In this case, let `n` be the value of that *unsuffixed integer literal expression*.
    2. `N` is a *statically determinable `T`-value with value `_n0`*. In this case, let `n` = `_n0`.
    3. `N` = `_N1 as T` or `N` = `(_N1 as T)` or `N` = `_N1 as U1 as T` or `N` = `(_N1 as U1 as T)`, where `_N1` is a *statically determinable `T`-value with value `_n1`* and `U1`, if it exists in the expression, is a *primitive integer type*. In this case, let `n` = `_n1` or, if `U1` exists in the expression, let `n` be the result of the cast `_n1 as U1`.
    4. `N` = `_N2 as T` or `N` = `(_N2 as T)` or `N` = `_N2 as U2 as T` or `N` = `(_N2 as U2 as T)`, where `_N2` is a *statically determinable U3-value with value `_n2`*, `U3` is a *primitive integer type*, `U3` ≠ `T`, and `U2`, if it exists in the expression, is a *primitive integer type*. In this case, let `n` be the result of the cast `_n2 as T` or, if `U2` exists in the expression, the result of the cast `_n2 as U2 as T`.

    In all cases, `n` + 1 must lie within the value range of `T`.

6. `decrementing_or_incrementing` ∈ {`+= STEP`, `-= STEP`, `+= STEP as T`, `-= STEP as T`, `= I + STEP`, `= I - STEP`, `= I + STEP as T`, `= I - STEP as T`, `= STEP + I`, `= STEP as T + I`, `= I.wrapping_add(STEP)`, `= I.wrapping_sub(STEP)`, `= I.wrapping_add(STEP as T)`, `= I.wrapping_sub(STEP as T)`, `= (STEP as T).wrapping_add(I)`}. For `STEP`, one of the following two conditions must hold:
    1. `STEP` is an *unsuffixed integer literal expression* and abs(`STEP`) lies within the value range of `usize`. The value range of `usize` is determined using `cx.session().target.pointer_width`. In this case, let `step` be the value of that *unsuffixed integer literal expression*.
    2. `STEP` is a *statically determinable `U4`-value with value `_step`*, where `U4` is a *primitive integer type* and abs(`_step`) lies within the value range of `usize`. In this case, let `step` = `_step`.

    If `decrementing_or_incrementing` contains `STEP as T`, `step` must lie within the value range of `T`.

    If `STEP` is a *statically determinable `U4`-value*, where `U4` is a *primitive integer type*, and `step` < 0, -`step` must lie within the value range of `U4`.

    In addition, one of the following conditions must be satisfied:

    1. `relational_operator_0` ∈ {`<`, `<=`} or `relational_operator_1` ∈ {`>`, `>=`}, respectively; `decrementing_or_incrementing` ∈ {`+= STEP`, `+= STEP as T`, `= I + STEP`, `= I + STEP as T`, `= STEP + I`, `= STEP as T + I`, `= I.wrapping_add(STEP)`, `= I.wrapping_add(STEP as T)`, `= (STEP as T).wrapping_add(I)`}; and `step` > 0.
    2. `relational_operator_0` ∈ {`<`, `<=`} or `relational_operator_1` ∈ {`>`, `>=`}, respectively; `decrementing_or_incrementing` ∈ {`-= STEP`, `-= STEP as T`, `= I - STEP`, `= I - STEP as T`, `= I.wrapping_sub(STEP)`, `= I.wrapping_sub(STEP as T)`}; and `step` < 0.
    3. `relational_operator_0` ∈ {`>`, `>=`} or `relational_operator_1` ∈ {`<`, `<=`}, respectively; `decrementing_or_incrementing` ∈ {`+= STEP`, `+= STEP as T`, `= I + STEP`, `= I + STEP as T`, `= STEP + I`, `= STEP as T + I`, `= I.wrapping_add(STEP)`, `= I.wrapping_add(STEP as T)`, `= (STEP as T).wrapping_add(I)`}; and `step` < 0.
    4. `relational_operator_0` ∈ {`>`, `>=`} or `relational_operator_1` ∈ {`<`, `<=`}, respectively; `decrementing_or_incrementing` ∈ {`-= STEP`, `-= STEP as T`, `= I - STEP`, `= I - STEP as T`, `= I.wrapping_sub(STEP)`, `= I.wrapping_sub(STEP as T)`}; and `step` > 0.
7. `body` does not contain a `continue` whose target is the `while` loop to be transformed. A `continue` in a nested loop is allowed.
8. Within `body`, `I` is only accessed for reading or is not accessed at all. *No reference or raw pointer to `I` is created within `body`*.

`expression` then has the form

```rust
Lʹ..Nʹ
```

if `relational_operator_0` = `<` or `relational_operator_1` = `>`, respectively, and abs(`step`) = 1,

```rust
(Lʹ..Nʹ).step_by(STEPʹʹ)
```

if `relational_operator_0` = `<` or `relational_operator_1` = `>`, respectively, and abs(`step`) ≠ 1,

```rust
Lʹ..=Nʹ
```

if `relational_operator_0` = `<=` or `relational_operator_1` = `>=`, respectively, and abs(`step`) = 1,

```rust
(Lʹ..=Nʹ).step_by(STEPʹʹ)
```

if `relational_operator_0` = `<=` or `relational_operator_1` = `>=`, respectively, and abs(`step`) ≠ 1,

```rust
(Nʹʹʹ..=L).rev()
```

if `relational_operator_0` = `>` or `relational_operator_1` = `<`, respectively, and abs(`step`) = 1,

```rust
(Nʹʹʹ..=L).rev().step_by(STEPʹʹ)
```

if `relational_operator_0` = `>` or `relational_operator_1` = `<`, respectively, and abs(`step`) ≠ 1,

```rust
(Nʹʹ..=L).rev()
```

if `relational_operator_0` = `>=` or `relational_operator_1` = `<=`, respectively, and abs(`step`) = 1,

```rust
(Nʹʹ..=L).rev().step_by(STEPʹʹ)
```

if `relational_operator_0` = `>=` or `relational_operator_1` = `<=`, respectively, and abs(`step`) ≠ 1.

`Lʹ` has the form

```rust
LT
```

if `L` is an *unsuffixed integer literal expression* and `T` ∈ {`i8`, `i16`, `i32`, `i64`, `i128`, `isize`, `u8`, `u16`, `u32`, `u64`, `u128`, `usize`},

```rust
L as T
```

if `L` is an *unsuffixed integer literal expression* and `T` ∈ {`::core::ffi::c_schar`, `::core::ffi::c_uchar`, `::core::ffi::c_short`, `::core::ffi::c_ushort`, `::core::ffi::c_int`, `::core::ffi::c_uint`, `::core::ffi::c_long`, `::core::ffi::c_ulong`, `::core::ffi::c_longlong`, `::core::ffi::c_ulonglong`},

```rust
L
```

if `L` is a *`T`-value*, and

```rust
_L as T
```

if `L` = `_L as T`, where `_L` is a *`U0`-value*.

`Nʹ` has the form

```rust
_N0
```

if `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*,

```rust
_N1
```

if `N` = `_N1 as T` or `N` = `(_N1 as T)` or `N` = `_N1 as U1 as T` or `N` = `(_N1 as U1 as T)`, where `_N1` is a *stable `T`-value* and `U1`, if it exists in the expression, is a *primitive integer type*,

```rust
_N2 as T
```

if `N` = `_N2 as T` or `N` = `(_N2 as T)` or `N` = `_N2 as U2 as T` or `N` = `(_N2 as U2 as T)`, where `_N2` is a *stable `U3`-value*, `U3` is a *primitive integer type*, `U3` ≠ `T`, and `U2`, if it exists in the expression, is a *primitive integer type*, and

```rust
N
```

otherwise.

Nʹʹ has the form

```rust
NT
```

if `N` is an *unsuffixed integer literal expression* and `T` ∈ {`i8`, `i16`, `i32`, `i64`, `i128`, `isize`, `u8`, `u16`, `u32`, `u64`, `u128`, `usize`},

```rust
_N0T
```

if `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*, and `T` ∈ {`i8`, `i16`, `i32`, `i64`, `i128`, `isize`, `u8`, `u16`, `u32`, `u64`, `u128`, `usize`},

```rust
N as T
```

if `N` is an *unsuffixed integer literal expression* and `T` ∈ {`::core::ffi::c_schar`, `::core::ffi::c_uchar`, `::core::ffi::c_short`, `::core::ffi::c_ushort`, `::core::ffi::c_int`, `::core::ffi::c_uint`, `::core::ffi::c_long`, `::core::ffi::c_ulong`, `::core::ffi::c_longlong`, `::core::ffi::c_ulonglong`},

```rust
_N0 as T
```

if `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*, and `T` ∈ {`::core::ffi::c_schar`, `::core::ffi::c_uchar`, `::core::ffi::c_short`, `::core::ffi::c_ushort`, `::core::ffi::c_int`, `::core::ffi::c_uint`, `::core::ffi::c_long`, `::core::ffi::c_ulong`, `::core::ffi::c_longlong`, `::core::ffi::c_ulonglong`},

```rust
N
```

if `N` is a *stable `T`-value*,

```rust
_N1
```

if `N` = `_N1 as T` or `N` = `(_N1 as T)` or `N` = `_N1 as U1 as T` or `N` = `(_N1 as U1 as T)`, where `_N1` is a *stable `T`-value* and `U1`, if it exists in the expression, is a *primitive integer type*, and

```rust
_N2 as T
```

if `N` = `_N2 as T` or `N` = `(_N2 as T)` or `N` = `_N2 as U2 as T` or `N` = `(_N2 as U2 as T)`, where `_N2` is a *stable `U3`-value*, `U3` is a *primitive integer type*, `U3` ≠ `T`, and `U2`, if it exists in the expression, is a *primitive integer type*.

`Nʹʹʹ` has the form

```text
N_plus_1T
```

if `N` is an *unsuffixed integer literal expression* and `T` ∈ {`i8`, `i16`, `i32`, `i64`, `i128`, `isize`, `u8`, `u16`, `u32`, `u64`, `u128`, `usize`}, where `N_plus_1` denotes an *unsuffixed integer literal expression* whose value is the mathematical sum of `N` and 1,

```text
_N0_plus_1T
```

if `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*, and `T` ∈ {`i8`, `i16`, `i32`, `i64`, `i128`, `isize`, `u8`, `u16`, `u32`, `u64`, `u128`, `usize`}, where `_N0_plus_1` denotes an *unsuffixed integer literal expression* whose value is the mathematical sum of `_N0` and 1,

```rust
N_plus_1 as T
```

if `N` is an *unsuffixed integer literal expression* and `T` ∈ {`::core::ffi::c_schar`, `::core::ffi::c_uchar`, `::core::ffi::c_short`, `::core::ffi::c_ushort`, `::core::ffi::c_int`, `::core::ffi::c_uint`, `::core::ffi::c_long`, `::core::ffi::c_ulong`, `::core::ffi::c_longlong`, `::core::ffi::c_ulonglong`}, where `N_plus_1` denotes an *unsuffixed integer literal expression* whose value is the mathematical sum of `N` and 1,

```rust
_N0_plus_1 as T
```

if `N` = `(_N0)`, where `_N0` is an *unsuffixed integer literal expression*, and `T` ∈ {`::core::ffi::c_schar`, `::core::ffi::c_uchar`, `::core::ffi::c_short`, `::core::ffi::c_ushort`, `::core::ffi::c_int`, `::core::ffi::c_uint`, `::core::ffi::c_long`, `::core::ffi::c_ulong`, `::core::ffi::c_longlong`, `::core::ffi::c_ulonglong`}, where `_N0_plus_1` denotes an *unsuffixed integer literal expression* whose value is the mathematical sum of `_N0` and 1,

```rust
N + 1
```

if `N` is a *stable `T`-value*,

```rust
_N1 + 1
```

if `N` = `_N1 as T` or `N` = `(_N1 as T)` or `N` = `_N1 as U1 as T` or `N` = `(_N1 as U1 as T)`, where `_N1` is a *stable `T`-value* and `U1`, if it exists in the expression, is a *primitive integer type*, and

```rust
_N2 as T + 1
```

if `N` = `_N2 as T` or `N` = `(_N2 as T)` or `N` = `_N2 as U2 as T` or `N` = `(_N2 as U2 as T)`, where `_N2` is a *stable `U3`-value*, `U3` is a *primitive integer type*, `U3` ≠ `T`, and `U2`, if it exists in the expression, is a *primitive integer type*.

`STEPʹʹ` is

```rust
abs(step)
```

if `STEP` is an *unsuffixed integer literal expression*,

```rust
STEPʹ
```

if `STEP` is a *statically determinable `U4`-value* and `U4` = `usize`, and

```rust
STEPʹ as usize
```

if `STEP` is a *statically determinable `U4`-value* and `U4` ≠ `usize`.

Here, `STEPʹ` =

```rust
STEP
```

if `step` > 0, and

```rust
-STEP
```

if `step` < 0.

#### iterate-arrays-idiomatically

Subsequently, if `iterate-arrays-idiomatically` is enabled, every `for` loop of the form

```rust
for I in L..N {
    body
}
```

where all variables referred to here are newly introduced variables and `I` is an arbitrary variable name, is rewritten into

```rust
for I in 0..Nʹ {
    body
}
```

if all of the following conditions are satisfied:

1. `L` = `0T` or `L` = `0 as T`, where `T` is a *primitive integer type*.
2. Globally or within the same function before the `for` loop, a 1-dimensional array, let its name be `array`, is defined with size `length`. array is not `static mut` and `length` is an *unsuffixed integer literal expression*.
3. `N` is one of the following:
    1. `length`
    2. `LENGTH`, `LENGTH as T` or `LENGTH as U0 as T`, where `LENGTH` is an arbitrary variable name, `U0`, if it exists in the expression, is a *primitive integer type*, and one of the following conditions holds:
        1. `LENGTH` is defined globally as follows:
            ```rust
            pub const LENGTH: U1 = length;
            ```
            where `U1` is a *primitive integer type* and the value of `length` lies within the value ranges of `U1`, `T`, and, if it exists in the expression, `U0`.
        2. `LENGTH` is defined globally as follows:
            ```rust
            pub const LENGTH: TYPE_ALIAS = length;
            ```
            where `TYPE_ALIAS` is an arbitrary type alias name and is defined as follows:
            ```rust
            pub type TYPE_ALIAS = U2;
            ```
            where `U2` is a *primitive integer type* and the value of `length` lies within the value ranges of `U2`, `T`, and, if it exists in the expression, `U0`.
4. `body` contains one of the following expressions:
    ```rust
    array[I]
    array[I as usize]
    ```
    at least once.

`Nʹ` then has the form

```rust
array.len()
```

if `T` = `usize`, and

```rust
array.len() as T
```

if `T` ≠ `usize`.

#### pass-arrays-as-slices

Subsequently, if `pass-arrays-as-slices` is enabled, every function of the form

```text
[c0]unsafe extern "C" fn func([c1]parameter_0[c2], parameter_1[c3]) [c4]{
	    [c5]
	    for I in L..N {
	        body
	    }
	    [c6]
	}
```

```text
[c0]unsafe extern "C" fn func([c1]parameter_1[c2], parameter_0[c3]) [c4]{
    [c5]
    for I in L..N {
        body
    }
    [c6]
}
```

where all variables referred to here are newly introduced variables, `[c0]` to `[c6]` denote arbitrary code (which may also be empty), `func` is an arbitrary function name, and `I` is an arbitrary variable name, is rewritten into

```text
[c0]unsafe extern "C" fn func([c1]slice[c2][c3]) [c4]{
    [c5]
    for I in 0..Nʹ {
        bodyʹ
    }
    [c6]
}
```

if all of the following conditions are satisfied:

1. If `transform-public-functions` is disabled, `[c0]` does not contain `pub`.  
    If `transform-public-functions` is enabled, `[c0]` may contain `pub`.
    If `[c0]` does not contain `pub`, only calls that refer to this function definition are regarded as calls to `func` for the conditions below.  
    If `[c0]` contains `pub`, every `extern "C"` function declaration in the entire crate that declares the same externally linked function as `func` has a function signature equivalent to the signature of `func` with respect to the number, order, and types of its parameters and its return type. Every call that refers either directly to the definition of `func` or to one of these `extern "C"` declarations is regarded as a call to func for the conditions below.  
    Except for its definition and, if `[c0]` contains `pub`, the `extern "C"` function declarations described above, every use of `func` and every use of one of these declarations in the entire crate must be a direct function call. In particular, neither `func` nor one of these declarations may be stored, returned, passed as an argument, or otherwise used as a function value.  
2. `parameter_0` has the form
    ```rust
    array: *mutability U0
    ```
    or
    ```rust
    mut array: *mutability U0
    ```
    where `mutability` ∈ {`const`, `mut`}, `array` is an arbitrary variable name and `U0` is an arbitrary type. Except for its declaration as `parameter_0`, `array` occurs only in the bodies of `for` loops matching this pattern.

3. `parameter_1` has the form
    ```rust
    length: U1
    ```
    or
    ```rust
    mut length: U1
    ```
    where `length` is an arbitrary variable name and `U1` is a *primitive integer type*. During the entire function, `length` is only accessed for reading and *no reference or raw pointer to it is created*.
4. `L` = `0T` or `L` = `0 as T`, where `T` is a *primitive integer type*.
5. For `N`, one of the following conditions must hold:
    1. `N` = `length` and `U1` = `T`.
    2. `N` = `length as T`, `U1` ≠ `T`, and the value range of `U1` lies within the value range of `T`.
6. `body` contains at least one of the following expressions:
    ```rust
    *array.offset(I as isize)
    *(array as *const U0).offset(I as isize)
    *(array as *mut U0).offset(I as isize)
    ```
    at least once, and every occurrence of `array` in `body` is part of one of these expressions. If `mutability` = `const`, every occurrence of `*(array as *mut U0).offset(I as isize)` is only accessed for reading.
7. For every call to `func` in the entire crate, as defined above, all of the following conditions hold:
    1. Inside the calling function, an array, let its name be `_array`, is defined as follows:
        ```rust
        let _array: [U0; _length] = [[c7]];
        ```
        or
        ```rust
        let mut _array: [U0; _length] = [[c7]];
        ```
        where `_length` is an *unsuffixed integer literal expression* and `[c7]` denotes arbitrary code.
    2. `_array` is passed as `parameter_0` in the function call in one of the following forms:
        ```rust
        &raw const _array as *const U0
        &raw mut _array as *const U0 as *mut U0
        &raw mut _array as *mut U0
        &raw mut _array as *mut U0 as *const U0
        ```
    3. One of the following expressions is passed as `parameter_1` in the function call:
        1. `_length`
        2. `LENGTH`, `LENGTH as U1` or `LENGTH as U2 as U1`, where `LENGTH` is an arbitrary variable name, `U2`, if it exists in the expression, is a *primitive integer type*, and one of the following conditions holds:
            1. `LENGTH` is defined globally as follows:
                ```rust
                pub const LENGTH: U3 = _length;
                ```
                where `U3` is a *primitive integer type* and the value of `_length` lies within the value ranges of `U3`, `U1`, and, if it exists in the expression, `U2`.
            2. `LENGTH` is defined globally as follows:
                ```rust
                pub const LENGTH: TYPE_ALIAS = _length;
                ```
                where `TYPE_ALIAS` is an arbitrary type alias name and is defined as follows:
                ```rust
                pub type TYPE_ALIAS = U4;
                ```
                where `U4` is a *primitive integer type* and the value of `_length` lies within the value ranges of `U4`, `U1`, and, if it exists in the expression, `U2`.
        3. ```rust
            (::core::mem::size_of::<[U0; _length]>() as usize).wrapping_div(::core::mem::size_of::<U0>() as usize) as U1
            ```
            where `::core::mem::size_of::<U0>() as usize` ≠ 0 and the value of `_length` lies within the value range of `U1`.
        4. ```rust
            (::core::mem::size_of::<[U0; _length]>() as U5).wrapping_div(::core::mem::size_of::<U0>() as U5)
            ```
            where `U5` ∈ {`size_t`, `usize`} and `::core::mem::size_of::<U0>() as U5` ≠ 0.
    4. For every call to `func`, the aliasing conditions below must hold at the time of the call:  
        If `mutability` = `mut`, there must be no other live reference or raw pointer to `_array` or to any element of `_array` when the call begins or during the execution of the call, except for the raw pointer created by the expression through which `_array` is passed as `parameter_0` in that call.  
        If `mutability` = `const`, there must be no live mutable reference or raw pointer to `_array` or to any element of `_array` when the call begins or during the execution of the call, except for the raw pointer created by the expression through which `_array` is passed as `parameter_0` in that call.  
        A reference or raw pointer is considered live at a call if it was created before the call and may still be used during or after that call. In particular, a reference or raw pointer is considered live if it is stored in a variable, static item, aggregate, struct field, array element, or other memory location, returned from a function, or passed to code for which it cannot be established that the reference or raw pointer does not escape.  
        A raw pointer created solely as an argument of an earlier function call is not considered live after that call has completed if all of the following conditions hold:  
        1. The raw pointer value is used only as that argument and is not otherwise stored, returned, or copied in the calling function.
        2. The definition of the called function is available in the crate, either directly or, if the call refers to an `extern "C"` function declaration, as the definition of the same externally linked function.
        3. It can be established from the definition of the called function that the corresponding raw-pointer parameter does not escape from that function. This is the case if every occurrence of that parameter in the function body is used only as part of an expression that accesses the value pointed to by the parameter, and neither the parameter nor any reference or raw pointer derived from it is otherwise stored, returned, copied, captured, or passed to another function.

`slice` is then

```rust
	array: &[U0]
```

if `mutability` = `const`, and

```rust
	array: &mut [U0]
```

if `mutability` = `mut`.

`Nʹ` is then

```rust
	array.len()
```

if `T` = `usize`, and

```rust
	array.len() as T
```

if `T` ≠ `usize`.

`bodyʹ` is identical to `body`, except that every occurrence of

```rust
	*array.offset(I as isize)
```

or

```rust
	*(array as *const U0).offset(...)
```

or

```rust
	*(array as *mut U0).offset(...)
```

is replaced with

```rust
	array[I]
```

if `T` = `usize`, and

```rust
	array[I as usize]
```

if `T` ≠ `usize`.

Also, if there is any occurrence of `length` in `body`, `[c5]`, or `[c6]`, except in `N`, a new line is added before `[c5]`:

```rust
	let length: U1 = array.len();
```

if `U1` = `usize`, and

```rust
	let length: U1 = array.len() as U1;
```

if `U1` ≠ `usize`.

If the name `length` is already resolvable within the scope of the `for` loop, `length_0` is used instead. If `length_0` is also already resolvable, `length_1` is used, and so on.

If multiple `for` loops matching this pattern occur within the function, every such `for` loop is rewritten in this way, while the function signature is rewritten only once at the end. All other `for` loops that do not match this pattern are regarded as part of `[c5]` or `[c6]`.

Every call to `func` in the entire crate is rewritten so that it matches the signature of `func`: `parameter_1` is omitted, and `_array` is passed as `parameter_0` as a slice reference:

```rust
	&_array
```

if `slice` = `array: &[U0]`, and

```rust
	&mut _array
```

if `slice` = `array: &mut [U0]`.

If `[c0]` contains `pub`, every `extern "C"` function declaration in the entire crate that declares the same externally linked function as `func` is also rewritten so that its signature matches the rewritten definition of `func`: `parameter_1` is removed and `parameter_0` is replaced with a parameter of type

```rust
	&[U0]
```

if `mutability` = `const`, and

```rust
	&mut [U0]
```

if `mutability` = `mut`.

Every call that refers to one of these declarations is rewritten in the same way as a call that refers directly to the definition of `func`.

#### index-type-to-usize

Subsequently, if `index-type-to-usize` is enabled, every `for` loop of the form

```rust
for I in 0..N {
    body
}
```

where all variables referred to here are newly introduced variables and `I` is an arbitrary variable name, is rewritten into

```rust
for I in 0..Nʹ {
    bodyʹ
}
```

if all of the following conditions are satisfied:

1. `N` = `array.len()` or `N` = `array.len() as T`, where `array` is an array or slice and `T`, if it exists in the expression, is a *primitive integer type*.
2. `body` contains one of these expressions:
    ```rust
    array[I]
    array[I as usize]
    ```
    at least once.


`Nʹ` then has the form

```rust
array.len()
```

`bodyʹ` is identical to `body`, except that every occurrence of

```rust
I
```

is replaced with

```rust
I as T
```

if `N` = `array.len() as T` and `T` ≠ `usize`, and every occurrence of

```rust
I as usize
```

is replaced with

```rust
I
```

#### index-based-to-iterator-based

Subsequently, if `index-based-to-iterator-based` is enabled, every `for` loop of the form

```rust
for I in 0..array.len() {
    body
}
```

where all variables referred to here are newly introduced variables, `I` is an arbitrary variable name, and `array` is an array or slice, is rewritten into

```rust
for variable in expression {
    bodyʹ
}
```

if `array` occurs at least once in `body` and occurs exclusively in the form

```rust
array[I]
```

`variable in expression` then has the form

```rust
element in array.iter()
```

or

```rust
element in array.iter_mut()
```

if `I` occurs in `body` only in the form `array[I]`, and

```rust
(I, element) in array.iter().enumerate()
```

or

```rust
(I, element) in array.iter_mut().enumerate()
```

if `I` also occurs in another form in `body`.

Here, `iter()` is used if `array[I]` is only read in `body`, and `iter_mut()` is used if `array[I]` is modified in `body`.

`bodyʹ` is identical to `body`, except that every occurrence of

```rust
array[I]
```

is replaced with

```rust
*element
```

If the name `element` is already resolvable within the scope of the `for` loop, `element_0` is used instead. If `element_0` is also already resolvable, `element_1` is used, and so on.
