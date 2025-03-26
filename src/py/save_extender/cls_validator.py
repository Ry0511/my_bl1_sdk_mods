import json
from typing import *

from .serialize_core import *

__all__ = [
    "ValidationError",
    "validate_cls",
]


class ValidationError(Exception):
    pass


def validate_cls(cls: Type[Any]) -> None:
    try:
        # Default instance construction
        obj = _default_create(cls)
        if not obj or not isinstance(obj, cls):
            raise ValidationError("Default construction of type failed")

        # Mapping instance to dictionary
        d = _default_to_dict(obj)
        if not d or not isinstance(d, dict) or len(d) == 0:
            raise ValidationError(f"Failed to convert '{obj}' to a dictionary; The dict '{d}'")

        # Converting to json and back to an instance
        json_str = json.dumps(obj, default=_default_to_dict)
        loaded_json = json.loads(json_str)
        if not loaded_json or not isinstance(loaded_json, dict):
            raise ValidationError(f"JSON loaded object is not a dictionary; The object '{loaded_json}'")

        obj = _default_create(cls, loaded_json)
        if not obj or not isinstance(obj, cls):
            raise ValidationError(f"Construction of type with dictionary '{loaded_json}' failed")

    except Exception as ex:  # noqa
        err = ex if isinstance(ex, ValidationError) else ValidationError(ex)
        err.add_note(f"Validation for type '{cls.__name__}' failed")
        raise err
