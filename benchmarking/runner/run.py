#!/usr/bin/env python3
"""Build, transpile, refactor, and validate selected C2Rust benchmarks."""

import argparse
import os
import resource
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

from c_analysis import CAnalysisError, count_c_for_loops
from rust_analysis import (
    RustAnalysisError, analyze_rust_project, build_rust_analyzer,
    format_summary, plain_tool_env, NIGHTLY,
)


# Project directories
runner_dir = Path(__file__).resolve().parent
benchmarking_dir = runner_dir.parent
benchmarks_dir = benchmarking_dir / "benchmarks"
project_root = benchmarking_dir.parent
c2rust_dir = project_root / "lib" / "c2rust" / "target" / "debug"
plugin_dir = project_root / "target" / "debug"

# Temporary workspace, recreated for each benchmark
temp_dir = runner_dir / "_temp"
input_dir = temp_dir / "in"
transpile_dir = temp_dir / "out" / "transpile"
refactor_dir = temp_dir / "out" / "refactor"

# Persistent text-only evaluation results
results_file = runner_dir / "_results.txt"

# Only benchmarks explicitly listed here are eligible for execution.
# Each entry has a name and a source directory; the remaining fields describe
# how to build, run, and transpile the benchmark.
#
# kind:
#   "binary"  -> transpile with --binary MODULE --thin-binaries; cargo run
#   "library" -> transpile with --emit-build-files; cargo build
#
# make_target: Makefile target to build, or None for the default target.
#
# Binary-only fields:
#   make_runs_binary: True if the Makefile target also runs the C executable
#   executable: executable path relative to the benchmark root, if not run by make
#   main_module: stem of the C file defining main() (e.g. src/main.c -> "main")
BENCHMARKS = [
    {
        "name": "test",
        "path": project_root / "test" / "in",
        "kind": "binary",
        "make_target": "out/c2rust_idiomatizer_test",
        "make_runs_binary": False,
        "executable": "out/c2rust_idiomatizer_test",
        "main_module": "main",  # src/main.c
    },
    {
        "name": "urlparser",
        "path": benchmarks_dir / "urlparser",
        "kind": "binary",
        "make_target": "url-test",
        "make_runs_binary": True,
        "main_module": "test",
    },
]

REFACTOR_COMMANDS = (
    "replace_integer_types",
    "remove_redundant_integer_casts",
    "idiomatize_loops",
)

class StepError(Exception):
    """A benchmark command could not start or exited unsuccessfully."""

    def __init__(
        self,
        stage: str,
        command: list[str],
        details: str,
        returncode: int | None = None,
    ) -> None:
        super().__init__(stage)
        self.stage = stage
        self.command = command
        self.details = details
        self.returncode = returncode


def run_step(
    stage: str,
    command: list[str],
    *,
    cwd: Path | None = None,
    env: dict[str, str] | None = None,
) -> None:
    """Hide successful commands' output; show their COMPLETE output on failure."""
    if env is None:
        env = plain_tool_env()
    try:
        result = subprocess.run(
            command,
            cwd=cwd,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            check=False,
        )
    except OSError as exc:
        raise StepError(stage, command, str(exc)) from exc

    if result.returncode != 0:
        raise StepError(stage, command, result.stdout, result.returncode)

    print(f"  [OK] {stage}", flush=True)


def ensure_refactor_plugin() -> None:
    """Build the plugin dylib if it isn't already present."""
    plugin_path = plugin_dir / "libc2rust_idiomatizer.so"
    if not plugin_path.is_file():
        print("=== Building missing C2Rust-Refactor plugin ===", flush=True)
        run_step(
            "C2Rust-Refactor plugin build",
            ["cargo", f"+{NIGHTLY}", "build", "--manifest-path", str(project_root / "Cargo.toml")],
            cwd=project_root,
        )
    if not plugin_path.is_file():
        raise RuntimeError(
            f"Expected C2Rust-Refactor plugin not found: {plugin_path}. "
            "The project's Cargo.toml must build a dylib named c2rust_idiomatizer."
        )
    print("  [OK] C2Rust-Refactor plugin available", flush=True)


def set_stack_limit() -> None:
    """Equivalent to the Linux shell command: ulimit -s 65536."""
    stack_bytes = 65536 * 1024
    _, hard_limit = resource.getrlimit(resource.RLIMIT_STACK)
    if hard_limit != resource.RLIM_INFINITY and hard_limit < stack_bytes:
        raise RuntimeError("Hard stack-size limit is lower than 64 MiB")
    resource.setrlimit(resource.RLIMIT_STACK, (stack_bytes, hard_limit))


