from __future__ import annotations

from importlib import resources
from typing import TYPE_CHECKING, cast
from unrealsdk.unreal import WrappedArray
from unrealsdk import logging, find_class
from mods_base import ENGINE, WillowObjectFlags


if TYPE_CHECKING:
    from BL1.GFxUI import GFxMovieInfo
    from BL1.WillowGame import StatusMenuExGFxMovie


def patch_flash() -> None:
    try:
        STATUS_MENU_INSTANCE = "menus_ingame_redux.FlashInstances.status_menu_instance"
        STATUS_MENU_MOVIE = "menus_ingame_redux.FlashMovies.status_menu"

        orig_ui = cast(
            "GFxMovieInfo",
            ENGINE.DynamicLoadObject(  # pyright: ignore[reportAny]
                STATUS_MENU_MOVIE, find_class("GFxMovieInfo")
            ),
        )

        ui_inst = cast(
            "StatusMenuExGFxMovie | None",
            ENGINE.DynamicLoadObject(  # pyright: ignore[reportAny]
                STATUS_MENU_INSTANCE, find_class("StatusMenuExGFxMovie")
            ),
        )

        if ui_inst is None:
            raise RuntimeError("failed to load game gfx instance")

        pth = resources.files("custom_skill_trees.assets") / "bl1_tree.swf"
        content = pth.read_bytes()
        xs: WrappedArray = orig_ui.RawData
        xs.clear()
        for elem in content:
            xs.append(elem)

        ui_inst.MovieInfo = orig_ui
        ui_inst.ObjectFlags |= WillowObjectFlags.KEEP_ALIVE  # pyright: ignore[reportUnknownMemberType, reportAttributeAccessIssue]
        orig_ui.ObjectFlags |= WillowObjectFlags.KEEP_ALIVE  # pyright: ignore[reportUnknownMemberType, reportAttributeAccessIssue]

    except Exception as ex:
        logging.error(f"Error trying to apply custom ui flash changes: {ex}")
        return
