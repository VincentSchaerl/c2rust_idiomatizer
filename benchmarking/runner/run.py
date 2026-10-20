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
import time
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
tractor_metrics_file = reports_dir / "tractor_metrics.txt"
tractor_tests_file = reports_dir / "tractor_tests.txt"
tractor_test_logs_dir = reports_dir / "tractor_test_logs"

# Standard Makefile benchmarks, in execution/report order: test first, then A-Z.
# Some library projects (libcsv, rgba) supply standalone C test programs.
# Translate these executables along with their library sources so the same
# functional checks can be run in the C and Rust variants.
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
        "name": "libcsv",
        "path": benchmarks_dir / "libcsv",
        "kind": "binary",  # Library exercised via its check_csv C test executable.
        "autotools_fix_timestamps": True,
        "prepare_commands": (
            ("Autotools configuration", ["sh", "configure", "--disable-shared"]),
        ),
        # Build the CSV library before linking its test driver. Building
        # check_csv alone fails with "cannot find -lcsv" on a clean checkout.
        "make_target": ["libcsv.la", "check_csv"],
        "make_runs_binary": False,
        "executable": "check_csv",
        "main_module": "test_csv",  # Source filename, not the Makefile target name.
    },
    {
        "name": "libtree",
        "path": benchmarks_dir / "libtree",
        "kind": "binary",  # Despite its name, libtree is a command-line executable.
        "make_target": "libtree",
        "make_runs_binary": False,
        "executable": "libtree",
        "main_module": "libtree",
        "run_args": ["--help"],
    },
    {
        "name": "lil",
        "path": benchmarks_dir / "lil",
        "kind": "binary",
        "make_target": "lil",
        "make_runs_binary": False,
        "executable": "lil",
        "main_module": "main",
        "input_args": ["hello.lil"],  # Real smoke test; --help is interpreted as a file.
    },
    {
        "name": "rgba",
        "path": benchmarks_dir / "rgba",
        "kind": "binary",  # Library exercised via its assertion-based test.c.
        "make_target": "test",
        "clean_target": None,
        "make_runs_binary": False,
        "executable": "test",
        "build_only_rule": "test: test.c src/rgba.c\n\t$(CC) -std=c99 $^ -o $@\n",
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


def select_cases(argv: list[str] | None = None) -> tuple[list[dict], list, bool, bool]:
    """Select benchmarks using hierarchical TRACTOR overrides.

    --tractor > --tractor/BATTERY > --tractor/BATTERY/CASE.
    The closest explicitly specified value controls each TRACTOR case, even if
    its parent was set to false. The *least specific supplied* TRACTOR level
    sets the base selection: true selects only TRACTOR by default; false keeps
    the standard benchmarks and excludes that TRACTOR scope by default.

    Explicit positive standard selectors are always combinable with TRACTOR
    selectors and restrict the standard side to the named benchmarks.
    Repeating the same option with opposite values is still an error.
    """
    parser = argparse.ArgumentParser(
        description="Run standard and TRACTOR benchmarks (all enabled by default).",
        allow_abbrev=False,
        epilog=("Boolean options accept --OPTION, --OPTION=true, or "
                "--OPTION=false. TRACTOR selectors use --tractor/BATTERY/CASE "
                "and more specific selectors override their parents."),
    )

    def add_bool(flag: str, dest: str, help_text: str) -> None:
        parser.add_argument(flag, dest=dest, nargs="?", const="true",
                            choices=("true", "false"), default=None,
                            metavar="{true,false}", help=help_text)

    for config in BENCHMARKS:
        name = config["name"]
        add_bool(f"--{name}", f"standard_{name}",
                 f"Include {name}, or exclude it with --{name}=false")
    add_bool("--tractor", "tractor", "Select or exclude the TRACTOR corpus")
    add_bool("--tractor-metrics", "tractor_metrics", "Enable only TRACTOR metrics, or disable them")
    add_bool("--tractor-tests", "tractor_tests", "Enable only TRACTOR tests, or disable them")
    for bundle in BUNDLE_ORDER:
        add_bool(f"--tractor/{bundle}", f"bundle_{bundle}",
                 f"Select or exclude battery {bundle}; individual cases may override")

    # Case switches are discovered from the same allowlisted corpus as the run.
    # --help and pure standard selections remain usable without a corpus checkout.
    given = list(sys.argv[1:] if argv is None else argv)
    if "--help" in given or "-h" in given:
        parser.parse_args(given)
    preliminary, unknown = parser.parse_known_args(given)
    standard_positive = any(
        getattr(preliminary, f"standard_{b['name']}") == "true"
        for b in BENCHMARKS)
    positive_battery = any(
        getattr(preliminary, f"bundle_{bundle}") == "true"
        for bundle in BUNDLE_ORDER)

    # Only valid canonical battery prefixes may be followed by case names.
    # Catch misspellings such as --tractor/B01/organic/... immediately.
    position = 0
    while position < len(unknown):
        token = unknown[position]
        flag = token.partition("=")[0]
        if flag.startswith("--tractor/"):
            parts = flag[len("--tractor/"):].split("/")
            if len(parts) != 2 or parts[0] not in BUNDLE_ORDER or not parts[1]:
                parser.error(f"Unknown TRACTOR selector {flag!r}; expected "
                             "--tractor/BATTERY/CASE with a canonical battery name")
            if "=" not in token and position + 1 < len(unknown) and \
                    unknown[position + 1] in ("true", "false"):
                position += 1  # A separated value for a dynamically added case.
        else:
            parser.error(f"unrecognized argument: {token}")
        position += 1

    any_battery_selector = any(
        getattr(preliminary, f"bundle_{bundle}") is not None
        for bundle in BUNDLE_ORDER)
    if not unknown and ((preliminary.tractor == "false" and not positive_battery) or
                        (standard_positive and preliminary.tractor is None
                         and not any_battery_selector)):
        discovered = []
    else:
        try:
            discovered = discover_tractor_cases()
        except (OSError, FileNotFoundError) as exc:
            parser.error(str(exc))

    for index, case in enumerate(discovered):
        parser.add_argument(f"--tractor/{case.case_id}", dest=f"case_{index}",
                            nargs="?", const="true", choices=("true", "false"),
                            default=None, metavar="{true,false}", help=argparse.SUPPRESS)

    # argparse ordinarily keeps the last value of a repeated switch. Only a
    # contradiction at the *same* specificity for the *same* option is an error.
    encountered: dict[str, str] = {}
    for position, token in enumerate(given):
        flag, separator, inline = token.partition("=")
        if not flag.startswith("--"):
            continue
        action = parser._option_string_actions.get(flag)
        if action is None or not isinstance(action, argparse._StoreAction):
            continue
        if separator:
            value = inline
        elif position + 1 < len(given) and given[position + 1] in ("true", "false"):
            value = given[position + 1]
        else:
            value = "true"
        if value not in ("true", "false"):
            continue  # Leave invalid-value messages to argparse.
        previous = encountered.get(action.dest)
        if previous is not None and previous != value:
            parser.error(f"Contradiction: {flag} specified with both true and false")
        encountered[action.dest] = value
    args = parser.parse_args(given)

    def yes(value: str | None) -> bool:
        return value == "true"

    standard_flags = {b["name"]: getattr(args, f"standard_{b['name']}")
                      for b in BENCHMARKS}
    std_positive = {name for name, value in standard_flags.items() if yes(value)}
    std_negative = {name for name, value in standard_flags.items() if value == "false"}
    battery_flags = {bundle: getattr(args, f"bundle_{bundle}") for bundle in BUNDLE_ORDER}
    battery_positive = {bundle for bundle, value in battery_flags.items() if yes(value)}
    case_flags = {case.case_id: getattr(args, f"case_{index}")
                  for index, case in enumerate(discovered)}
    case_positive = {name for name, value in case_flags.items() if yes(value)}

    # The shallowest explicitly mentioned TRACTOR level controls the standard
    # benchmark defaults. A positive standard selector narrows that side even
    # when --tractor=false (it can still be combined with selected descendants).
    if args.tractor is not None:
        tractor_base_is_positive = yes(args.tractor)
    elif any(value is not None for value in battery_flags.values()):
        tractor_base_is_positive = bool(battery_positive)
    elif any(value is not None for value in case_flags.values()):
        tractor_base_is_positive = bool(case_positive)
    else:
        tractor_base_is_positive = False

    if std_positive:
        selected_standard = [b for b in BENCHMARKS if b["name"] in std_positive]
    elif tractor_base_is_positive:
        selected_standard = []
    else:
        selected_standard = [b for b in BENCHMARKS if b["name"] not in std_negative]

    # For an unspecified level, select everything unless an explicit true
    # selector at this level (or a higher level) establishes a restricted set.
    # Explicit battery and individual-case flags override the inherited value.
    if args.tractor is not None:
        default_tractor = yes(args.tractor)
    elif any(value is not None for value in battery_flags.values()):
        default_tractor = not bool(battery_positive)
    elif any(value is not None for value in case_flags.values()):
        default_tractor = not bool(case_positive)
    else:
        default_tractor = not bool(std_positive)

    cases = []
    for case in discovered:
        enabled = default_tractor
        battery_value = battery_flags[case.bundle]
        if battery_value is not None:
            enabled = yes(battery_value)
        case_value = case_flags[case.case_id]
        if case_value is not None:
            enabled = yes(case_value)
        if enabled:
            cases.append(case)

    tractor_positive = (yes(args.tractor) or bool(battery_positive) or bool(case_positive))
    if tractor_positive and not cases:
        parser.error("TRACTOR cases were explicitly selected, but none remain enabled")
    if not selected_standard and not cases:
        parser.error("No benchmarks selected")

    # Analysis switches configure selected TRACTOR cases; they do not select
    # benchmarks themselves. A single positive analysis switch limits the run
    # to that analysis unless the other was explicitly enabled too.
    metrics = args.tractor_metrics != "false"
    tests = args.tractor_tests != "false"
    if args.tractor_metrics == "true" and args.tractor_tests is None:
        tests = False
    if args.tractor_tests == "true" and args.tractor_metrics is None:
        metrics = False
    if cases and not (metrics or tests):
        parser.error("Contradiction: TRACTOR cases selected, but both "
                     "--tractor-metrics=false and --tractor-tests=false")
    if not cases and (yes(args.tractor_metrics) or yes(args.tractor_tests)):
        parser.error("Contradiction: TRACTOR analysis requested, but no TRACTOR cases selected")

    for config in selected_standard:
        if not config["path"].is_dir():
            parser.error(f"Benchmark source directory does not exist: {config['path']}")
    return selected_standard, cases, metrics, tests


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


def fix_autotools_timestamps(directory: Path) -> None:
    """Normalize archive mtimes only in the temporary copy of libcsv.

    Some ZIP archives contain files with future modification timestamps.
    Autotools then tries to regenerate old prebuilt scripts with aclocal-1.14,
    which need not be installed. Make the distributed source inputs older
    than the already supplied, generated Autotools scripts. No autoreconf.
    """
    baseline = time.time() - 120
    for path in directory.rglob("*"):
        if path.is_file() and not path.is_symlink():
            old = path.stat()
            if old.st_mtime > baseline:
                os.utime(path, (old.st_atime, baseline))
    for index, filename in enumerate(("aclocal.m4", "configure", "Makefile.in")):
        file = directory / filename
        if not file.is_file():
            raise RuntimeError(f"Missing pregenerated Autotools file: {file}")
        timestamp = baseline + 20 * (index + 1)
        os.utime(file, (timestamp, timestamp))
    print("  [OK] Autotools generated-script timestamps normalized", flush=True)


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

    # Project-specific preparation happens only in _temp, never in the inputs.
    if config.get("autotools_fix_timestamps"):
        fix_autotools_timestamps(input_dir)
    for stage, command in config.get("prepare_commands", ()):
        run_step(stage, command, cwd=input_dir)

    # Arguments referring to bundled test inputs must use the copied input
    # tree, because Cargo normally executes from the generated Rust project.
    run_args = [*config.get("run_args", ()),
                *(str(input_dir / path) for path in config.get("input_args", ()))]

    make_command = ["make", "-C", str(input_dir)]
    if config.get("build_only_rule"):
        # Add a tiny Makefile overlay under _temp. It changes no C source and
        # preserves all original prerequisites and compiler/linker flags.
        # Only the recipe running the test during the build is replaced.
        override = input_dir / "benchmark_build_only.mk"
        override.write_text(config["build_only_rule"], encoding="utf-8")
        make_command.extend(["-f", "Makefile", "-f", override.name])
    targets = config.get("make_target")
    if isinstance(targets, str):
        make_command.append(targets)
    elif targets is not None:
        make_command.extend(targets)
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
                                      *run_args], cwd=input_dir)
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
    cargo_args = (["--", *run_args] if kind == "binary" and run_args else [])
    def cargo_command(directory: Path) -> list[str]:
        return ["cargo", cargo_action, "--release", "--manifest-path",
                str(directory / "Cargo.toml"),
                *(["--quiet"] if kind == "binary" else []), *cargo_args]

    transpiled_output = run_step(
        "Transpiled Rust execution" if kind == "binary" else "Transpiled Rust release build",
        cargo_command(transpile_dir),
        cwd=transpile_dir,
        env=rust_env)

    shutil.copytree(transpile_dir, refactor_dir, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns("target"))
    print("  [OK] Refactoring copy created", flush=True)
    for pass_name in REFACTOR_COMMANDS:
        run_step(f"Refactoring: {pass_name}",
                 [str(c2rust_dir / "c2rust-refactor"), "--cargo", "--plugin-dir",
                  str(plugin_dir), "--plugin-name", "c2rust_idiomatizer",
                  "--rewrite-mode", "inplace", pass_name,
                  *(["transform-public-functions=true"] if pass_name == "idiomatize_loops" else [])],
                 cwd=refactor_dir)
    if not format_rust(refactor_dir, "Refactored Rust formatting"):
        warnings.append("Refactored Rust: formatting failed; compiled unformatted Rust")
    refactored_output = run_step(
        "Refactored Rust execution" if kind == "binary" else "Refactored Rust release build",
        cargo_command(refactor_dir),
        cwd=refactor_dir,
        env=rust_env)

    if kind == "binary":
        with outputs_file.open("a", encoding="utf-8") as file:
            file.write(f"=== Benchmark: {name} ({kind}) ===\n\n")
            for title, output in (("Original C execution output", original_output),
                                  ("Transpiled Rust execution output", transpiled_output),
                                  ("Refactored Rust execution output", refactored_output)):
                # An assertion-based test can succeed silently; record that
                # explicitly rather than leaving an apparently missing result.
                readable = output.rstrip() or "(exit status 0; no stdout)"
                file.write(f"--- {title} ---\n\n{readable}\n\n")

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
    header = f"{'Metric':35} {'Transpiled':>14} {'Refactored':>14}"
    lines = [f"=== Benchmark: {case['name']} ({case['kind']}) ===", "",
             header, "-" * len(header)]
    for key, label in METRIC_LABELS:
        lines.append(f"{label:35} {metric_value(case.get('transpiled'), key):>14} "
                     f"{metric_value(case.get('refactored'), key):>14}")

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


