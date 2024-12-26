from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .options import *
from .hooks import *

build_mod(
    keybinds=[],
    hooks=[
        hook_commit_map_change,
        hook_level_loaded,
    ],
    commands=[],
    options=[_flat_scalar],
    settings_file=SETTINGS_DIR / "rys_dynamic_game_scaling.json",
)
