from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .options import *
from .hooks import *
from . import data

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

# Register Serialised Data
from save_extender import sfe_register

data._dgs_save_data = sfe_register(data.ScalingData, "dynamic_game_scaling", overwrite=True)
