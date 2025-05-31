from typing import Union
from unrealsdk.unreal import UFunction, UObject

__all__: tuple[str, ...] = (
    "initialise",
    "terminate",
    "get_addr",
)


def get_addr(func: UFunction) -> str: ...


def initialise() -> None: ...


def terminate() -> None: ...
