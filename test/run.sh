#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

rm -rf .out/*
rm -rf out/*

mkdir -p .out/gcc
gcc src/main.c -o .out/gcc/main

mkdir -p .out/c2rust-transpile
c2rust-transpile -b main -o .out/c2rust-transpile src/main.c
mkdir -p out
cp .out/c2rust-transpile/src/main.rs out/main_transpile.rs

cd .out/c2rust-transpile
c2rust-refactor --cargo --bin main -P ~/projects/c2rust_idiomatizer/target/debug -p c2rust_idiomatizer -r alongside test_one_plus_one \; replace_integers
cp src/main.new ../../out/main_refactor.rs