def validate_benchmark_config(name: str, config: dict) -> None:
    """Reject missing or inconsistent benchmark settings before running it."""
    kind = config.get("kind")
    if kind not in {"binary", "library"}:
        raise RuntimeError(f"Benchmark '{name}': kind must be 'binary' or 'library'")

    target = config.get("make_target")
    if target is not None and (not isinstance(target, str) or not target):
        raise RuntimeError(f"Benchmark '{name}': make_target must be a name or None")

    if kind == "binary":
        runs = config.get("make_runs_binary")
        if not isinstance(runs, bool):
            raise RuntimeError(
                f"Benchmark '{name}': make_runs_binary must be True or False"
            )
        module = config.get("main_module")
        if not isinstance(module, str) or not module or module.endswith(".c"):
            raise RuntimeError(
                f"Benchmark '{name}': main_module must name a C module without .c"
            )
        if not runs:
            executable = config.get("executable")
            if not isinstance(executable, str) or not executable:
                raise RuntimeError(
                    f"Benchmark '{name}': executable is required when "
                    "make_runs_binary is False"
                )


def select_benchmarks() -> list[dict]:
    """Select registered benchmarks by --NAME, or all registered benchmarks."""
    registered = {}
    for config in BENCHMARKS:
        name = config.get("name")
        if not isinstance(name, str) or not name or name.startswith("-"):
            raise RuntimeError(f"Invalid registered benchmark name: {name!r}")
        if name in registered:
            raise RuntimeError(f"Duplicate benchmark name in BENCHMARKS: {name}")
        registered[name] = config

    parser = argparse.ArgumentParser(
        description="Build, transpile, refactor, and validate registered C benchmarks.",
        allow_abbrev=False,
    )
    for name in registered:
        parser.add_argument(
            f"--{name}",
            dest="selected",
            action="append_const",
            const=name,
            help=f"Process the {name} benchmark",
        )

    args = parser.parse_args()
    if not registered:
        parser.error("No benchmarks configured in BENCHMARKS")

    selected_names = (
        list(dict.fromkeys(args.selected))
        if args.selected
        else list(registered)
    )
    selected = [registered[name] for name in selected_names]
    for config in selected:
        source = config.get("path")
        if not isinstance(source, Path) or not source.is_dir():
            parser.error(
                f"Benchmark '{config['name']}' directory does not exist: {source}"
            )
    return selected


