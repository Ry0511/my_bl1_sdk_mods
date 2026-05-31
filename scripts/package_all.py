import package_mod
from pathlib import Path

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
    package_mod.package(
        mod,
        DEST_DIR / (mod.name + ".sdkmod"),
        ignore_globs=(
            "*__pycache__*",
            "*.ruff_cache*",
        )
    )
