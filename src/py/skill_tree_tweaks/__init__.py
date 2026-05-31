from unrealsdk import logging
from unrealsdk.unreal import WrappedArray
from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .hooks import *


def _patch_flash_objects(undo: bool) -> None:
    try:
        from mods_base import ENGINE, WillowObjectFlags
        from unrealsdk import load_package, find_object, find_class
        from unrealsdk.unreal import UObject
        from pathlib import Path
        from importlib import resources

        STATUS_MENU_INSTANCE = "menus_ingame_redux.FlashInstances.status_menu_instance"
        STATUS_MENU_MOVIE = "menus_ingame_redux.FlashMovies.status_menu"

        # in-world ui movie object - fast fail if we can't get this
        ui_inst: UObject = ENGINE.DynamicLoadObject(STATUS_MENU_INSTANCE, find_class("StatusMenuExGFxMovie"))
        if ui_inst is None:
            raise RuntimeError(f"failed to load game gfx instance")

        orig_ui = ENGINE.DynamicLoadObject(STATUS_MENU_MOVIE, find_class("GFxMovieInfo"))

        if undo:
            if orig_ui is None:
                raise RuntimeError(f"failed to load original game gfx instance for undo")

            ui_inst.ObjectFlags &= (~WillowObjectFlags.KEEP_ALIVE)
            ui_inst.MovieInfo.ObjectFlags &= (~WillowObjectFlags.KEEP_ALIVE)
            ui_inst.MovieInfo = orig_ui

        else:
            pth = resources.files("skill_tree_tweaks") / "custom_ui_flash.bin"
            content = pth.read_bytes()
            xs: WrappedArray = orig_ui.RawData
            xs.clear()
            for elem in content:
                xs.append(elem)

            ui_inst.MovieInfo = orig_ui
            ui_inst.ObjectFlags |= WillowObjectFlags.KEEP_ALIVE
            orig_ui.ObjectFlags |= WillowObjectFlags.KEEP_ALIVE

    except Exception as ex:
        logging.error(f"Error trying to apply custom ui flash changes: {ex}")
        return


# noinspection PyArgumentList
build_mod(
    hooks=[
        hook_skill_tree_changed1,
        hook_skill_tree_changed2,
        hook_skill_tree_init,
        hook_skill_tree_selection_after,
    ],
    on_enable=lambda: _patch_flash_objects(undo=False),
    on_disable=lambda: _patch_flash_objects(undo=True),
    settings_file=SETTINGS_DIR / "skill_tree_ui_tweaks.json",
)
