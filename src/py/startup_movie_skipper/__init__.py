from __future__ import annotations
from typing import TYPE_CHECKING, cast
from unrealsdk import find_all
from unrealsdk.hooks import Type
from mods_base import hook, build_mod, Game, Library

if TYPE_CHECKING:
    from BL1E.WillowGame import WillowPlayerController, WillowGFxMoviePressStart


@hook("Engine.GameViewportClient:Tick", Type.POST, immediately_enable=True)
def hook_skip_to_main_menu(_1, _2, _3, _4) -> None:  # pyright: ignore[reportUnknownParameterType, reportMissingParameterType]
    for obj in find_all("WillowGame.WillowGFxMoviePressStart"):
        if obj.ObjectFlags & (0x400 | 0x200):
            continue

        menu = cast("WillowGFxMoviePressStart", obj)
        if Game.get_current() == Game.BL1E:
            wpc = cast("WillowPlayerController", menu.PlayerOwner)
            menu.extContinue()
            wpc.GFxUIManager.CloseWaitDialog()  # pyright: ignore[reportOptionalMemberAccess]
        else:
            menu.extContinue()

    hook_skip_to_main_menu.disable()


_ = build_mod(cls=Library)
