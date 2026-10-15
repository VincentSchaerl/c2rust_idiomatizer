"""Select CMake translation units for exactly one executable or library target.

CMake's global compile_commands.json includes *all* targets, which may contain
mutually exclusive implementations of the same exported C symbol. Use the
CMake File API target dependency graph to choose the requested target and
its dependencies, without modifying C sources or generated Rust code.
"""

from __future__ import annotations

import json
import re
import shlex
from pathlib import Path


class CMakeTargetError(RuntimeError):
    """CMake does not identify a unique and trustworthy translation target."""


def request_codemodel(build_dir: Path) -> None:
    """Call before `cmake -S ... -B ...` to request the CMake File API reply."""
    query_dir = build_dir / ".cmake" / "api" / "v1" / "query"
    query_dir.mkdir(parents=True, exist_ok=True)
    (query_dir / "codemodel-v2").touch()


def _codemodel(build_dir: Path) -> dict[str, dict]:
    reply_dir = build_dir / ".cmake" / "api" / "v1" / "reply"
    indices = sorted(reply_dir.glob("index-*.json"))
    if not indices:
        raise CMakeTargetError("CMake did not produce its requested target metadata")
    index = json.loads(indices[-1].read_text(encoding="utf-8"))
    model_ref = index.get("reply", {}).get("codemodel-v2")
    if not model_ref:
        raise CMakeTargetError("CMake codemodel-v2 reply is missing")
    model = json.loads((reply_dir / model_ref["jsonFile"]).read_text(encoding="utf-8"))
    configurations = model.get("configurations", [])
    if len(configurations) != 1:
        raise CMakeTargetError("Expected one CMake configuration (Ninja generator)")
    targets = {}
    for ref in configurations[0].get("targets", []):
        detail = json.loads((reply_dir / ref["jsonFile"]).read_text(encoding="utf-8"))
        targets[ref["id"]] = detail
    return targets


def _main_module(target: dict, source_dir: Path) -> str:
    """Locate the C translation unit defining main(), not a guessed 'main.c'."""
    modules = []
    for source in target.get("sources", []):
        if source.get("isGenerated") or not source["path"].endswith(".c"):
            continue
        path = source_dir / source["path"]
        if not path.is_file():
            continue
        code = path.read_text(encoding="utf-8", errors="replace")
        # Strip comments; declarations ending in ';' do not count as a main definition.
        code = re.sub(r"/\*.*?\*/|//[^\n]*", " ", code, flags=re.S)
        if re.search(r"\bmain\s*\([^;{}]*\)\s*(?:\{|<%|\?\?<)", code, flags=re.S):
            modules.append(path.stem)
    if len(modules) != 1:
        raise CMakeTargetError(
            f"Target {target['name']!r}: expected one C main() definition, found {len(modules)}"
        )
    return modules[0]


def _pick_target(targets: dict[str, dict], kind: str, target_name: str | None = None) -> str:
    if kind == "binary":
        candidates = {key for key, target in targets.items() if target.get("type") == "EXECUTABLE"}
    else:
        candidates = {key for key, target in targets.items()
                      if target.get("type") in {"STATIC_LIBRARY", "SHARED_LIBRARY", "MODULE_LIBRARY", "OBJECT_LIBRARY"}}
    if not candidates:
        raise CMakeTargetError(f"CMake has no {kind} target")

    # Explicit overrides may name a nested library that would otherwise be
    # discarded as an internal dependency.
    if target_name is not None:
        matches = [key for key, target in targets.items() if target["name"] == target_name]
        if not matches:
            raise CMakeTargetError(f"Unknown CMake target {target_name!r}")
        if len(matches) != 1 or matches[0] not in candidates:
            raise CMakeTargetError(
                f"CMake target {target_name!r} is not a {kind} target"
            )
        return matches[0]

    # Exclude libraries that are internal dependencies of another library.
    if kind == "library":
        internal_dependencies = {
            dep["id"]
            for key in candidates
            for dep in targets[key].get("dependencies", [])
            if dep["id"] in candidates
        }
        roots = candidates - internal_dependencies
        if roots:
            candidates = roots

    if len(candidates) > 1:
        # A single top-level executable named 'driver' is a common TRACTOR
        # convention. Never arbitrarily choose among unrelated applications.
        drivers = [key for key in candidates if targets[key]["name"] == "driver"]
        if kind == "binary" and len(drivers) == 1:
            return drivers[0]
        names = ", ".join(sorted(targets[key]["name"] for key in candidates))
        raise CMakeTargetError(
            f"Multiple possible {kind} targets ({names}); cannot safely select one"
        )
    return next(iter(candidates))


def _output_path(entry: dict) -> Path | None:
    output = entry.get("output")
    if not output:
        command = entry.get("arguments") or shlex.split(entry.get("command", ""))
        if "-o" in command:
            position = command.index("-o") + 1
            output = command[position] if position < len(command) else None
    if not output:
        return None
    return (Path(entry["directory"]) / output).resolve()


