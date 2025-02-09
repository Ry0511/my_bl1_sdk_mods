from mods_base import build_mod, ModType, Library

from .hooks import *
from .save_manager import *
from .utils import *
from .data_source import *

# Common imports
__all__ = [
    "sfe_dataclass",
    "sfe_save",
    "sfe_register",
    "sfe_load_save",
    "sfe_get_data",
    "PersistentData",
]

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
