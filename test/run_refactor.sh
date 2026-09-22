#!/usr/bin/env bash
set -e
set -x
ulimit -s 65536

test_dir="$(cd "$(dirname "$0")" && pwd)"
project_root="$(cd "$test_dir/.." && pwd)"
c2rust_dir="$project_root/lib/c2rust/target/debug"
plugin_dir="$project_root/target/debug"

rm -rf "$test_dir/out/refactor"
mkdir -p "$test_dir/out/refactor"

cp -r "$test_dir/out/transpile/"* "$test_dir/out/refactor"
cd "$test_dir/out/refactor"
"$c2rust_dir/c2rust-refactor" \
  --cargo \
  --plugin-dir "$plugin_dir" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  replace_integer_types
"$c2rust_dir/c2rust-refactor" \
  --cargo \
  --plugin-dir "$plugin_dir" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  remove_redundant_integer_casts
"$c2rust_dir/c2rust-refactor" \
  --cargo \
  --plugin-dir "$plugin_dir" \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  idiomatize_loops
cargo fmt --manifest-path "$test_dir/out/refactor/Cargo.toml"
RUSTFLAGS="-Awarnings"  cargo run --manifest-path "$test_dir/out/refactor/Cargo.toml"
