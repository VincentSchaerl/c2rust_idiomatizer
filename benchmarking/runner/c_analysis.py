"""Count C for-loops using libclang and Bear's compilation database."""

import json
import os
import shlex
from pathlib import Path


class CAnalysisError(RuntimeError):
    """C source analysis failed or could not be trusted."""


def _parse_arguments(entry: dict, source: Path, directory: Path) -> list[str]:
    """Keep compiler parsing flags, dropping compiler/build-only arguments."""
    if "arguments" in entry:
        command = entry["arguments"]
    elif "command" in entry:
        command = shlex.split(entry["command"])
    else:
        raise CAnalysisError(f"Missing compilation command for {source}")

    if not command:
        raise CAnalysisError(f"Empty compilation command for {source}")

    # First element is the compiler executable (e.g. 'cc' or '/usr/bin/gcc').
    args = command[1:]
    result = []
    skip_next = False
    value_flags = {"-o", "-MF", "-MT", "-MQ", "-MJ"}
    build_flags = {"-c", "-S", "-E", "-MD", "-MMD", "-MP"}

    for arg in args:
        if skip_next:
            skip_next = False
            continue
        if arg in value_flags:
            skip_next = True
            continue
        if arg in build_flags:
            continue
        if any(arg.startswith(prefix) and arg != prefix for prefix in value_flags):
            continue

        # index.parse() receives the input source path separately.
        if not arg.startswith("-") and (directory / arg).resolve() == source:
            continue

        result.append(arg)

    return result


def count_c_for_loops(compilation_database: Path, source_root: Path) -> int:
    """Count distinct, benchmark-owned C for statements in compiled C units.

    Parsing uses build flags captured by Bear. A loop in a header included
    by several translation units is counted once by source location.
    Loops in external/system headers are excluded.
    """
    try:
        from clang.cindex import CursorKind, Diagnostic, Index
    except ImportError as exc:
        raise CAnalysisError(
            "libclang Python bindings are missing. On Ubuntu/Debian, "
            "install them with: sudo apt install python3-clang"
        ) from exc

    compilation_database = compilation_database.resolve()
    source_root = source_root.resolve()
    try:
        commands = json.loads(compilation_database.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        raise CAnalysisError(f"Cannot read {compilation_database}: {exc}") from exc

    if not isinstance(commands, list) or not commands:
        raise CAnalysisError(f"No compilation commands in {compilation_database}")

    try:
        index = Index.create()
    except Exception as exc:
        raise CAnalysisError(f"Cannot load libclang: {exc}") from exc

    locations: set[tuple[Path, int, int]] = set()
    parsed_c_files = 0
    original_cwd = Path.cwd()

    try:
        for entry in commands:
            directory = Path(entry["directory"]).resolve()
            source = (directory / entry["file"]).resolve()

            # Bear can capture compilation from projects other than this benchmark.
            if source.suffix != ".c" or not source.is_relative_to(source_root):
                continue

            if not source.is_file():
                raise CAnalysisError(f"C source file is missing: {source}")

            args = _parse_arguments(entry, source, directory)
            try:
                # Keep relative -I / -include paths relative to the compiler's
                # original working directory (not the Python script directory).
                os.chdir(directory)
                unit = index.parse(str(source), args=args)
            except Exception as exc:
                raise CAnalysisError(f"Clang could not parse {source}: {exc}") from exc
            finally:
                os.chdir(original_cwd)

            errors = [
                diagnostic
                for diagnostic in unit.diagnostics
                if diagnostic.severity >= Diagnostic.Error
            ]
            if errors:
                raise CAnalysisError(
                    f"Clang reported errors while analyzing {source}:\n"
                    + "\n".join(str(d) for d in errors)
                )

            parsed_c_files += 1
            for cursor in unit.cursor.walk_preorder():
                if cursor.kind != CursorKind.FOR_STMT:
                    continue
                location = cursor.location
                if location.file is None:
                    continue
                file = Path(location.file.name).resolve()
                if file.is_relative_to(source_root):
                    locations.add((file, location.line, location.column))
    finally:
        os.chdir(original_cwd)

    if parsed_c_files == 0:
        raise CAnalysisError(
            "No C source files under the benchmark directory were found "
            "in compile_commands.json"
        )

    return len(locations)
