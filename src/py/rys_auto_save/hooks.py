import time
from typing import Any
from unrealsdk import logging
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction
from mods_base import get_pc, hook

from .options import *

__all__ = [
    "hook_auto_save",
]

_last_tick_time: float | int | None = None


@hook(hook_func="WillowGame.WillowPlayerController:PlayerTick")
def hook_auto_save(
    __obj: UObject,
    _args: WrappedStruct,
    _ret: Any,
    _func: BoundFunction,
) -> None:

    global _last_tick_time
    tick_time_now = time.time()

    if _last_tick_time is None:
        _last_tick_time = tick_time_now
        return

    if (tick_time_now - _last_tick_time) < _auto_save_frequency.value:
        return
    _last_tick_time = time.time()

    pc = get_pc()
    file_name = pc.SaveGameName

    if file_name is None or file_name == "":  # No valid file to save to
        return

    if _auto_save_type.value == "To Savefile":
        pc.sg_save_game()
        logging.info(f"Autosave: '{file_name}'")
    elif _auto_save_type.value == "Into Autosave Directory":
        pc.sg_save_game(f"Autosave/{file_name}")
        logging.info(f"Autosave: 'Autosave/{file_name}'")
