"""Run TRACTOR's official Cando test vectors and read its JUnit report.

The corpus runner (Nix + Docker) is responsible for setup scripts, filesystem
changes, library FFI runners, UB skips, and output comparison. Reimplementing
only its JSON stdout comparisons would give misleading pass counts.
"""

from __future__ import annotations

import os
import re
import tomllib
import shutil
import subprocess
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path

from tractor_integration import BUNDLE_ORDER


TOTALS_BANNER = "\n".join(("=" * 80, "-" * 36 + " TOTALS " + "-" * 36, "=" * 80))


# Per Python benchmark-runner process: the official Nix launcher only needs
# to import exec_test_vector:latest once, not for every C/Rust variant.
_IMAGE_READY = False


@dataclass
class VectorResults:
    passed: int = 0
    failed: int = 0
    skipped: int = 0
    status: str = "not run"  # complete, failed, or not run
    reason: str = ""
    details: list[str] = field(default_factory=list)


def _corpus_root(case) -> Path:
    for ancestor in case.case_dir.parents:
        if ancestor.name == "Public-Tests":
            return ancestor.parent
    raise ValueError(f"TRACTOR case is not under Public-Tests: {case.case_dir}")


def _read_junit(report: Path, case_id: str) -> VectorResults:
    """Count only test vectors, not the corpus runner's setup/build phases."""
    root = ET.parse(report).getroot()
    if root.tag not in ("testsuite", "testsuites"):
        raise ValueError(f"Unexpected TRACTOR JUnit root: {root.tag}")
    suites = [root] if root.tag == "testsuite" else list(root.iter("testsuite"))
    expected = "Public-Tests/" + case_id
    matching = [suite for suite in suites
                if suite.attrib.get("name", "").rstrip("/") == expected]
    if len(matching) != 1:
        raise ValueError(f"Expected exactly one JUnit suite {expected!r}; found {len(matching)}")

    result = VectorResults(status="complete")
    for test in matching[0].findall("testcase"):
        name = test.attrib.get("name", "<unnamed>")
        if name in {"config", "build", "build-runners"}:
            # These are infrastructure tests, not TRACTOR test vectors.
            problem = test.find("failure")
            if problem is None:
                problem = test.find("error")
            if problem is not None:
                result.status = "failed"
                result.reason = f"TRACTOR {name} phase failed: {problem.attrib.get('message', '')}"
            continue
        skip = test.find("skipped")
        failure = test.find("failure")
        error = test.find("error")
        if skip is not None:
            result.skipped += 1
            msg = skip.attrib.get("message", "")
            result.details.append(f"SKIPPED {name}: {msg}")
        elif failure is not None or error is not None:
            result.failed += 1
            node = failure if failure is not None else error
            msg = node.attrib.get("message", "") or (node.text or "").strip()
            result.details.append(f"FAILED {name}: {msg[:500]}")
        else:
            result.passed += 1
    if result.passed + result.failed + result.skipped == 0:
        raise ValueError("TRACTOR JUnit suite contains no test vectors")
    if result.failed:
        result.status = "failed"
    return result


def _test_prerequisites() -> str | None:
    missing = [tool for tool in ("nix", "docker") if shutil.which(tool) is None]
    if missing:
        return "Missing TRACTOR test dependencies: " + ", ".join(missing) + " (requires Nix and Docker)"
    return None




def _tractor_library_name(case) -> str:
    """Read Cando's declared library name, defaulting to the case name.

    E.g. TRACTOR's 001_helloworld_lib runner declares `library: "hello"`.
    The CMake target/filename is then `libhello.so`, not the directory name.
    """
    source_dir = case.case_dir / "runner" / "src"
    names: set[str] = set()
    if source_dir.is_dir():
        for source in source_dir.rglob("*.rs"):
            contents = source.read_text(encoding="utf-8")
            names.update(re.findall(r'(?m)^\s*library\s*:\s*"([^"\n]+)"', contents))
    if len(names) > 1:
        raise ValueError(f"Ambiguous TRACTOR Cando library names for {case.case_id}: {sorted(names)}")
    return next(iter(names)) if names else case.case_id.rsplit("/", 1)[-1]


