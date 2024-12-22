from unrealsdk import logging
from mods_base.hook import *
from mods_base import get_pc

from .options import *

__all__ = [
    "hook_auto_save",
]

_theta = 0.0


@hook(hook_func="WillowGame.WillowPlayerController:PlayerTick")
def hook_auto_save(
        __obj: UObject,
        args: WrappedStruct,
        __ret: any,  # Need to stop doing this lmao
        __func: BoundFunction,
) -> None:
    global _theta
    _theta += args.DeltaTime

    if _theta < _auto_save_frequency.value:
        return

    pc = get_pc()
    file_name = pc.SaveGameName

    if file_name is None or file_name == "":  # No valid file to save to
        _theta = 0.0
        return

    if _auto_save_type.value == "To Savefile":
        pc.sg_save_game()
        logging.info(f"Autosave: '{file_name}'")
    elif _auto_save_type.value == "Into Autosave Directory":
        pc.sg_save_game(f"Autosave/{file_name}")
        logging.info(f"Autosave: 'Autosave/{file_name}'")

    _theta = 0.0
