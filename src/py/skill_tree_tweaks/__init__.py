from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .hooks import *

build_mod(
    keybinds=[],
    hooks=[hook_skill_tree_changed1, hook_skill_tree_changed2, hook_skill_tree_init],
    commands=[],
    options=[],
    settings_file=SETTINGS_DIR / "skill_tree_ui_tweaks.json",
)
