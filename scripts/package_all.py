import fnmatch
import zipfile
from collections.abc import Sequence
from pathlib import Path


def package_mod(
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


DEST_DIR = Path(__file__).parent.parent / "packaged"
SDK_MODS = Path(__file__).parent.parent / "src" / "py"

paths_to_package = (
    SDK_MODS / "rys_auto_save",
    SDK_MODS / "rys_qol",
    SDK_MODS / "skill_tree_tweaks",
    SDK_MODS / "startup_movie_skipper",
)

for mod in paths_to_package:
    print(f"packaging {mod.name}.sdkmod")
    package_mod(
        mod,
        DEST_DIR / (mod.name + ".sdkmod"),
        ignore_globs=(
            "*__pycache__*",
            "*.ruff_cache*",
        ),
    )
