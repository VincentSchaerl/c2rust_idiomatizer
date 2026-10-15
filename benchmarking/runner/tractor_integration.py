#!/usr/bin/env python3
"""Discover executable and library tests from the DARPA TRACTOR public corpus.

This module only discovers cases. It does not build, translate, or execute them.
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


BENCHMARKS_DIR = Path(__file__).resolve().parents[1] / "benchmarks"
DEFAULT_CORPUS_DIR = BENCHMARKS_DIR / "PUBLIC-Test-Corpus"

# Only these batteries are part of the idiomatizer benchmark, in this order.
BUNDLE_ORDER = (
    "B01_organic",
    "B01_synthetic",
    "B02_organic",
    "B02_synthetic",
    "P00_perlin_noise",
)


@dataclass(frozen=True)
class TractorCase:
    case_id: str             # Unique across bundles, e.g. B01_synthetic/001_hello
    name: str                # Test case folder name
    bundle: str              # e.g. B01_synthetic
    case_dir: Path           # Contains test_case/ and test_vectors/
    c_source_dir: Path       # Directory passed to C2Rust
    test_vectors_dir: Path   # Kept separate from translator input
    kind: str                # "binary" or "library"


def discover_tractor_cases(
    corpus_dir: Path = DEFAULT_CORPUS_DIR,
) -> list[TractorCase]:
    """Find cases in the five supported batteries, in configured order."""
    public_dir = corpus_dir / "Public-Tests"
    if not public_dir.is_dir():
        raise FileNotFoundError(
            f"TRACTOR Public-Tests directory not found: {public_dir}\n"
            "Initialize the PUBLIC-Test-Corpus Git submodule first."
        )

    cases: list[TractorCase] = []
    for c_source_dir in sorted(public_dir.rglob("test_case")):
        if not c_source_dir.is_dir():
            continue

        case_dir = c_source_dir.parent
        relative = case_dir.relative_to(public_dir)
        if not relative.parts or relative.parts[0] not in BUNDLE_ORDER:
            continue
        test_vectors_dir = case_dir / "test_vectors"
        if not test_vectors_dir.is_dir():
            continue

        cases.append(
            TractorCase(
                case_id=relative.as_posix(),
                name=case_dir.name,
                bundle=relative.parts[0],
                case_dir=case_dir,
                c_source_dir=c_source_dir,
                test_vectors_dir=test_vectors_dir,
                kind="library" if case_dir.name.endswith("_lib") else "binary",
            )
        )

    return sorted(cases, key=lambda case: (BUNDLE_ORDER.index(case.bundle), case.case_id))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--corpus-dir", type=Path, default=DEFAULT_CORPUS_DIR,
        help="Root of the PUBLIC-Test-Corpus Git submodule",
    )
    parser.add_argument(
        "--bundle", help="Optional bundle filter, e.g. B01_synthetic",
    )
    parser.add_argument(
        "--list", action="store_true", help="Print each discovered case ID",
    )
    args = parser.parse_args()

    cases = discover_tractor_cases(args.corpus_dir)
    if args.bundle:
        cases = [case for case in cases if case.bundle == args.bundle]

    print(f"TRACTOR cases: {len(cases)} (executables: {sum(c.kind == 'binary' for c in cases)}, libraries: {sum(c.kind == 'library' for c in cases)})")
    for bundle, count in sorted(Counter(c.bundle for c in cases).items(),
                                key=lambda item: BUNDLE_ORDER.index(item[0])):
        print(f"  {bundle}: {count}")

    if args.list:
        for case in cases:
            print(case.case_id)


if __name__ == "__main__":
    main()
