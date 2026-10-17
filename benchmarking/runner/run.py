#!/usr/bin/env python3
"""Build, execute, transpile, and analyze the standard and TRACTOR benchmarks.

Standard Makefile benchmarks run their C and Rust executables; the five selected
TRACTOR batteries are built, statically analyzed, and functionally tested
using the official TRACTOR test runner (Nix/Docker).
"""

import argparse
import os
import re
import resource
import shlex
import shutil
import signal
import subprocess
import sys
from pathlib import Path

from c_analysis import CAnalysisError, count_c_for_loops
from cmake_targets import CMakeTargetError, request_codemodel, select_cmake_compilation
from rust_analysis import (
    RustAnalysisError, analyze_rust_project, build_rust_analyzer,
    plain_tool_env,
)
from tractor_integration import BUNDLE_ORDER, discover_tractor_cases
from tractor_tests import run_tractor_vectors, summarize_tests


runner_dir = Path(__file__).resolve().parent
benchmarking_dir = runner_dir.parent
benchmarks_dir = benchmarking_dir / "benchmarks"
project_root = benchmarking_dir.parent
c2rust_root = project_root / "lib" / "c2rust"
c2rust_dir = c2rust_root / "target" / "release"
plugin_dir = project_root / "target" / "release"
temp_dir = runner_dir / "_temp"
reports_dir = runner_dir / "_results"
results_file = reports_dir / "results.txt"
outputs_file = reports_dir / "outputs.txt"
tractor_results_file = reports_dir / "results_tractor.txt"
tractor_tests_file = reports_dir / "tests_tractor.txt"
tractor_test_logs_dir = reports_dir / "tractor_test_logs"

