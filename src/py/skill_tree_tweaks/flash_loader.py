from __future__ import annotations
from typing import TYPE_CHECKING, override, cast
from enum import Enum
from importlib import resources
from importlib.util import find_spec

from mods_base import (
    ENGINE,  # pyright: ignore[reportAssignmentType]
    ObjectFlags,
    Game,
)
from unrealsdk import logging, find_class

if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.GFxUI import GFxMovie

    ENGINE: Object

_has_classic_ui = find_spec("Classic UI") is not None
_has_custom_skill_trees = find_spec("custom_skill_trees") is not None


class FlashOption(Enum):
    Original = "Original"
    BetterOriginal = "Better Original"
    Enhanced = "Enhanced"
    BetterEnhanced = "Better Enhanced"
    BaseGame = Original if Game.get_current() == Game.BL1 else Enhanced
    Default = (
        BetterOriginal
        if Game.get_current() == Game.BL1 or _has_classic_ui
        else BetterEnhanced
    )

    @override
    def __str__(self) -> str:
        return self.value


def get_flash_file_for_opt(opt: str | FlashOption) -> str:
    match str(opt):  # pyright: ignore[reportMatchNotExhaustive]
        case "Original":
            return "original.swf"
        case "Better Original":
            return "better_original.swf"
        case "Enhanced":
            return "enhanced.swf"
        case "Better Enhanced":
            return "better_enhanced.swf"
    raise ValueError("unknown flash option: " + str(opt))


def patch_flash_objects(target_ui: str | FlashOption) -> None:
    # would prefer a better solution, but this works for now - don't need to mess with the flash
    #  files if it is enabled
    if _has_custom_skill_trees:
        from custom_skill_trees import MOD_INSTANCE as CST_MOD_INST

        if CST_MOD_INST.is_enabled:
            return

    try:
        movie = cast(
            "GFxMovie",
            ENGINE.DynamicLoadObject(
                "menus_ingame_redux.FlashInstances.status_menu_instance",
                find_class("GFxMovie"),
            ),
        )
        assert movie is not None, "failed to load status menu instance"

        FLASH_PATH = "skill_tree_tweaks.flash"
        pth = resources.files(FLASH_PATH) / get_flash_file_for_opt(target_ui)
        content = pth.read_bytes()

        xs = movie.MovieInfo.RawData
        xs.clear()
        for elem in content:
            xs.append(elem)

        movie.ObjectFlags |= ObjectFlags.KEEP_ALIVE
        movie.MovieInfo.ObjectFlags |= ObjectFlags.KEEP_ALIVE

    except Exception as ex:
        logging.error(f"Error trying to apply custom ui flash changes: {ex}")
        return
