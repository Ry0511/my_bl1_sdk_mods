import dataclasses
import json

from pathlib import Path
from typing import *

from mods_base import MODS_DIR

__all__ = [
    "SupportsFactoryCreate",
    "SupportsToDict",
    "MaybeSerialise",
    "_default_create",
    "_default_to_dict",
    "save_as_json",
    "load_from_json",
    "EXTENDED_SAVE_DIR",
]

_T = TypeVar("_T")

EXTENDED_SAVE_DIR = MODS_DIR / "_savedata"


################################################################################
# | HELPERS |
################################################################################


def save_as_json(obj: Any, fp: Path) -> None:
    if not fp.is_file():
        fp.parent.mkdir(parents=True, exist_ok=True)
        fp.touch()

    payload = json.dumps(obj, indent=2, default=_default_to_dict)
    fp.write_text(payload, encoding="utf-8")


def load_from_json(fp: Path) -> Any | None:
    if not fp.is_file():
        return None
    return json.loads(fp.read_text(encoding="utf-8"))


def get_bl1_save(filename: str) -> Path:
    # C:\Users\-Ry\Documents\my games\borderlands\savedata
    save_dir = Path().home() / "Documents" / "My Games" / "Borderlands" / "savedata"
    if not save_dir.is_dir():
        raise RuntimeError(f"Save directory does not exist at '{save_dir!r}'")
    return save_dir / filename


def get_ext_save(filename: str) -> Path:
    name = filename.split(".")[0]
    return

################################################################################
# | SERIALISATION PROTOCOLS |
################################################################################


@runtime_checkable
class SupportsFactoryCreate(Protocol):
    @staticmethod
    def create(d: Union[Dict[str, Any], None]) -> _T:
        """
        Called to create instances of your data class; Default implementation is using the
        constructor (via T(**d) or T() if d is None)
        :param d: The loaded data dictionary or None if default constructing.
        :return: A valid instance.
        """


def _default_create(cls: Type[Any], d: Dict[str, Any] | None = None) -> _T:
    """
    Attempts to instantiate the provided type using the SupportsFactoryCreate protocol or using the
    constructor of the type.
    :param cls: The type to instantiate.
    :param d: The dictionary to use to instantiate the type; or None if default constructing.
    :return: A valid instance of type cls
    """
    if isinstance(cls, SupportsFactoryCreate):
        return cls.create(d)
    elif dataclasses.is_dataclass(cls):
        return cls(**d) if d is not None else cls()
    else:
        raise TypeError(
            f"'{cls.__name__}' ca not be instantiated. It needs to be a dataclass with default "
            f"ctor & kwargs ctor or satisfy the SupportsFactoryCreate protocol."
        )


@runtime_checkable
class SupportsToDict(Protocol):
    def to_dict(self) -> Dict[str, Any]:
        """
        Called to convert your class to a dictionary; Default implementation is dataclasses.asdict()
        :return: A valid dictionary of attributes to serialise.
        """


def _default_to_dict(obj: Any) -> Dict[str, Any]:
    """
    Attempts to convert the provided object to a dictionary. Will attempt to use the SupportsToDict
    protocol if available and if it is not available it will fallback to dataclasses.asdict().
    :param obj: The object to convert.
    :return: A valid dictionary of the objects attributes.
    """
    if isinstance(obj, SupportsToDict):
        return obj.to_dict()
    elif dataclasses.is_dataclass(obj):
        return dataclasses.asdict(obj)
    else:
        raise TypeError(
            f"'{type(obj).__name__}' is not a dataclass or a valid SupportsToDict protocol instance"
        )


# We might be able to serialise this but will need to try first
MaybeSerialise = Union[SupportsFactoryCreate, SupportsToDict]
