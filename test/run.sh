#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

rm -rf .out/*
rm -rf out/*

mkdir -p .out/gcc
gcc src/main.c -o .out/gcc/main

mkdir -p .out/c2rust
c2rust-transpile -b main -o .out/c2rust src/main.c
mkdir -p out
cp .out/c2rust/src/main.rs out/main_transpile.rs

cd .out/c2rust
c2rust-refactor --cargo --bin main -P ~/projects/c2rust_idiomatizer/target/debug -p c2rust_idiomatizer -r alongside replace_integers \; remove_redundant_integer_casts
cp src/main.new ../../out/main_refactor.rs
