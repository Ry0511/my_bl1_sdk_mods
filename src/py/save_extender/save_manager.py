from pathlib import Path
from dataclasses import dataclass
from typing import TypeVar, Type, Dict, Any, Union

from unrealsdk import logging
from mods_base import MODS_DIR
from .data_source import DataManager, JSONDataManager

__all__ = [
    "EXTENDED_SAVE_DIR",
    "sfe_register",
    "sfe_get_data",
    "sfe_load_save",
    "sfe_save",
    "PersistentData",
]

################################################################################
# | HELPERS |
################################################################################

_T = TypeVar("_T")


@dataclass
class PersistentData[_T]:
    manager: DataManager
    value: _T


EXTENDED_SAVE_DIR: Path = MODS_DIR / "_savedata"

_current_savefile: str | None = None
_data_register: Dict[str, PersistentData[Any]] = {}


def _get_savefile_dir(filename: str, create=True) -> Path:
    # Assuming this will work
    save_dir = EXTENDED_SAVE_DIR / filename.split(".")[0].strip().replace(" ", "_")
    if create:
        save_dir.mkdir(parents=True, exist_ok=True)
    return save_dir


################################################################################
# | PUBLIC API |
################################################################################


def sfe_register(
    data_cls: Type[_T],
    data_name: str,
    manager: DataManager = None,
    overwrite: bool = False,
) -> PersistentData[_T]:
    if data_name is None:
        raise ValueError("None is an invalid data name")

    # The caller really should check this; but i don't want to throw
    if sfe_is_data_registered(data_name) and not overwrite:
        return sfe_get_data(data_name)

    if manager is None:
        manager = JSONDataManager(data_cls, data_name)

    data = PersistentData(manager, None)
    _data_register[data_name] = data
    return data


def sfe_is_data_registered(data_name: str) -> bool:
    return data_name in _data_register


def sfe_get_data(data_name: str) -> Union[PersistentData[_T], None]:
    """
    :param data_name: The name of the data to get
    :return: The PersistentData found or None if not found
    """
    if data_name not in _data_register:
        return None
    else:
        return _data_register[data_name]


def sfe_load_save(savefile: str) -> None:
    global _current_savefile

    if _current_savefile == savefile:
        return  # This can happen but just save-quit reloading

    if _current_savefile is not None:
        sfe_save()

    _current_savefile = savefile

    savefile_dir = _get_savefile_dir(savefile)
    for _, v in _data_register.items():
        data_file = savefile_dir / v.manager.get_data_filename()

        if data_file.is_file():
            v.value = v.manager.create_or_default(data_file)
        else:
            v.value = v.manager.create_or_default(None)


def sfe_save(data_name: str = None) -> None:
    if _current_savefile is None:
        logging.dev_warning("[SFE] Can't save current savefile is None")
        return

    savefile_dir = _get_savefile_dir(_current_savefile)

    # Saving a specific data object
    if data_name is not None:
        if data_name in _data_register:
            k, v = _data_register[data_name]
            v.manager.save_to_file(savefile_dir / v.manager.get_data_filename())
        return

    # Saving all data objects
    for _, v in _data_register.items():
        if v.value is None:
            continue
        v.manager.save_to_file(v.value, savefile_dir / v.manager.get_data_filename())
