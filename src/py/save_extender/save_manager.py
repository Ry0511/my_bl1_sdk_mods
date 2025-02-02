from pathlib import Path
from typing import *

from mods_base import MODS_DIR
from unrealsdk import logging
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


class PersistentData[_T]:
    manager: DataManager
    value: _T

    def __init__(self, manager: DataManager):
        self.manager = manager
        self.value = None


EXTENDED_SAVE_DIR: Path = MODS_DIR / "_savedata"

_current_savefile: str | None = None
_data_register: Dict[str, PersistentData[Any]] = {}


def _get_savefile_dir(filename: str, create=True) -> Path:
    # Assuming this will work
    save_dir = EXTENDED_SAVE_DIR / filename.split(".")[0]
    if create:
        save_dir.mkdir(parents=True, exist_ok=True)
    return save_dir


################################################################################
# | PUBLIC API |
################################################################################


def sfe_register(
    data_cls: Type[_T],
    data_name: str,
    manager: DataManager = None
) -> PersistentData[_T]:
    """
    Registers the provided class as a datasource for the provided data name.
    :param data_cls: The data class
    :param data_name: The name of the data; Must only be alphanumeric with underscores and hyphens
    :param manager: The manager of this datasource; Defaults to JSONDataManager if None.
    :return: PersistentData
    """
    if data_name is None:
        raise ValueError("None is an invalid data name")

    if data_name in _data_register:
        return _data_register[data_name]

    if manager is None:
        manager = JSONDataManager(data_cls, data_name)

    data = PersistentData(manager)
    _data_register[data_name] = data
    return data


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


def sfe_save() -> None:

    if _current_savefile is None:
        logging.dev_warning("[SFE] Can't save current savefile is None")
        return

    savefile_dir = _get_savefile_dir(_current_savefile)
    for _, v in _data_register.items():

        if v.value is None:
            continue

        data_file = savefile_dir / v.manager.get_data_filename()
        v.manager.save_to_file(v.value, data_file)
