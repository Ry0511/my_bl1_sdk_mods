from typing import Dict, Any
from dataclasses import dataclass, is_dataclass, asdict

__all__ = [
    "sfe_dataclass",
]


def _default_to_dict(self) -> Dict[str, Any]:
    d = asdict(self)
    if d is None or len(d) == 0:
        raise ValueError(f"Type '{self.__class__.__name__}' has no attributes")
    return d


def sfe_dataclass(cls=None):
    def wrap(cls):

        if not is_dataclass(cls):
            cls = dataclass(cls)

        if not hasattr(cls, "to_dict"):
            setattr(cls, "to_dict", _default_to_dict)

        to_dict = getattr(cls, "to_dict")
        if not callable(to_dict):
            raise TypeError(f"Type '{cls.__name__}' should provide a "
                            f"'to_dict(self) -> Dict[str, Any]' method")

        return cls

    return wrap(cls) if cls else wrap
