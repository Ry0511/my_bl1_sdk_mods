from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .options import *
from .hooks import *

build_mod(
    keybinds=[],
    hooks=[
        hook_commit_map_change,
        hook_level_loaded,
        hook_on_mission_complete,
    ],
    commands=[],
    options=[_enable_aw_level],
    settings_file=SETTINGS_DIR / "rys_dynamic_game_scaling.json",
)

from save_extender import *
from . import data

data._dgs_save_data = sfe_register(data.ScalingData, "dynamic_game_scaling")
