from typing import *
from unrealsdk.unreal import *


def import_text(
    parent: UObject,
    prop: UProperty,
    text: str,
    flags: int,
    obj_flags: int
) -> bool:
    """
    Sets the given property to the value represented by the text string.
    :param parent: The source object we are writing to
    :param prop: The property we are writing
    :param text: The text data to import from
    :param flags: relates to EPropertyPortFlags
    :param obj_flags: The object flags to set on the parent object (on success)
    :return: True on success
    """


def import_object(
    base_pkg: str,
    text: str
) -> List[UObject]:
    ...


def find_object(obj_path_name: str, bthrow=False) -> Union[UObject, None]:
    """
    Attempts to find an object with the given name
    :param bthrow:
    :param obj_path_name:
    :return:
    """
