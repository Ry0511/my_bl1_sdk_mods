from mods_base import build_mod, Library

from .blutils import *
from .console_commands import *

__all__ = [
    "import_text",
    "find_object"
]

build_mod(
    cls=Library,
    commands=[
        cmd_set,
        cmd_obj_set,
        cmd_test_obj_set
    ]
)