def process_benchmark(config: dict, analyzer_exe: Path) -> dict:
    name = config["name"]
    benchmark = config["path"]
    validate_benchmark_config(name, config)
    kind = config["kind"]
    make_command = ["make", "-C", str(input_dir)]
    if config.get("make_target") is not None:
        make_command.append(config["make_target"])

    print(f"\n=== Benchmark: {name} ({kind}) ===", flush=True)

    # Keep _temp after the run for inspection; reset it before each benchmark.
    if temp_dir.is_symlink():
        raise RuntimeError(f"Refusing to delete symbolic link: {temp_dir}")
    if temp_dir.exists():
        shutil.rmtree(temp_dir)

    input_dir.mkdir(parents=True)
    transpile_dir.mkdir(parents=True)
    refactor_dir.mkdir(parents=True)
    shutil.copytree(benchmark, input_dir, dirs_exist_ok=True)

    # Build original C project, and run it when it is an executable benchmark.
    run_step("C clean", ["make", "-C", str(input_dir), "clean"])
    if kind == "binary" and config["make_runs_binary"]:
        run_step("C build and execution (Makefile)", make_command)
    else:
        run_step("C build", make_command)
        if kind == "binary":
            executable = config["executable"]
            run_step(
                "C execution",
                [str(input_dir / executable)],
                cwd=input_dir,
            )

    # Bear must capture a fresh compilation for C2Rust and future C analysis.
    run_step(
        "C clean before Bear",
        ["make", "-C", str(input_dir), "clean"],
    )
    compile_commands = input_dir / "compile_commands.json"
    run_step(
        "Compilation database generation",
        ["bear", "--output", str(compile_commands), "--", *make_command],
    )
    if not compile_commands.is_file():
        raise RuntimeError(f"Bear succeeded but did not create {compile_commands}")

    # Count original C for-loops with Clang, using Bear's exact build flags.
    c_for_loops = count_c_for_loops(compile_commands, input_dir)
    print("  [OK] C for-loop analysis", flush=True)

    # Choose C2Rust's Cargo project type based on the benchmark configuration.
    transpile_command = [str(c2rust_dir / "c2rust-transpile")]
    if kind == "binary":
        transpile_command.extend([
            "--binary", config["main_module"], "--thin-binaries",
        ])
    else:
        transpile_command.append("--emit-build-files")
    transpile_command.extend([
        "--output-dir", str(transpile_dir), str(compile_commands),
    ])

    run_step("C2Rust transpilation", transpile_command)
    run_step(
        "Transpiled Rust formatting",
        ["cargo", "fmt", "--manifest-path", str(transpile_dir / "Cargo.toml")],
        cwd=transpile_dir,
    )

    # Binaries can be run; library crates should be built instead.
    cargo_action = "run" if kind == "binary" else "build"
    stage_action = "execution" if kind == "binary" else "build"
    run_step(
        f"Transpiled Rust {stage_action}",
        ["cargo", cargo_action, "--manifest-path", str(transpile_dir / "Cargo.toml")],
        cwd=transpile_dir,
        env={**plain_tool_env(), "RUSTFLAGS": "-Awarnings"},
    )

    # Don't copy Cargo's compiled artifacts into the refactored project.
    shutil.copytree(
        transpile_dir,
        refactor_dir,
        dirs_exist_ok=True,
        ignore=shutil.ignore_patterns("target"),
    )
    print("  [OK] Refactoring copy created", flush=True)

    for refactor_command in REFACTOR_COMMANDS:
        run_step(
            f"Refactoring: {refactor_command}",
            [
                str(c2rust_dir / "c2rust-refactor"),
                "--cargo",
                "--plugin-dir", str(plugin_dir),
                "--plugin-name", "c2rust_idiomatizer",
                "--rewrite-mode", "inplace",
                refactor_command,
            ],
            cwd=refactor_dir,
        )

    run_step(
        "Refactored Rust formatting",
        ["cargo", "fmt", "--manifest-path", str(refactor_dir / "Cargo.toml")],
        cwd=refactor_dir,
    )
    run_step(
        f"Refactored Rust {stage_action}",
        ["cargo", cargo_action, "--manifest-path", str(refactor_dir / "Cargo.toml")],
        cwd=refactor_dir,
        env={**plain_tool_env(), "RUSTFLAGS": "-Awarnings"},
    )

    # Evaluate both finished Rust projects using the pinned rustc analyzer.
    transpiled_metrics = analyze_rust_project(transpile_dir, analyzer_exe)
    print("  [OK] Transpiled Rust analysis", flush=True)
    refactored_metrics = analyze_rust_project(refactor_dir, analyzer_exe)
    print("  [OK] Refactored Rust analysis", flush=True)

    for metrics in (transpiled_metrics, refactored_metrics):
        metrics["for_loop_ratio"] = (
            100.0 * metrics["for_loops"] / c_for_loops
            if c_for_loops else None
        )

    print(f"=== Completed: {name} ===", flush=True)
    return {"transpiled": transpiled_metrics, "refactored": refactored_metrics}


def main() -> int:
    selected = select_benchmarks()
    try:
        # Never leave stale results from an earlier successful run.
        results_file.unlink(missing_ok=True)
        set_stack_limit()
        print("=== Building pinned-nightly Rust analyzer ===", flush=True)
        analyzer_exe = build_rust_analyzer(runner_dir / "rust_metrics")
        print("  [OK] Rust analyzer built", flush=True)
        ensure_refactor_plugin()

        benchmark_results = {}
        for benchmark in selected:
            benchmark_results[benchmark["name"]] = process_benchmark(benchmark, analyzer_exe)

        summary = format_summary(benchmark_results)
        print("\n" + summary)
        results_file.write_text(summary + "\n", encoding="utf-8")
        print(f"\nResults saved in: {results_file}")
    except StepError as exc:
        print(f"\n[ERROR] {exc.stage} failed", file=sys.stderr)
        print(f"Command: {shlex.join(exc.command)}", file=sys.stderr)
        if exc.returncode is not None:
            print(f"Exit status: {exc.returncode}", file=sys.stderr)
        if exc.details.strip():
            print("Output (complete):", file=sys.stderr)
            print(exc.details.rstrip(), file=sys.stderr)
        return 1
    except (OSError, RuntimeError, RustAnalysisError) as exc:
        print(f"\n[ERROR] {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
