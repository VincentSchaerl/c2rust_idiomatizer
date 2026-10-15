#!/usr/bin/env python3
"""Audit the CMake targets of the supported TRACTOR cases without transpiling.

Run from any directory, e.g.:
  python3 benchmarking/runner/audit_tractor_targets.py
  python3 benchmarking/runner/audit_tractor_targets.py --case B01_synthetic/043_iso646_and_digraphs

Successful selections are automatic: only genuinely ambiguous cases need an
explicit entry in run.py's TRACTOR_TARGET_OVERRIDES map, after manual review.
"""

import argparse
import contextlib
import io
import subprocess
import tempfile
from pathlib import Path

from cmake_targets import request_codemodel, select_cmake_compilation
from tractor_integration import discover_tractor_cases


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--case", action="append", default=[], metavar="BUNDLE/CASE",
                        help="Audit only this supported case; repeatable")
    args = parser.parse_args()

    cases = discover_tractor_cases()
    known = {case.case_id for case in cases}
    unknown = sorted(set(args.case) - known)
    if unknown:
        parser.error("Unknown or unsupported case(s): " + ", ".join(unknown))
    if args.case:
        selected = set(args.case)
        cases = [case for case in cases if case.case_id in selected]

    failed = 0
    for case in cases:
        try:
            with tempfile.TemporaryDirectory(prefix="tractor_target_audit_") as tmp:
                build_dir = Path(tmp) / "build"
                request_codemodel(build_dir)
                result = subprocess.run(
                    ["cmake", "-S", str(case.c_source_dir), "-B", str(build_dir),
                     "-G", "Ninja", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                     "-DCMAKE_C_COMPILER=clang"],
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                    text=True, errors="replace", timeout=90, check=False,
                )
                if result.returncode:
                    raise RuntimeError(f"CMake configuration failed: {result.stdout.strip()[-350:]}")
                # select_cmake_compilation already verifies that one target and,
                # for binaries, the defining translation unit of main() exist.
                with contextlib.redirect_stdout(io.StringIO()):
                    _database, main_module, target, _flags = select_cmake_compilation(
                        build_dir, case.c_source_dir, case.kind,
                    )
                suffix = f", main module: {main_module}" if main_module else ""
                print(f"[OK] {case.case_id} -> CMake target: {target}{suffix}", flush=True)
        except Exception as exc:
            # A single ambiguous/failed CMake project should not block the audit.
            failed += 1
            print(f"[REVIEW] {case.case_id}: {exc}", flush=True)

    print(f"\nAudited: {len(cases)}, needs review: {failed}")
    if failed:
        print("Only assign explicit targets after inspecting these cases' CMake files.")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
