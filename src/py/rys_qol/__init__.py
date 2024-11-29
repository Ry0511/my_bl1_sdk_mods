from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .hooks import on_player_loaded
from .commands import balance_me_cmd
from .keybinds import (
    on_save_position,
    on_restore_position,
    on_quit_without_saving,
    on_toggle_ghost,
    on_toggle_hlq_noclip,
    on_make_op,
)

build_mod(
    keybinds=[
        on_save_position,
        on_restore_position,
        on_quit_without_saving,
        on_toggle_ghost,
        on_toggle_hlq_noclip,
        on_make_op
    ],
    hooks=[on_player_loaded],
    commands=[balance_me_cmd],
    settings_file=SETTINGS_DIR / "RysQoL.json",
)