def totals_report(cases: list[dict]) -> str:
    """Single paired total: include a case only if both Rust metrics exist."""
    matched = [case for case in cases
               if case.get("transpiled") is not None and case.get("refactored") is not None]
    transpiled = sum_metrics(matched, "transpiled")
    refactored = sum_metrics(matched, "refactored")
    heading = "TRACTOR benchmarks — total"
    header = f"{'Metric':35} {'Transpiled':>14} {'Refactored':>14}"
    lines = [f"=== {heading} ===", "", header, "-" * len(header)]
    for key, label in METRIC_LABELS:
        lines.append(f"{label:35} {metric_value(transpiled, key):>14} "
                     f"{metric_value(refactored, key):>14}")
    excluded = len(cases) - len(matched)
    lines.extend(["", f"Cases included: {len(matched)} of {len(cases)}."])
    if excluded:
        lines.append(f"Note: {excluded} TRACTOR benchmark(s) excluded from this total "
                     "because not all required variants were analyzed.")
    return "\n".join(lines)


def save_results(cases: list[dict]) -> None:
    """Keep paired totals and partial per-case results after every case."""
    reports = [case_report(case) for case in cases]
    reports.append(totals_report(cases))
    tractor_metrics_file.write_text("\n\n".join(reports) + "\n", encoding="utf-8")


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


