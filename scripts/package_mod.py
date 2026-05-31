import fnmatch
import zipfile
from pathlib import Path
from typing import Sequence


def package(
    src: Path,
    output: Path,
    ignore_globs: Sequence[str] | None = None,
) -> None:
    lowered_globs = [g.lower() for g in ignore_globs] if ignore_globs else None

    with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as zf:
        for path in sorted(src.rglob("*")):
            if not path.is_file():
                continue

            rel = path.relative_to(src).as_posix()
            if lowered_globs is not None and any(
                fnmatch.fnmatchcase(rel.lower(), glob) for glob in lowered_globs
            ):
                continue

            zf.write(path, f"{src.name}/{rel}")
