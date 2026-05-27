#!/usr/bin/env bash
set -e
set -x
cd "$(dirname "$0")"

rm -rf out/*
mkdir -p out/transpile
mkdir -p out/refactor

cd in
make clean
make
make clean
bear -- make
cd ..
~/projects/c2rust_idiomatizer/lib/c2rust/target/debug/c2rust-transpile \
  --emit-build-files \
  --output-dir out/transpile \
  in/compile_commands.json

cp -r out/transpile/* out/refactor
cd out/refactor
~/projects/c2rust_idiomatizer/lib/c2rust/target/debug/c2rust-refactor \
  --cargo \
  --plugin-dir ~/projects/c2rust_idiomatizer/target/debug \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  replace_integers
~/projects/c2rust_idiomatizer/lib/c2rust/target/debug/c2rust-refactor \
  --cargo \
  --plugin-dir ~/projects/c2rust_idiomatizer/target/debug \
  --plugin-name c2rust_idiomatizer \
  --rewrite-mode inplace \
  remove_redundant_integer_casts
