#!/usr/bin/env python3
"""Evaluate a compiled Rust Cargo project with pinned rustc."""

import os
import subprocess
from functools import lru_cache
import tempfile
from collections import defaultdict
from pathlib import Path

NIGHTLY = "nightly-2022-11-03"
METRICS = (
    "characters",
    "c_type_occurrences",
    "casts",
    "for_loops",
    "raw_pointer_parameters",
    "raw_pointer_arithmetic",
    "raw_pointer_dereferences",
    "indexing_operations",
)


class RustAnalysisError(RuntimeError):
    pass


@lru_cache(maxsize=1)
def _nightly_sysroot() -> Path:
    """Discover the pinned rustc sysroot containing its standard library."""
    command = ["rustc", f"+{NIGHTLY}", "--print", "sysroot"]
    try:
        result = subprocess.run(
            command, text=True, capture_output=True, check=True
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        raise RustAnalysisError(
            f"Could not discover Rust sysroot using {' '.join(command)}: {exc}"
        ) from exc
    sysroot = Path(result.stdout.strip()).resolve()
    if not (sysroot / "lib" / "rustlib").is_dir():
        raise RustAnalysisError(
            f"Rust toolchain sysroot is missing standard-library directories: {sysroot}"
        )
    return sysroot


def _nightly_library_directory() -> Path:
    """Find LLVM/rustc shared libraries in the pinned sysroot."""
    return _nightly_sysroot() / "lib"


def plain_tool_env() -> dict[str, str]:
    """Do not let analyzer-specific linker settings leak into C2Rust."""
    env = os.environ.copy()
    env.pop("RUSTFLAGS", None)
    env.pop("RUSTC_WRAPPER", None)
    env.pop("RUSTC_WORKSPACE_WRAPPER", None)

    # Also tolerate an invocation using the old, shell-wide workaround:
    # LD_LIBRARY_PATH="$SYSROOT/lib:..." python3 benchmarking/runner/run.py
    library_dir = str(_nightly_library_directory())
    existing = env.get("LD_LIBRARY_PATH")
    if existing is not None:
        remaining = [p for p in existing.split(os.pathsep) if p != library_dir]
        if remaining:
            env["LD_LIBRARY_PATH"] = os.pathsep.join(remaining)
        else:
            env.pop("LD_LIBRARY_PATH", None)
    return env


def _analyzer_env(*, linking: bool) -> dict[str, str]:
    """Add LLVM linking/runtime paths only to analyzer subprocesses."""
    env = plain_tool_env()
    library_dir = str(_nightly_library_directory())
    if linking:
        env["RUSTFLAGS"] = f"-L native={library_dir}"
    original = env.get("LD_LIBRARY_PATH")
    env["LD_LIBRARY_PATH"] = (
        library_dir + (os.pathsep + original if original else "")
    )
    return env


def _execute(command: list[str], cwd: Path, env: dict) -> str:
    """Capture output without writing logs; include it all on failure."""
    try:
        result = subprocess.run(
            command, cwd=cwd, env=env, text=True, errors="replace",
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
        )
    except OSError as exc:
        raise RustAnalysisError(f"Could not start {command[0]}: {exc}") from exc
    if result.returncode != 0:
        raise RustAnalysisError(
            f"{' '.join(command)} exited with {result.returncode}:\n"
            f"{result.stdout.rstrip()}"
        )
    return result.stdout


def _under_project(filename: str, root: Path) -> bool:
    if not filename or filename.startswith("<"):
        return False
    path = Path(filename)
    if not path.is_absolute():
        path = root / path
    try:
        relative = path.resolve().relative_to(root.resolve())
    except ValueError:
        return False
    return bool(relative.parts) and "target" not in relative.parts and path.suffix == ".rs"


def analyze_rust_project(project: Path, analyzer_exe: Path) -> dict:
    """Analyze a generated Rust project without leaving analysis artifacts."""
    project = project.resolve()
    manifest = str(project / "Cargo.toml")
    # Keep the analyzer's TSV files under the generated project's target/
    # (which is itself inside runner/_temp/out/); remove after reading them.
    (project / "target").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="rust_metrics_", dir=project / "target") as directory:
        report_dir = Path(directory)
        wrapper_env = _analyzer_env(linking=False)
        wrapper_env["RUSTC_WORKSPACE_WRAPPER"] = str(analyzer_exe.resolve())
        wrapper_env["RUST_METRICS_OUTPUT_DIR"] = str(report_dir.resolve())
        wrapper_env["RUST_METRICS_SYSROOT"] = str(_nightly_sysroot())
        _execute(
            ["cargo", f"+{NIGHTLY}", "check", "--manifest-path", manifest,
             "--jobs", "1"],
            project, wrapper_env,
        )
        outputs = sorted(report_dir.glob("*.tsv"))
        if not outputs:
            raise RustAnalysisError(
                f"rustc completed without producing analysis output in {report_dir}. "
                "Try 'cargo clean' for this generated project and rerun."
            )

        source_chars: dict[Path, int] = {}
        observed: dict[str, set[tuple[Path, int, int]]] = defaultdict(set)
        for path in outputs:
            for line in path.read_text(encoding="utf-8").splitlines():
                fields = line.split("\t")
                if len(fields) == 3 and fields[0] == "F":
                    filename = fields[1]
                    if _under_project(filename, project):
                        source_chars[Path(filename).resolve()] = int(fields[2])
                elif len(fields) == 7 and fields[0] == "E":
                    _, metric, filename, start_line, start_col, end_line, end_col = fields
                    if metric not in METRICS:
                        raise RustAnalysisError(f"Unexpected metric in {path}: {metric}")
                    if _under_project(filename, project):
                        observed[metric].add((
                            Path(filename).resolve(),
                            int(start_line), int(start_col), int(end_line), int(end_col),
                        ))
                else:
                    raise RustAnalysisError(f"Malformed analyzer output in {path}: {line!r}")

    if not source_chars:
        raise RustAnalysisError("No project Rust source files observed by rustc")

    results = {
        metric: len(observed[metric]) for metric in METRICS
        if metric != "characters"
    }
    results["characters"] = sum(source_chars.values())
    return results