def process_case(case, analyzer_exe: Path | None, progress: dict, *,
                 evaluate_metrics: bool = True, evaluate_tests: bool = True) -> None:
    """Build one C/Rust pair; run independently enabled metrics and test vectors."""
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

    progress["stage"] = "CMake target selection"
    database, main_module, cmake_target, native_link_flags = select_cmake_compilation(
        build_dir, input_dir, case.kind,
    )
    progress["stage"] = "C build (selected CMake target)"
    run_step(progress["stage"], ["cmake", "--build", str(build_dir),
                                  "--target", cmake_target])

    if evaluate_tests:
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

    if evaluate_metrics:
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

    if evaluate_tests:
        test_variant("transpiled", "Transpiled Rust", transpile_dir)
    if evaluate_metrics:
        progress["stage"] = "Transpiled Rust analysis"
        baseline = analyze_rust_project(transpile_dir, analyzer_exe)
        baseline["for_loop_ratio"] = (baseline["for_loops"], c_for_loops)
        progress["transpiled"] = baseline
        print("  [OK] Transpiled Rust analysis", flush=True)

    # Only the transform-public-functions=true configuration is benchmarked.
    prefix = "Refactored Rust"
    directory = work_dir / "out" / "refactor"
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
            if refactor_command == "idiomatize_loops":
                command.append("transform-public-functions=true")
            run_step(progress["stage"], command, cwd=directory)

        progress["stage"] = f"{prefix}: formatting"
        if not format_rust(directory, progress["stage"]):
            progress.setdefault("warnings", []).append(
                f"{prefix}: formatting failed; compiled unformatted Rust")
        progress["stage"] = f"{prefix}: release build"
        run_step(progress["stage"], ["cargo", "build", "--release", "--manifest-path",
                                      str(directory / "Cargo.toml")],
                 cwd=directory, env=rust_env)
        if evaluate_tests:
            test_variant("refactored", prefix, directory)
        if evaluate_metrics:
            progress["stage"] = f"{prefix}: analysis"
            metrics = analyze_rust_project(directory, analyzer_exe)
            metrics["for_loop_ratio"] = (metrics["for_loops"], c_for_loops)
            progress["refactored"] = metrics
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
    standards, cases, tractor_metrics, tractor_tests = select_cases()
    try:
        set_stack_limit()
        ensure_release_tools()
        analyzer = None
        if standards or (cases and tractor_metrics):
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
            # Remove stale reports from an earlier run, even if this run disables
            # one of the two analyses. Never delete an old report before cases
            # have actually been selected.
            tractor_metrics_file.unlink(missing_ok=True)
            tractor_tests_file.unlink(missing_ok=True)
            if tractor_tests:
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
                process_case(case, analyzer, progress,
                             evaluate_metrics=tractor_metrics, evaluate_tests=tractor_tests)
            except Exception as exc:
                # A single failing TRACTOR case must not terminate the batch.
                error = exc
                stage = exc.stage if isinstance(exc, StepError) else progress.get("stage", "Setup")
                print(f"  [FAILED] tractor/{case.case_id}: {stage}", flush=True)

            test_results = progress.get("vector_tests", {})
            tests_failed = tractor_tests and (
                len(test_results) != 3 or
                any(result.status != "complete" for result in test_results.values()))
            failed = error is not None or bool(progress.get("errors")) or tests_failed
            failures += bool(failed)
            collected.append({
                "name": f"tractor/{case.case_id}",
                "kind": case.kind,
                **progress,
                "error": error,
            })
            if tractor_metrics:
                save_results(collected)
            if tractor_tests:
                tractor_tests_file.write_text(summarize_tests(collected), encoding="utf-8")

        if standards:
            print(f"\nStandard results saved in: {results_file}")
            print(f"Standard benchmarks processed: {len(standards)}")
        if cases:
            if tractor_metrics:
                print(f"\nTRACTOR metrics saved in: {tractor_metrics_file}")
            if tractor_tests:
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
