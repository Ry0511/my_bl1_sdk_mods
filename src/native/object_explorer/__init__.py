from mods_base import build_mod
from .object_explorer import initialise, terminate, get_addr

__all__ = [
    'initialise',
    'terminate',
    'get_addr',
]

build_mod(
    on_enable=lambda: initialise(),
    on_disable=lambda: terminate(),
    auto_enable=False
)