def build_rust_analyzer(source: Path) -> Path:
    """Compile the rustc_private analyzer with the pinned nightly."""
    _execute(
        ["cargo", f"+{NIGHTLY}", "build", "--manifest-path", str(source / "Cargo.toml")],
        source, _analyzer_env(linking=True),
    )
    exe = source / "target" / "debug" / "rust_metrics"
    if not exe.is_file():
        raise RustAnalysisError(f"Rust analyzer executable missing: {exe}")
    return exe


def format_summary(results: dict[str, dict]) -> str:
    """Return exactly the metric tables to print and save in _results.txt."""
    labels = [
        ("characters", "Characters of code"),
        ("c_type_occurrences", "C FFI type occurrences"),
        ("casts", "Casts"),
        ("for_loop_ratio", "For-loop ratio"),
        ("raw_pointer_parameters", "Raw pointer parameters"),
        ("raw_pointer_arithmetic", "Raw pointer arithmetic operations"),
        ("raw_pointer_dereferences", "Raw pointer dereferences"),
        ("indexing_operations", "Indexing operations"),
    ]
    lines = []
    for name, versions in results.items():
        if lines:
            lines.append("")
        lines.append(f"=== Benchmark: {name} ===")
        lines.append("")
        lines.append(f"{'Metric':35} {'Transpiled':>14} {'Refactored':>14}")
        lines.append("-" * 66)
        for metric, label in labels:
            a, b = (versions[k][metric] for k in ("transpiled", "refactored"))
            def fmt(value):
                if metric == "for_loop_ratio":
                    rust_loops, c_loops = value
                    return f"{rust_loops}/{c_loops}"
                return f"{value:,}"
            lines.append(f"{label:35} {fmt(a):>14} {fmt(b):>14}")
    return "\n".join(lines)