# Original Makefile benchmarks, in their execution and report order.
BENCHMARKS = [
    {
        "name": "test",
        "path": project_root / "test" / "in",
        "kind": "binary",
        "make_target": "out/c2rust_idiomatizer_test",
        "make_runs_binary": False,
        "executable": "out/c2rust_idiomatizer_test",
        "main_module": "main",
    },
    {
        "name": "buffer",
        "path": benchmarks_dir / "buffer",
        "kind": "binary",
        "make_target": "test",
        "clean_target": None,
        "make_runs_binary": True,
        "main_module": "test",
    },
    {
        "name": "bzip2",
        "path": benchmarks_dir / "bzip2",
        "kind": "binary",
        "make_target": "bzip2",
        "make_runs_binary": False,
        "executable": "bzip2",
        "main_module": "bzip2",
        "run_args": ["--help"],
    },
    {
        "name": "genann",
        "path": benchmarks_dir / "genann",
        "kind": "binary",
        "make_target": "check",
        "make_runs_binary": True,
        "main_module": "test",
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


# TRACTOR's macrodepth variants use CMake cache variables to select their
# macro-generated operation and repetition count. CMake defaults (add, 5)
# do not match the expected test vectors for these two named configurations.
# Apply overrides only to our temporary CMake build; do not edit the corpus.
TRACTOR_CMAKE_OVERRIDES = {
    "B02_synthetic/macrodepth_mul_4": ("-DOP=mul", "-DREPEAT=4"),
    "B02_synthetic/macrodepth_sub_6": ("-DOP=sub", "-DREPEAT=6"),
}
METRIC_LABELS = (
    ("characters", "Characters of code"),
    ("c_type_occurrences", "C FFI type occurrences"),
    ("casts", "Casts"),
    ("for_loop_ratio", "For-loop ratio"),
    ("raw_pointer_parameters", "Raw pointer parameters"),
    ("raw_pointer_arithmetic", "Raw pointer arithmetic operations"),
    ("raw_pointer_dereferences", "Raw pointer dereferences"),
    ("indexing_operations", "Indexing operations"),
)


class StepError(RuntimeError):
    """A command failed, retaining its complete diagnostic output."""

    def __init__(self, stage: str, command: list[str], details: str,
                 returncode: int | None = None) -> None:
        super().__init__(stage)
        self.stage = stage
        self.command = command
        self.details = details
        self.returncode = returncode


def run_step(stage: str, command: list[str], *, cwd: Path | None = None,
             env: dict[str, str] | None = None) -> str:
    """Run a command, printing short progress and preserving logs on failures."""
    if env is None:
        env = plain_tool_env()
    try:
        result = subprocess.run(
            command, cwd=cwd, env=env, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True, errors="replace", check=False,
        )
    except OSError as exc:
        raise StepError(stage, command, str(exc)) from exc
    if result.returncode != 0:
        raise StepError(stage, command, result.stdout, result.returncode)
    print(f"  [OK] {stage}", flush=True)
    return result.stdout


def format_rust(directory: Path, stage: str) -> bool:
    """Try cargo fmt, but continue to the Rust build after any formatting failure."""
    command = ["cargo", "fmt", "--manifest-path", str(directory / "Cargo.toml")]
    try:
        run_step(stage, command, cwd=directory)
        return True
    except StepError as error:
        print(f"  [WARN] {stage}: {short_error(error)}; "
              "continuing with Rust build and analysis", flush=True)
        return False


def ensure_release_tools() -> None:
    """Verify prebuilt release tools; do not compile or repair missing artifacts."""
    required = (
        ("C2Rust transpiler", c2rust_dir / "c2rust-transpile", True),
        ("C2Rust refactorer", c2rust_dir / "c2rust-refactor", True),
        ("Idiomatizer plugin", plugin_dir / "libc2rust_idiomatizer.so", False),
    )
    print("=== Checking prebuilt release tools ===", flush=True)
    problems = []
    for label, path, executable in required:
        if not path.is_file():
            problems.append(f"{label}: missing file: {path}")
        elif not os.access(path, os.X_OK if executable else os.R_OK):
            permission = "executable" if executable else "readable"
            problems.append(f"{label}: file is not {permission}: {path}")
        else:
            print(f"  [OK] {label}: {path}", flush=True)
    if problems:
        raise RuntimeError(
            "Required prebuilt release artifacts are unavailable; no build was started.\n"
            + "\n".join(f"  - {problem}" for problem in problems)
            + "\nBuild the missing artifacts manually and rerun the benchmark."
        )


def set_stack_limit() -> None:
    """Equivalent to `ulimit -s 65536` for all subprocesses."""
    stack_bytes = 65536 * 1024
    _, hard_limit = resource.getrlimit(resource.RLIMIT_STACK)
    if hard_limit != resource.RLIM_INFINITY and hard_limit < stack_bytes:
        raise RuntimeError("Hard stack-size limit is lower than 64 MiB")
    resource.setrlimit(resource.RLIMIT_STACK, (stack_bytes, hard_limit))


def select_cases() -> tuple[list[dict], list]:
    """Select standard benchmarks first, then supported TRACTOR cases.

    With no flags, run all five standard benchmarks followed by all allowed
    TRACTOR cases. Excluding standard benchmarks with --NAME=false also keeps
    the default TRACTOR run; positive --NAME selections choose only standards.
    Explicit TRACTOR selections run the requested TRACTOR cases.
    """
    parser = argparse.ArgumentParser(
        description="Run the five standard benchmarks and the five supported TRACTOR batteries.",
        allow_abbrev=False,
    )
    for config in BENCHMARKS:
        name = config["name"]
        parser.add_argument(
            f"--{name}", nargs="?", const="true", choices=("true", "false"),
            default=None, metavar="{true,false}",
            help=f"Select {name}, or exclude it with --{name}=false",
        )
    parser.add_argument("--tractor-case", action="append", default=[],
                        metavar="BUNDLE/CASE", help="Select one allowed test case; repeatable")
    parser.add_argument("--tractor-bundle", action="append", default=[],
                        metavar="BUNDLE", help="Select one allowed battery; repeatable")
    parser.add_argument("--tractor-all", action="store_true",
                        help="Select all five allowed TRACTOR batteries")
    parser.add_argument("--tractor-limit", type=int, metavar="N",
                        help="Take the first N cases per selected --tractor-bundle")
    args = parser.parse_args()

    if args.tractor_all and (args.tractor_case or args.tractor_bundle or
                             args.tractor_limit is not None):
        parser.error("--tractor-all cannot be combined with case, bundle, or limit")
    if args.tractor_limit is not None:
        if args.tractor_limit < 1:
            parser.error("--tractor-limit must be at least 1")
        if not args.tractor_bundle:
            parser.error("--tractor-limit requires --tractor-bundle")

    for bundle in args.tractor_bundle:
        if bundle not in BUNDLE_ORDER:
            parser.error(f"Unsupported TRACTOR battery: {bundle}. "
                         f"Allowed: {', '.join(BUNDLE_ORDER)}")
    for case_id in args.tractor_case:
        if case_id.split("/", 1)[0] not in BUNDLE_ORDER:
            parser.error(f"Unsupported TRACTOR case: {case_id}. "
                         f"Allowed batteries: {', '.join(BUNDLE_ORDER)}")

    explicit_standard = any(getattr(args, b["name"]) is not None for b in BENCHMARKS)
    affirmative = [b for b in BENCHMARKS if getattr(args, b["name"]) == "true"]
    explicit_tractor = bool(args.tractor_all or args.tractor_case or args.tractor_bundle)
    # Pure --NAME=false flags exclude standard benchmarks, but should not
    # suppress the default TRACTOR run. Positive selections remain standard-only.
    default_tractor = not affirmative and not explicit_tractor
    if not explicit_standard and not explicit_tractor:
        selected_standard = list(BENCHMARKS)
    elif explicit_standard:
        selected_standard = affirmative if affirmative else [
            b for b in BENCHMARKS if getattr(args, b["name"]) != "false"]
    else:
        selected_standard = []

    cases = []
    if explicit_tractor or default_tractor:
        discovered = discover_tractor_cases()  # Excludes all unsupported batteries.
        by_id = {case.case_id: case for case in discovered}
        ids: set[str] = set()
        if args.tractor_all or default_tractor:
            ids.update(by_id)
        ids.update(args.tractor_case)
        for bundle in args.tractor_bundle:
            matching = [case.case_id for case in discovered if case.bundle == bundle]
            if not matching:
                parser.error(f"No TRACTOR cases found in battery: {bundle}")
            ids.update(matching[:args.tractor_limit])
        for case_id in ids:
            if case_id not in by_id:
                parser.error(f"Unknown TRACTOR case: {case_id}")
        cases = [case for case in discovered if case.case_id in ids]

    if not selected_standard and not cases:
        parser.error("No benchmarks selected")
    for config in selected_standard:
        if not config["path"].is_dir():
            parser.error(f"Benchmark source directory does not exist: {config['path']}")
    return selected_standard, cases


def standard_report(name: str, kind: str, transpiled: dict, refactored: dict,
                    warnings: list[str]) -> str:
    """Match original two-column report, with explicit executable/library type."""
    header = f"{'Metric':35} {'Transpiled':>14} {'Refactored':>14}"
    lines = [f"=== Benchmark: {name} ({kind}) ===", "", header, "-" * len(header)]
    for key, label in METRIC_LABELS:
        lines.append(f"{label:35} {metric_value(transpiled, key):>14} "
                     f"{metric_value(refactored, key):>14}")
    if warnings:
        lines.append("")
        lines.extend(f"Warning: {warning}" for warning in warnings)
    return "\n".join(lines)


def process_standard(config: dict, analyzer_exe: Path) -> tuple[dict, dict, list[str]]:
    """Original Make/Bear and execution pipeline, using release Rust artifacts."""
    name = config["name"]
    kind = config["kind"]
    input_dir = temp_dir / name / "in"
    transpile_dir = temp_dir / name / "out" / "transpile"
    refactor_dir = temp_dir / name / "out" / "refactor"
    print(f"\n=== Benchmark: {name} ({kind}) ===", flush=True)
    input_dir.mkdir(parents=True)
    transpile_dir.mkdir(parents=True)
    shutil.copytree(config["path"], input_dir, dirs_exist_ok=True)

    make_command = ["make", "-C", str(input_dir)]
    if config.get("make_target") is not None:
        make_command.append(config["make_target"])
    clean_target = config.get("clean_target", "clean")
    if clean_target is not None:
        run_step("C clean", ["make", "-C", str(input_dir), clean_target])
    else:
        print("  [SKIP] C clean (no clean target)", flush=True)

    if kind == "binary" and config["make_runs_binary"]:
        original_output = run_step(
            "C build and execution (Makefile)", ["make", "--silent", *make_command[1:]])
    else:
        run_step("C build", make_command)
        original_output = (
            run_step("C execution", [str(input_dir / config["executable"]),
                                     *config.get("run_args", [])], cwd=input_dir)
            if kind == "binary" else ""
        )

    if clean_target is not None:
        run_step("C clean before Bear", ["make", "-C", str(input_dir), clean_target])
    else:
        print("  [SKIP] C clean before Bear (no clean target)", flush=True)
    database = input_dir / "compile_commands.json"
    database.unlink(missing_ok=True)
    run_step("Compilation database generation",
             ["bear", "--output", str(database), "--", *make_command])
    if not database.is_file():
        raise RuntimeError(f"Bear succeeded but did not create {database}")

    c_for_loops = count_c_for_loops(database, input_dir)
    print("  [OK] C for-loop analysis", flush=True)

    command = [str(c2rust_dir / "c2rust-transpile")]
    if kind == "binary":
        command += ["--binary", config["main_module"], "--thin-binaries"]
    else:
        command += ["--emit-build-files"]
    command += ["--output-dir", str(transpile_dir), str(database)]
    transpile_output = run_step("C2Rust transpilation", command)
    if kind == "binary" and re.search(r"warning:\s*binary not used:", transpile_output, re.I):
        raise RuntimeError(f"C2Rust ignored main module {config['main_module']!r}")
    warnings = []
    if not format_rust(transpile_dir, "Transpiled Rust formatting"):
        warnings.append("Transpiled Rust: formatting failed; compiled unformatted Rust")

    rust_env = {**plain_tool_env(), "RUSTFLAGS": "-Awarnings"}
    cargo_action = "run" if kind == "binary" else "build"
    cargo_args = (["--", *config["run_args"]]
                  if kind == "binary" and config.get("run_args") else [])
    def cargo_command(directory: Path) -> list[str]:
        return ["cargo", cargo_action, "--release", "--manifest-path",
                str(directory / "Cargo.toml"),
                *(["--quiet"] if kind == "binary" else []), *cargo_args]

    transpiled_output = run_step(
        "Transpiled Rust execution" if kind == "binary" else "Transpiled Rust release build",
        cargo_command(transpile_dir), cwd=transpile_dir, env=rust_env)

    shutil.copytree(transpile_dir, refactor_dir, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns("target"))
    print("  [OK] Refactoring copy created", flush=True)
    for pass_name in REFACTOR_COMMANDS:
        run_step(f"Refactoring: {pass_name}",
                 [str(c2rust_dir / "c2rust-refactor"), "--cargo", "--plugin-dir",
                  str(plugin_dir), "--plugin-name", "c2rust_idiomatizer",
                  "--rewrite-mode", "inplace", pass_name], cwd=refactor_dir)
    if not format_rust(refactor_dir, "Refactored Rust formatting"):
        warnings.append("Refactored Rust: formatting failed; compiled unformatted Rust")
    refactored_output = run_step(
        "Refactored Rust execution" if kind == "binary" else "Refactored Rust release build",
        cargo_command(refactor_dir), cwd=refactor_dir, env=rust_env)

    if kind == "binary":
        with outputs_file.open("a", encoding="utf-8") as file:
            file.write(f"=== Benchmark: {name} ({kind}) ===\n\n")
            for title, output in (("Original C execution output", original_output),
                                  ("Transpiled Rust execution output", transpiled_output),
                                  ("Refactored Rust execution output", refactored_output)):
                file.write(f"--- {title} ---\n\n{output.rstrip()}\n\n")

    baseline = analyze_rust_project(transpile_dir, analyzer_exe)
    print("  [OK] Transpiled Rust analysis", flush=True)
    refactored = analyze_rust_project(refactor_dir, analyzer_exe)
    print("  [OK] Refactored Rust analysis", flush=True)
    for metrics in (baseline, refactored):
        metrics["for_loop_ratio"] = (metrics["for_loops"], c_for_loops)
    print(f"=== Completed: {name} ===", flush=True)
    return baseline, refactored, warnings


def metric_value(metrics: dict | None, key: str) -> str:
    if metrics is None:
        return "—"
    value = metrics[key]
    if key == "for_loop_ratio":
        return f"{value[0]}/{value[1]}"
    return f"{value:,}"


def case_report(case: dict) -> str:
    library = case["kind"] == "library"
    header = (f"{'Metric':35} {'Transpiled':>14} {'Refactored (true)':>20} "
              f"{'Refactored (false)':>21}" if library else
              f"{'Metric':35} {'Transpiled':>14} {'Refactored (true)':>20}")
    lines = [f"=== Benchmark: {case['name']} ({case['kind']}) ===", "", header, "-" * len(header)]
    for key, label in METRIC_LABELS:
        first = metric_value(case.get("transpiled"), key)
        true = metric_value(case.get("refactored"), key)
        row = f"{label:35} {first:>14} {true:>20}"
        if library:
            false = metric_value(case.get("refactored_abi"), key)
            row += f" {false:>21}"
        lines.append(row)

    errors = case.get("errors", [])
    if case.get("error") is not None:
        error = case["error"]
        stage = error.stage if isinstance(error, StepError) else case.get("stage", "Setup or analysis")
        errors = [*errors, (stage, error)]
    if errors:
        lines.append("")
        lines.extend(f"Error during {stage}: {short_error(error)}" for stage, error in errors)
    if case.get("warnings"):
        lines.append("")
        lines.extend(f"Warning: {warning}" for warning in case["warnings"])
    return "\n".join(lines)


def sum_metrics(cases: list[dict], variant: str) -> dict | None:
    if not cases:
        return None
    sums = {}
    for key, _ in METRIC_LABELS:
        values = [case[variant][key] for case in cases]
        if key == "for_loop_ratio":
            sums[key] = (sum(v[0] for v in values), sum(v[1] for v in values))
        else:
            sums[key] = sum(values)
    return sums


def total_table(heading: str, cohort: list[dict], variant: str,
                eligible_count: int) -> str:
    label = "true" if variant == "refactored" else "false"
    header = f"{'Metric':35} {'Transpiled':>14} {('Refactored (' + label + ')'):>20}"
    a, b = sum_metrics(cohort, "transpiled"), sum_metrics(cohort, variant)
    lines = [f"=== {heading} ===", "", header, "-" * len(header)]
    for key, name in METRIC_LABELS:
        lines.append(f"{name:35} {metric_value(a, key):>14} {metric_value(b, key):>20}")
    excluded = eligible_count - len(cohort)
    if excluded > 0:
        qualifier = heading.removeprefix("TRACTOR benchmarks — total ")
        lines.extend([
            "",
            f"Note: {excluded} of {eligible_count} TRACTOR benchmarks excluded from "
            f"the total {qualifier} because not all required variants were analyzed.",
        ])
    return "\n".join(lines)


def totals_report(cases: list[dict]) -> str:
    libraries = [case for case in cases if case["kind"] == "library"]
    # The mixed total requires both variants for libraries (three measurements),
    # but only baseline+true for executables (false is not applicable).
    true_both = [
        case for case in cases
        if case.get("transpiled") is not None
        and case.get("refactored") is not None
        and (case["kind"] != "library" or case.get("refactored_abi") is not None)
    ]
    # Each library-only total uses its own matched baseline/variant cohort.
    true_libraries = [
        case for case in libraries
        if case.get("transpiled") is not None and case.get("refactored") is not None
    ]
    false_libraries = [
        case for case in libraries
        if case.get("transpiled") is not None and case.get("refactored_abi") is not None
    ]
    return "\n\n".join((
        total_table("TRACTOR benchmarks — total (true, both)", true_both,
                    "refactored", len(cases)),
        total_table("TRACTOR benchmarks — total (true, libraries)", true_libraries,
                    "refactored", len(libraries)),
        total_table("TRACTOR benchmarks — total (false, libraries)", false_libraries,
                    "refactored_abi", len(libraries)),
    ))


def save_results(cases: list[dict]) -> None:
    """Keep complete-case totals and partial per-case results after each case."""
    reports = [case_report(case) for case in cases]
    reports.append(totals_report(cases))
    tractor_results_file.write_text("\n\n".join(reports) + "\n", encoding="utf-8")


def short_error(error: Exception) -> str:
    if isinstance(error, StepError) and error.returncode is not None:
        if error.returncode < 0:
            number = -error.returncode
            try:
                name = signal.Signals(number).name
            except ValueError:
                name = str(number)
            return f"process terminated by {name} (exit status {error.returncode})"
    output = error.details if isinstance(error, StepError) else str(error)
    output = re.sub(r"\x1b\[[0-?]*[ -/]*[@-~]", "", output)
    lines = [line.strip() for line in output.splitlines() if line.strip()]
    for line in lines:
        if re.match(r"^(?:fatal error|error(?:\[[^]]+\])?):", line, re.I):
            return line[:200]
    if isinstance(error, StepError) and error.returncode is not None:
        return f"exit status {error.returncode}: {(lines[-1] if lines else 'no diagnostic')[:160]}"
    return (lines[0] if lines else "unknown error")[:200]


def process_case(case, analyzer_exe: Path, progress: dict) -> None:
    """Process a TRACTOR case, keeping baseline and variant outcomes separate."""
    name = "tractor__" + case.case_id.replace("/", "__")
    display_name = f"tractor/{case.case_id}"
    print(f"\n=== Benchmark: {display_name} ({case.kind}) ===", flush=True)
    work_dir = temp_dir / name
    input_dir = work_dir / "in"
    build_dir = work_dir / "out" / "c_build"
    transpile_dir = work_dir / "out" / "transpile"
    input_dir.mkdir(parents=True)
    transpile_dir.mkdir(parents=True)
    shutil.copytree(case.c_source_dir, input_dir, dirs_exist_ok=True)

    if not (input_dir / "CMakeLists.txt").is_file():
        raise RuntimeError(f"No CMakeLists.txt for {case.case_id}")

    progress["stage"] = "CMake configuration"
    request_codemodel(build_dir)
    cmake_overrides = TRACTOR_CMAKE_OVERRIDES.get(case.case_id, ())
    run_step(
        "CMake configuration",
        ["cmake", "-S", str(input_dir), "-B", str(build_dir), "-G", "Ninja",
         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_C_COMPILER=clang",
         *cmake_overrides],
    )
    if cmake_overrides:
        print(f"  [OK] TRACTOR case CMake parameters: {' '.join(cmake_overrides)}",
              flush=True)

    # Keep this selection logic even for the smaller batteries: it excludes
    # unrelated translation units and finds the *actual* main C source.
    progress["stage"] = "CMake target selection"
    database, main_module, cmake_target, native_link_flags = select_cmake_compilation(
        build_dir, input_dir, case.kind,
    )
    progress["stage"] = "C build (selected CMake target)"
    run_step(progress["stage"], ["cmake", "--build", str(build_dir),
                                  "--target", cmake_target])

    test_logs_dir = tractor_test_logs_dir / name
    progress["vector_tests"] = {}

    def test_variant(key: str, label: str, rust_project: Path | None) -> None:
        progress["stage"] = f"{label}: TRACTOR test vectors"
        result = run_tractor_vectors(
            case, key, rust_project, test_logs_dir,
            binary_name=cmake_target if case.kind == "binary" else None,
            main_module=main_module if case.kind == "binary" else None)
        progress["vector_tests"][key] = result
        if result.status == "not run":
            print(f"  [WARN] {progress['stage']}: not run ({result.reason})", flush=True)
        elif result.status == "failed":
            print(f"  [FAILED] {progress['stage']}: "
                  f"{result.passed} passed, {result.failed} failed, {result.skipped} skipped"
                  + (f" ({result.reason})" if result.reason else ""), flush=True)
        else:
            print(f"  [OK] {progress['stage']}: "
                  f"{result.passed} passed, {result.failed} failed, {result.skipped} skipped",
                  flush=True)

    test_variant("c", "Original C", None)

    progress["stage"] = "C for-loop analysis"
    c_for_loops = count_c_for_loops(database, input_dir)
    print("  [OK] C for-loop analysis", flush=True)

    progress["stage"] = "C2Rust transpilation"
    command = [str(c2rust_dir / "c2rust-transpile")]
    if case.kind == "binary":
        command += ["--binary", main_module, "--thin-binaries"]
    else:
        command.append("--emit-build-files")
    command += ["--output-dir", str(transpile_dir), str(database)]
    output = run_step(progress["stage"], command)
    if case.kind == "binary" and re.search(r"warning:\s*binary not used:", output, re.I):
        raise RuntimeError(f"C2Rust ignored main module {main_module!r}")

    progress["stage"] = "Transpiled Rust formatting"
    if not format_rust(transpile_dir, progress["stage"]):
        progress.setdefault("warnings", []).append(
            "Transpiled Rust: formatting failed; compiled unformatted Rust")

    rust_env = {**plain_tool_env(),
                "RUSTFLAGS": shlex.join(["-Awarnings", *native_link_flags])}
    progress["stage"] = "Transpiled Rust release build"
    run_step(progress["stage"], ["cargo", "build", "--release", "--manifest-path",
                                  str(transpile_dir / "Cargo.toml")],
             cwd=transpile_dir, env=rust_env)

    test_variant("transpiled", "Transpiled Rust", transpile_dir)

    progress["stage"] = "Transpiled Rust analysis"
    baseline = analyze_rust_project(transpile_dir, analyzer_exe)
    baseline["for_loop_ratio"] = (baseline["for_loops"], c_for_loops)
    progress["transpiled"] = baseline
    print("  [OK] Transpiled Rust analysis", flush=True)

    variants = [(True, "refactored", "refactor_public_true")]
    if case.kind == "library":
        variants.append((False, "refactored_abi", "refactor_public_false"))

    for transform_public, key, folder in variants:
        prefix = f"Refactored Rust (transform-public-functions={str(transform_public).lower()})"
        directory = work_dir / "out" / folder
        try:
            progress["stage"] = f"{prefix}: copy"
            shutil.copytree(transpile_dir, directory, dirs_exist_ok=True,
                            ignore=shutil.ignore_patterns("target"))
            print(f"  [OK] {progress['stage']}", flush=True)
            for refactor_command in REFACTOR_COMMANDS:
                progress["stage"] = f"{prefix}: {refactor_command}"
                command = [str(c2rust_dir / "c2rust-refactor"), "--cargo",
                           "--plugin-dir", str(plugin_dir),
                           "--plugin-name", "c2rust_idiomatizer",
                           "--rewrite-mode", "inplace", refactor_command]
                if refactor_command == "idiomatize_loops" and not transform_public:
                    command.append("transform-public-functions=false")
                run_step(progress["stage"], command, cwd=directory)

            progress["stage"] = f"{prefix}: formatting"
            if not format_rust(directory, progress["stage"]):
                progress.setdefault("warnings", []).append(
                    f"{prefix}: formatting failed; compiled unformatted Rust")
            progress["stage"] = f"{prefix}: release build"
            run_step(progress["stage"], ["cargo", "build", "--release", "--manifest-path",
                                          str(directory / "Cargo.toml")],
                     cwd=directory, env=rust_env)
            test_variant(key, prefix, directory)
            progress["stage"] = f"{prefix}: analysis"
            metrics = analyze_rust_project(directory, analyzer_exe)
            metrics["for_loop_ratio"] = (metrics["for_loops"], c_for_loops)
            progress[key] = metrics
            print(f"  [OK] {progress['stage']}", flush=True)
        except (StepError, OSError, RuntimeError, RustAnalysisError, CAnalysisError) as error:
            stage = error.stage if isinstance(error, StepError) else progress["stage"]
            progress.setdefault("errors", []).append((stage, error))
            print(f"  [FAILED] {stage}", flush=True)

    if not progress.get("errors") and all(
        result.status == "complete" for result in progress.get("vector_tests", {}).values()
    ):
        print(f"=== Completed: {display_name} ===", flush=True)


def main() -> int:
    standards, cases = select_cases()
    try:
        set_stack_limit()
        ensure_release_tools()
        print("=== Building release Rust analyzer ===", flush=True)
        analyzer = build_rust_analyzer(runner_dir / "rust_metrics")
        print("  [OK] Rust analyzer built", flush=True)

        if temp_dir.is_symlink():
            raise RuntimeError(f"Refusing to delete symbolic link: {temp_dir}")
        if temp_dir.exists():
            shutil.rmtree(temp_dir)
        temp_dir.mkdir(parents=True)
        reports_dir.mkdir(parents=True, exist_ok=True)
        if standards:
            results_file.unlink(missing_ok=True)
            outputs_file.unlink(missing_ok=True)
        if cases:
            tractor_results_file.unlink(missing_ok=True)
            tractor_tests_file.unlink(missing_ok=True)
            if tractor_test_logs_dir.is_symlink():
                raise RuntimeError(f"Refusing symbolic link: {tractor_test_logs_dir}")
            if tractor_test_logs_dir.exists():
                shutil.rmtree(tractor_test_logs_dir)

        standard_reports = []
        collected = []
        failures = 0

        # Always process the standard benchmarks before any TRACTOR battery.
        for config in standards:
            name = config["name"]
            try:
                baseline, refactored, warnings = process_standard(config, analyzer)
                standard_reports.append(
                    standard_report(name, config["kind"], baseline, refactored, warnings))
            except Exception as error:
                # Continue to the next benchmark (but still propagate Ctrl+C).
                failures += 1
                stage = error.stage if isinstance(error, StepError) else "Setup or analysis"
                print(f"  [FAILED] {name}: {stage}", flush=True)
                standard_reports.append(
                    f"=== Benchmark: {name} ({config['kind']}) ===\n\n"
                    f"Error during {stage}: {short_error(error)}")
                results_file.write_text("\n\n".join(standard_reports) + "\n", encoding="utf-8")
            else:
                results_file.write_text("\n\n".join(standard_reports) + "\n", encoding="utf-8")

        for case in cases:
            progress = {}
            error = None
            try:
                process_case(case, analyzer, progress)
            except Exception as exc:
                # A single failing TRACTOR case must not terminate the batch.
                error = exc
                stage = exc.stage if isinstance(exc, StepError) else progress.get("stage", "Setup")
                print(f"  [FAILED] tractor/{case.case_id}: {stage}", flush=True)

            test_results = progress.get("vector_tests", {})
            tests_failed = any(result.status != "complete" for result in test_results.values())
            failed = error is not None or bool(progress.get("errors")) or tests_failed
            failures += bool(failed)
            collected.append({
                "name": f"tractor/{case.case_id}",
                "kind": case.kind,
                **progress,
                "error": error,
            })
            save_results(collected)
            tractor_tests_file.write_text(summarize_tests(collected), encoding="utf-8")

        if standards:
            print(f"\nStandard results saved in: {results_file}")
            print(f"Standard benchmarks processed: {len(standards)}")
        if cases:
            print(f"\nTRACTOR results saved in: {tractor_results_file}")
            print(f"TRACTOR test vectors saved in: {tractor_tests_file}")
            print(f"TRACTOR cases processed: {len(collected)}")
        print(f"Failed benchmarks: {failures}")
        return 1 if failures else 0

    except StepError as exc:
        print(f"\n[ERROR] {exc.stage} failed", file=sys.stderr)
        print(f"Command: {shlex.join(exc.command)}", file=sys.stderr)
        if exc.returncode is not None:
            print(f"Exit status: {exc.returncode}", file=sys.stderr)
        if exc.details.strip():
            print(exc.details.rstrip(), file=sys.stderr)
        return 1
    except (OSError, RuntimeError, RustAnalysisError, CAnalysisError,
            CMakeTargetError) as exc:
        print(f"\n[ERROR] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
