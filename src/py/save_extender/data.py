import re
from typing import List, TypeVar, Union

__all__: List[str] = [
    "PersistentData",
]

_T = TypeVar("_T")


class PersistentData[_T]:
    filename: str
    """The filename for this file excluding file extension; Must match [a-zA-Z0-9-_]{5,40}"""

    value: Union[_T, None]
    """The loaded value instance or None if no value has been loaded"""

    def __init__(self, name: str, value: _T = None) -> None:
        allowed_chars = re.compile(r'[a-zA-Z0-9-_]{5,40}')
        if not re.fullmatch(allowed_chars, name):
            raise ValueError(f"Invalid data name: '{name}'")
        self.filename = name
        self.value = value

    def __repr__(self) -> str:
        return f"PersistentData( '{self.filename}', {self.value!r} )"