def _native_link_flags(targets: dict[str, dict], included: set[str], build_dir: Path) -> list[str]:
    """Rust flags for external native libraries linked by the selected CMake targets.

    CMake-built libraries are deliberately omitted: their C sources are already
    translated into Rust. Linking the original libraries as well could reintroduce
    duplicate symbols. We only forward external -lNAME or libNAME.so/.a items.
    """
    native = []
    seen_libraries = set()
    seen_directories = set()
    internal_names = {targets[key]["name"] for key in targets}
    internal_artifacts = {
        Path(artifact["path"]).name
        for target in targets.values()
        for artifact in target.get("artifacts", [])
    }

    def add_library(name: str, directory: Path | None = None) -> None:
        if not name or name in internal_names:
            return
        if directory is not None:
            directory = directory.resolve()
            if directory not in seen_directories:
                native.extend(["-L", f"native={directory}"])
                seen_directories.add(directory)
        if name not in seen_libraries:
            native.extend(["-l", name])
            seen_libraries.add(name)

    for target_id in sorted(included):
        fragments = targets[target_id].get("link", {}).get("commandFragments", [])
        for fragment in fragments:
            if fragment.get("role") != "libraries":
                continue
            words = shlex.split(fragment.get("fragment", ""))
            pos = 0
            while pos < len(words):
                word = words[pos]
                if word == "-l" and pos + 1 < len(words):
                    pos += 1
                    add_library(words[pos])
                elif word.startswith("-l") and len(word) > 2:
                    add_library(word[2:])
                else:
                    path = Path(word)
                    # Exclude output archives/shared objects from our own CMake
                    # targets, even if referred to by relative filename.
                    if path.name in internal_artifacts:
                        pos += 1
                        continue
                    match = re.fullmatch(r"lib(.+?)\.(?:so(?:\.\d+)*|a|dylib)", path.name)
                    if match and path.is_absolute() and not path.is_relative_to(build_dir):
                        add_library(match.group(1), path.parent)
                pos += 1
    return native


def select_cmake_compilation(
    build_dir: Path, source_dir: Path, kind: str,
    *, target_name: str | None = None,
) -> tuple[Path, str | None, str, list[str]]:
    """Return (filtered database, main module, target name, native Rust link flags).

    Only compilation units of the selected target and all transitive target
    dependencies are included. An ambiguous selection fails explicitly unless
    target_name identifies the exact intended CMake target.
    """
    build_dir = build_dir.resolve()
    source_dir = source_dir.resolve()
    targets = _codemodel(build_dir)
    chosen_id = _pick_target(targets, kind, target_name)
    chosen = targets[chosen_id]
    main_module = _main_module(chosen, source_dir) if kind == "binary" else None

    included = set()
    pending = [chosen_id]
    while pending:
        target_id = pending.pop()
        if target_id in included:
            continue
        included.add(target_id)
        pending.extend(dep["id"] for dep in targets[target_id].get("dependencies", [])
                       if dep["id"] in targets)

    output_prefixes = {}
    for target_id, target in targets.items():
        path = build_dir / target.get("paths", {}).get("build", ".")
        output_prefixes[target_id] = (
            path / "CMakeFiles" / (target["name"] + ".dir")
        ).resolve()

    database = build_dir / "compile_commands.json"
    entries = json.loads(database.read_text(encoding="utf-8"))
    filtered = []
    unrecognized = []
    for entry in entries:
        # C2Rust requires C translation units; unrelated C++/assembler targets
        # must not contaminate the translated Cargo crate.
        if Path(entry["file"]).suffix != ".c":
            continue
        output = _output_path(entry)
        owners = [target_id for target_id, prefix in output_prefixes.items()
                  if output is not None and output.is_relative_to(prefix)]
        if not owners:
            unrecognized.append(entry["file"])
            continue
        if len(owners) != 1:
            raise CMakeTargetError(f"C compilation unit has multiple target owners: {entry['file']}")
        if owners[0] in included:
            filtered.append(entry)

    if unrecognized:
        raise CMakeTargetError(
            f"Cannot associate {len(unrecognized)} C compilation unit(s) with CMake targets; "
            f"first: {Path(unrecognized[0]).name}"
        )
    if not filtered:
        raise CMakeTargetError(f"Target {chosen['name']!r} has no C compilation units")

    target_database = build_dir / "compile_commands_selected.json"
    target_database.write_text(json.dumps(filtered, indent=2) + "\n", encoding="utf-8")
    print(
        f"  [OK] CMake target selection: {chosen['name']} "
        f"({len(filtered)} of {len(entries)} compilation units)",
        flush=True,
    )
    native_flags = _native_link_flags(targets, included, build_dir)
    if native_flags:
        print(f"  [OK] CMake native libraries: {shlex.join(native_flags)}", flush=True)
    return target_database, main_module, chosen["name"], native_flags
