from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .options import *
from .hooks import *

build_mod(
    keybinds=[],
    hooks=[hook_auto_save],
    commands=[],
    options=[
        _auto_save_enabled,
        _auto_save_log,
        _auto_save_type,
        _auto_save_frequency
    ],
    settings_file=SETTINGS_DIR / "rys_auto_save.json",
)
