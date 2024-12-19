from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .options import _should_sort_fast_travels

from .hooks import *
from .keybinds import *

build_mod(
    keybinds=[
        on_save_position,
        on_restore_position,
        on_quit_without_saving,
        on_toggle_ghost,
        on_toggle_hlq_noclip,
        on_make_op
    ],
    hooks=[
        on_player_loaded,
        hook_sort_fast_travels,
        hook_sort_fast_travels_2,
        hook_indent_fast_travel_tab_changed,
        hook_fast_travel_indent
    ],
    commands=[],
    options=[_should_sort_fast_travels],
    settings_file=SETTINGS_DIR / "rys_qol.json",
)
