from mods_base import build_mod, ModType, Library

from .data import PersistentData
from .serialize_core import *
from .sfe_core import *
from .hooks import *

__all__ = [
    "PersistentData",
    "sfe_register",
    "sfe_load",
    "sfe_save",
    "SupportsToDict",
    "SupportsFactoryCreate",
]

EXTENDED_SAVE_DIR.mkdir(parents=True, exist_ok=True)

if not EXTENDED_SAVE_DIR.is_dir():
    raise RuntimeError(f"Failed to create/validate extended save directory: '{EXTENDED_SAVE_DIR.resolve()}'")

build_mod(
    cls=Library,
    mod_type=ModType.Library,
    hooks=[
        hook_on_save_loaded,
        hook_on_game_save,
        hook_on_game_save2,
    ]
)
