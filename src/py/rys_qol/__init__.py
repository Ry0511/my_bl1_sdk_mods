from mods_base import SETTINGS_DIR, build_mod

from .hooks import (
    on_player_loaded,
    hook_fix_auto_mission_select,
    hook_sort_fast_travels,
    hook_sort_fast_travels_2,
    hook_indent_fast_travel_tab_changed,
    hook_fast_travel_indent,
)

from .keybinds import (
    on_save_position,
    on_restore_position,
    on_quit_without_saving,
    on_toggle_hlq_noclip,
    on_make_op,
)

from .options import should_sort_fast_travels, fix_auto_mission_select

_ = build_mod(
    keybinds=(
        on_save_position,
        on_restore_position,
        on_quit_without_saving,
        on_toggle_hlq_noclip,
        on_make_op,
    ),
    hooks=(
        on_player_loaded,
        hook_fix_auto_mission_select,
        hook_sort_fast_travels,
        hook_sort_fast_travels_2,
        hook_indent_fast_travel_tab_changed,
        hook_fast_travel_indent,
    ),
    commands=[],
    options=(should_sort_fast_travels, fix_auto_mission_select),
    settings_file=SETTINGS_DIR / "rys_qol.json",
)
