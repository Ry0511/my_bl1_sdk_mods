from enum import Enum
from typing import override
from mods_base import ENGINE, WillowObjectFlags, Game
from importlib import resources
from unrealsdk.unreal import UObject, WrappedArray
from unrealsdk import logging, find_class
from importlib.util import find_spec

_has_classic_ui = find_spec("Classic UI") is not None


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
    try:
        STATUS_MENU_INSTANCE = "menus_ingame_redux.FlashInstances.status_menu_instance"
        STATUS_MENU_MOVIE = "menus_ingame_redux.FlashMovies.status_menu"

        orig_ui = ENGINE.DynamicLoadObject(
            STATUS_MENU_MOVIE, find_class("GFxMovieInfo")
        )

        # in-world ui movie object - fast fail if we can't get this
        ui_inst: UObject = ENGINE.DynamicLoadObject(
            STATUS_MENU_INSTANCE, find_class("StatusMenuExGFxMovie")
        )
        if ui_inst is None:
            raise RuntimeError(f"failed to load game gfx instance")

        pth = resources.files("skill_tree_tweaks.flash") / get_flash_file_for_opt(
            target_ui
        )
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