def _prepare_tractor_library(manifest: Path, library_name: str) -> None:
    """Make the *staged copy* expose the .so expected by TRACTOR's FFI runner.

    C2Rust emits a Cargo package typically named `transpile`, which may build
    only an rlib.  TRACTOR's library harness loads lib<test_case_name>.so.
    Only Cargo target metadata changes; the original project is untouched.
    """
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", library_name):
        raise ValueError(f"Unexpected TRACTOR library name: {library_name!r}")
    source = manifest.read_text(encoding="utf-8")
    head = re.search(r"(?m)^\[lib\][ \t]*(?:#.*)?$", source)
    settings = f'name = "{library_name}"\ncrate-type = ["cdylib"]\n'
    if head is None:
        # Cargo defaults to src/lib.rs, but C2Rust can emit lib.rs at root.
        if (manifest.parent / "src" / "lib.rs").is_file():
            path_setting = ""
        elif (manifest.parent / "lib.rs").is_file():
            path_setting = 'path = "lib.rs"\n'
        else:
            raise ValueError(f"Cannot find a library crate root beside {manifest}")
        updated = source.rstrip() + "\n\n[lib]\n" + settings + path_setting
    else:
        # Replace name/crate-type only inside [lib], preserving path and all
        # other Cargo settings (dependencies, edition, targets, etc.).
        next_section = re.search(r"(?m)^\[", source[head.end():])
        end = head.end() + next_section.start() if next_section else len(source)
        section = source[head.end():end]
        for key in ("name", "crate-type"):
            section = re.sub(rf"(?m)^[ \t]*{key}[ \t]*=[^\n]*(?:\n|$)", "", section)
        updated = source[:head.end()] + "\n" + settings + section.lstrip("\n") + source[end:]
    # Refuse to run with an invalid Cargo manifest rather than count a setup
    # failure as a behavioral failure. This does not validate rustc linking.
    tomllib.loads(updated)
    manifest.write_text(updated, encoding="utf-8")

def _prepare_tractor_binary(manifest: Path, binary_name: str,
                            main_module: str | None = None) -> None:
    """Expose the CMake executable name from the staged Cargo project.

    C2Rust commonly creates a single binary named ``main`` or ``transpile``.
    TRACTOR's executable runner instead looks for the CMake executable name,
    e.g. ``target/release/driver``. Change only staged Cargo target metadata;
    never rename generated Rust source or the actual benchmark output.
    """
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_-]*", binary_name):
        raise ValueError(f"Unexpected TRACTOR executable name: {binary_name!r}")
    source = manifest.read_text(encoding="utf-8")
    parsed = tomllib.loads(source)
    bins = parsed.get("bin", [])
    headers = list(re.finditer(r"(?m)^\[\[bin\]\][ \t]*(?:#.*)?$", source))
    if len(bins) != len(headers):
        raise ValueError(f"Cannot locate Cargo [[bin]] sections in {manifest}")

    if bins:
        matches = [i for i, entry in enumerate(bins)
                   if entry.get("name") == binary_name]
        if matches:
            # The expected executable already has a matching Cargo target.
            return
        if len(bins) == 1:
            index = 0
        else:
            candidates = [i for i, entry in enumerate(bins)
                          if main_module and (
                              entry.get("name") == main_module or
                              Path(entry.get("path", "")).stem == main_module)]
            if len(candidates) != 1:
                raise ValueError(
                    f"Cannot identify executable {binary_name!r} among Cargo "
                    f"binary targets: {[entry.get('name') for entry in bins]}")
            index = candidates[0]
        start = headers[index].end()
        next_section = re.search(r"(?m)^\[", source[start:])
        end = start + next_section.start() if next_section else len(source)
        section = source[start:end]
        key = re.search(r"(?m)^([ \t]*name[ \t]*=[ \t]*)[^\n]*", section)
        if key:
            section = section[:key.start()] + key.group(1) + f'"{binary_name}"' + section[key.end():]
        else:
            section = f'\nname = "{binary_name}"' + section
        updated = source[:start] + section + source[end:]
    else:
        # An implicit Cargo binary target is normally src/main.rs. C2Rust can
        # also emit main.rs at the crate root or use the source module name.
        paths = ["src/main.rs", "main.rs"]
        if main_module:
            paths.extend([f"src/bin/{main_module}.rs", f"{main_module}.rs"])
        candidates = [path for path in dict.fromkeys(paths)
                      if (manifest.parent / path).is_file()]
        if len(candidates) != 1:
            raise ValueError(
                f"Cannot identify one Cargo executable entrypoint for "
                f"{binary_name!r} in {manifest.parent}: {candidates}")
        updated = (source.rstrip() + "\n\n[[bin]]\n" +
                   f'name = "{binary_name}"\npath = "{candidates[0]}"\n')

    check = tomllib.loads(updated)
    if not any(entry.get("name") == binary_name for entry in check.get("bin", [])):
        raise ValueError(f"Failed to name staged Cargo executable {binary_name!r}")
    manifest.write_text(updated, encoding="utf-8")


