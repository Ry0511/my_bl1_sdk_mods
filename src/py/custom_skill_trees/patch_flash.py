from __future__ import annotations

from importlib import resources
from typing import TYPE_CHECKING, cast
from unrealsdk.unreal import WrappedArray
from unrealsdk import logging, find_class
from mods_base import (
    ENGINE,  # pyright: ignore[reportAssignmentType]
    ObjectFlags,
)


if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.GFxUI import GFxMovie

    ENGINE: Object


def patch_flash() -> None:
    try:
        movie = cast(
            "GFxMovie",
            ENGINE.DynamicLoadObject(
                "menus_ingame_redux.FlashInstances.status_menu_instance",
                find_class("GFxMovie"),
            ),
        )
        assert movie is not None

        pth = resources.files("custom_skill_trees.assets") / "bl1_tree.swf"
        content = pth.read_bytes()
        xs: WrappedArray = movie.MovieInfo.RawData
        xs.clear()
        for elem in content:
            xs.append(elem)

        movie.ObjectFlags |= ObjectFlags.KEEP_ALIVE
        movie.MovieInfo.ObjectFlags |= ObjectFlags.KEEP_ALIVE
    except Exception as ex:
        logging.error(f"Error trying to apply custom ui flash changes: {ex}")
        return
