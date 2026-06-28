from __future__ import annotations
from typing import TYPE_CHECKING, cast
import time
from typing import Any
from unrealsdk import logging
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction
from mods_base import get_pc, hook

from .options import _auto_save_type, _auto_save_frequency

if TYPE_CHECKING:
    from BL1E.WillowGame import WillowPlayerController

__all__ = [
    "hook_auto_save",
]

_last_tick_time: float | int | None = None


@hook(hook_func="WillowGame.WillowPlayerController:PlayerTick")
def hook_auto_save(
    obj: UObject,
    _args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _func: BoundFunction,
) -> None:
    obj = cast("WillowPlayerController", obj)
    global _last_tick_time
    tick_time_now = time.time()

    if _last_tick_time is None:
        _last_tick_time = tick_time_now
        return

    if (tick_time_now - _last_tick_time) < _auto_save_frequency.value:
        return
    _last_tick_time = time.time()

    pc = cast("WillowPlayerController", get_pc())
    file_name = pc.SaveGameName

    if file_name == "":
        return

    if _auto_save_type.value == "To Savefile":
        pc.sg_save_game()
        logging.info(f"Autosave: '{file_name}'")
    elif _auto_save_type.value == "Into Autosave Directory":
        pc.sg_save_game(f"Autosave/{file_name}")
        logging.info(f"Autosave: 'Autosave/{file_name}'")