def run_tractor_vectors(case, variant: str, rust_project: Path | None,
                        report_dir: Path, *, binary_name: str | None = None,
                        main_module: str | None = None) -> VectorResults:
    """Evaluate one C/Rust variant, preserving the original corpus checkout.

    ``variant`` is one of c, transpiled, refactored.
    Rust translations are temporarily copied into the corpus's documented
    translated_rust/ location (never overwrite an existing one).
    """
    report_dir.mkdir(parents=True, exist_ok=True)
    junit = report_dir / f"{variant}.xml"
    log = report_dir / f"{variant}.log"
    junit.unlink(missing_ok=True)
    missing = _test_prerequisites()
    if missing:
        return VectorResults(status="not run", reason=missing)

    corpus = _corpus_root(case)
    if not (corpus / "tools" / "test_runner").is_dir():
        return VectorResults(status="not run", reason="Official TRACTOR tools/test_runner not found")
    translation = case.case_dir / "translated_rust"
    if rust_project is not None and (translation.exists() or translation.is_symlink()):
        return VectorResults(status="not run", reason=f"Refusing to overwrite {translation}")

    # The upstream Nix app unconditionally runs `docker load` before launching
    # the Python orchestrator. Use that official app for the first variant in
    # this process, then run the *same* Python orchestrator in the flake's
    # devShell (same pyEnv, runtime tools, and exported build environment).
    # This avoids repeatedly importing a multi-GiB Docker image while keeping
    # the corpus's functional tests and JUnit reports unchanged.
    global _IMAGE_READY
    args = ["--subset", f"Public-Tests/{case.case_id}",
            "--junit-xml", str(junit.resolve()),
            "--jobs", "1", "--keep-going"]
    if rust_project is not None:
        args.append("--rust")
    if _IMAGE_READY:
        command = ["nix", "develop", "./tools/test_runner", "--command",
                   "python3", "-m", "runtests.orchestrator", *args]
    else:
        command = ["nix", "run", "./tools/test_runner", "--", *args]
    env = os.environ.copy()
    # Nix itself requires flakes and nix-command; don't change the user's config.
    env.pop("RUSTC_WRAPPER", None)
    env.pop("RUSTC_WORKSPACE_WRAPPER", None)
    env.pop("RUSTFLAGS", None)

    staged = False
    try:
        if rust_project is not None:
            if not (rust_project / "Cargo.toml").is_file():
                return VectorResults(status="not run", reason=f"Missing Rust manifest: {rust_project}")
            # Set before copying, so a partial copy is also cleaned up.
            staged = True
            shutil.copytree(rust_project, translation,
                            ignore=shutil.ignore_patterns("target", ".git"))
            if case.kind == "library":
                _prepare_tractor_library(translation / "Cargo.toml",
                                         _tractor_library_name(case))
            elif case.kind == "binary":
                if binary_name is None:
                    raise ValueError(f"Missing selected CMake executable name for {case.case_id}")
                _prepare_tractor_binary(translation / "Cargo.toml",
                                        binary_name, main_module)
        with log.open("w", encoding="utf-8") as stream:
            stream.write("Command: " + " ".join(command) + "\n\n")
            stream.flush()
            try:
                completed = subprocess.run(command, cwd=corpus, env=env,
                                           stdout=stream, stderr=subprocess.STDOUT,
                                           timeout=3600, check=False)
                exit_code = completed.returncode
            except subprocess.TimeoutExpired:
                return VectorResults(status="not run", reason="TRACTOR runner timed out after 3600s")
            except OSError as exc:
                return VectorResults(status="not run", reason=f"Could not launch TRACTOR runner: {exc}")
    except (OSError, ValueError) as exc:
        return VectorResults(status="not run", reason=f"TRACTOR test setup failed: {exc}")
    finally:
        if staged and (translation.exists() or translation.is_symlink()):
            if translation.is_symlink():
                translation.unlink()
            else:
                shutil.rmtree(translation)

    # Only switch to the faster path after the upstream launcher explicitly
    # confirms that `docker load` completed successfully. An unsuccessful
    # first launch may be retried on the next vector.
    if not _IMAGE_READY and command[1] == "run":
        try:
            with log.open("r", encoding="utf-8", errors="replace") as stream:
                _IMAGE_READY = any(
                    line.strip() == "Loaded image: exec_test_vector:latest"
                    for _, line in zip(range(60), stream)
                )
        except OSError:
            pass

    if not junit.is_file():
        return VectorResults(status="not run",
                             reason=f"TRACTOR runner exited {exit_code} without JUnit report; see {log}")
    try:
        result = _read_junit(junit, case.case_id)
    except (OSError, ValueError, ET.ParseError) as exc:
        return VectorResults(status="not run", reason=f"Cannot interpret TRACTOR JUnit: {exc}; see {log}")
    if exit_code and result.status == "complete":
        result.status = "failed"
        result.reason = f"TRACTOR runner exited {exit_code}; see {log}"
    return result


