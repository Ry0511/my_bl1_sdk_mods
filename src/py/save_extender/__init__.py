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

################################################################################
# | TESTING |
################################################################################

from .utils import sfe_dataclass
from .save_manager import *

@sfe_dataclass
class MyData:
    some_str: str = "Some String"
    some_int: int = 12345

    def to_dict(self):
        return {
            "some_str": self.some_str,
            "some_int": self.some_int,
        }


_data = sfe_register(MyData, "test_data")
