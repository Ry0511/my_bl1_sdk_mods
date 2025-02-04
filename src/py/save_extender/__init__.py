from mods_base import build_mod, ModType, Library

from .hooks import *
from .save_manager import EXTENDED_SAVE_DIR

EXTENDED_SAVE_DIR.mkdir(parents=True, exist_ok=True)

build_mod(
    cls=Library,
    mod_type=ModType.Library,
    hooks=[
        hook_on_save_loaded,
        hook_on_game_save,
        hook_on_game_save2,
    ]
)