def summarize_tests(cases: list[dict]) -> str:
    """Individual reports plus paired vector totals per battery and overall.

    Only cases with readable reports for all three variants enter a total;
    missing variants are excluded from their battery and the overall total.
    """
    variants = (("c", "Original C"), ("transpiled", "Transpiled"),
                ("refactored", "Refactored"))
    header = f"{'Test vectors':20}" + "".join(f" {label:>21}" for _, label in variants)
    fields = (("Passed", "passed"), ("Failed", "failed"), ("Skipped", "skipped"))
    lines = []
    for case in cases:
        lines.extend([f"=== Benchmark: {case['name']} ({case['kind']}) ===", "",
                      header, "-" * len(header)])
        tests = case.get("vector_tests", {})
        for label, field_name in fields:
            lines.append(f"{label:20}" + "".join(
                f" {str(getattr(tests[key], field_name)) if key in tests and tests[key].status != 'not run' else '—':>21}"
                for key, _ in variants))
        details = []
        for key, label in variants:
            result = tests.get(key)
            if result is None or result.status == "not run":
                details.append(f"Not run ({label}): {result.reason if result else 'Build/translation did not complete'}")
            else:
                if result.reason:
                    details.append(f"{label}: {result.reason}")
                details.extend(f"{label}: {message}" for message in result.details)
        if details:
            lines.append("")
            lines.extend(details)
        lines.append("")

    def totals(subset: list[dict], heading: str) -> list[str]:
        counts = {key: {field: 0 for _, field in fields} for key, _ in variants}
        included = []
        excluded = []
        for case in subset:
            tests = case.get("vector_tests", {})
            if all(key in tests and tests[key].status != "not run" for key, _ in variants):
                included.append(case)
                for key, _ in variants:
                    for _, field in fields:
                        counts[key][field] += getattr(tests[key], field)
            else:
                excluded.append(case)
        result = [f"=== {heading} ===", "", header, "-" * len(header)]
        for label, field in fields:
            result.append(f"{label:20}" + "".join(
                f" {counts[key][field]:>21}" for key, _ in variants))
        result.extend(["", f"Cases included: {len(included)} of {len(subset)}"])
        if excluded:
            result.append(f"{len(excluded)} cases excluded:")
            result.extend(f"- {case['name'].removeprefix('tractor/')}" for case in excluded)
        return result

    # One divider between the final individual report and the first totals.
    # The final case already ends with one empty line; add one more so
    # the banner has exactly two empty lines on either side.
    if cases:
        lines.append("")
    lines.extend(TOTALS_BANNER.splitlines())
    lines.extend(("", ""))

    for battery in BUNDLE_ORDER:
        subset = [case for case in cases
                  if case.get("bundle", case["name"].split("/")[1]) == battery]
        if subset:
            lines.extend(totals(subset, f"TRACTOR {battery} test vectors — totals"))
            lines.append("")
    lines.extend(totals(cases, "TRACTOR test vectors — totals"))
    return "\n".join(lines) + "\n"
