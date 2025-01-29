from unrealsdk import logging
from mods_base.hook import *

from .save_manager import *

__all__ = [
    "hook_on_save_loaded",
    "hook_on_game_save",
    "hook_on_game_save2",
]

_sfe_invalid_save_error = (
    "Filename '{}' is invalid (None or not ending with '.sav');"
    " SaveExtender functionality will not work."
)


def is_valid_savefile(sf: str | None) -> bool:
    # Could be extra pedantic but 99.9% of the time this will be enough
    return sf is not None and sf.endswith(".sav")


################################################################################
# | LOADING |
################################################################################


@hook(hook_func="WillowGame.WillowPlayerController:LoadPlayerProfile", hook_type=Type.POST_UNCONDITIONAL)
def hook_on_save_loaded(
    obj: UObject,
    __args: WrappedStruct,
    __ret,
    __func: BoundFunction,
) -> None:
    savefile = obj.SaveGameName

    if not is_valid_savefile(savefile):
        logging.dev_warning(_sfe_invalid_save_error.format(savefile))
        return

    sfe_load_save(str(savefile))


################################################################################
# | SAVING |
################################################################################


@hook(hook_func="WillowGame.WillowPlayerController:sg_save_game")
def hook_on_game_save(
    obj: UObject,
    __args: WrappedStruct,
    __ret,
    __func: BoundFunction,
) -> None:
    if obj.IsLoadingMoviePlaying():
        return

    filename = obj.SaveGameName
    if not is_valid_savefile(filename):
        logging.dev_warning(_sfe_invalid_save_error.format(filename))
        return

    sfe_save()


@hook(hook_func="WillowGame.WillowPlayerController:SaveGame")
def hook_on_game_save2(
    obj: UObject,
    __args: WrappedStruct,
    __ret,
    __func: BoundFunction,
) -> None:
    if obj.IsLoadingMoviePlaying():
        return

    filename = obj.SaveGameName
    if not is_valid_savefile(filename):
        logging.dev_warning(_sfe_invalid_save_error.format(filename))
        return

    sfe_save()
