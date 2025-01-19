from typing import *
from unrealsdk.unreal import UObject

################################################################################
# | STAR IMPORT |
################################################################################


__all__: list[str] = [
    "is_package",
    "find_package",
    "get_all_packages",
    "get_package_contents",
    "get_packages_in_package",
    "get_launch_args"
]


################################################################################
# | API |
################################################################################


def is_package(obj: UObject) -> bool: ...


@overload
def find_package(pkg_name: str) -> Union[UObject, None]:
    ...


@overload
def find_package(pkg_name: str, pkg: UObject) -> Union[UObject, None]:
    ...


@overload
def find_package(pkg_name: str, pkg: Union[UObject, None]) -> Union[UObject, None]:
    """
    Finds a package by path name. Always pass in the full path to the package to obtain i.e.,
    `gd_globals` or `gd_globals.Skills` an optional pkg paramter can be provided which makes
    querying inside packages slightly more efficent (likely negligble).
    :param pkg_name: The full path name of the package
    :param pkg: The package to search inside; optional
    :return: The found package or None
    """
    ...


def get_all_packages() -> List[UObject]:
    """
    :return: All top level packages.
    """
    ...


def get_package_contents(pkg: UObject) -> List[UObject]:
    """
    Gets the top level objects in a package.
    :param pkg: The package to get the contents from.
    :return: Package contents
    """
    ...


def get_packages_in_package(pkg: UObject) -> List[UObject]:
    """
    Gets all the packages in a package.
    :param pkg: The package get the packages from.
    :return: Packages in the package
    """
    ...


def get_objects_in_package(pkg: UObject) -> List[UObject]:
    """
    Gets all the objects in a package excluding packages.
    :param pkg: The package get objects from.
    :return: Objects in the package
    """
    ...


def get_launch_args() -> Union[str, None]:
    ...
