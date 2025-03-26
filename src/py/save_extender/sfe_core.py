from typing import *

from .data import PersistentData
from .serialize_core import *
from .cls_validator import *

from unrealsdk import logging

__all__ = [
    "sfe_register",
    "sfe_load",
    "sfe_save",
]

################################################################################
# | GLOBALS |
################################################################################


_savefile: str | None = None
_data_register: Dict[Type[Any], PersistentData[Any]] = {}
_T = TypeVar('_T')


################################################################################
# | REGISTER |
################################################################################

@overload
def sfe_register(
    cls: Type[MaybeSerialise],
    name: str,
    skip_validation=False,
    instant_load=True,
):
    """
    Register a serialisa
    :param cls:
    :param name: 
    :param skip_validation: 
    :param instant_load: 
    :return: 
    """


@overload
def sfe_register(
    cls: Type[Any],
    name: str,
    skip_validation=False,
    instant_load=True,
): ...


def sfe_register(
    cls: Type[Any],
    name: str,
    skip_validation=False,
    instant_load=True,
) -> PersistentData[_T]:
    pass


################################################################################
# | SAVING |
################################################################################


def sfe_save(cls: Type[Any] = None) -> bool:
    pass


################################################################################
# | LOADING |
################################################################################

def sfe_load(filename: str, cls: Type[Any] = None) -> None:
    pass
