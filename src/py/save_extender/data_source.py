import json

from typing import Any, Type, Dict
from pathlib import Path
from abc import ABC, abstractmethod

from .exceptions import *


class DataManager(ABC):
    data_cls: Type[Any]
    data_name: str

    def __init__(self, data_cls: Type[Any], data_name: str):
        self.data_cls = data_cls
        self.data_name = data_name

        try:
            __ignore = self.data_cls()
            if __ignore is None:
                raise SFEInvalidDataClass("Class should be default constructable")

        except Exception as ex:
            err = ex if isinstance(ex, SFEBaseException) else SFEInvalidDataClass(ex)
            err.add_note(f"Data Class: '{self.data_cls}'")
            err.add_note(f"Data Name: '{self.data_name}'")
            raise err

        import re
        if data_name is None or not re.fullmatch(r'[a-zA-Z_\-0-9]+', data_name):
            raise ValueError(f'Invalid data name: "{data_name}" should match "some_DATA-123"')

    @abstractmethod
    def get_data_filename(self) -> str:
        """
        :return: Filename for this data-file; Must only contain these characters 'some_DATA-123'
        (excluding quotes)
        """

    @abstractmethod
    def create_or_default(self, fp: Path | None) -> Any:
        """
        Creates a new instance of the data we are managing.
        :param fp: If not None then this is a filepath to initialise from.
        :return: A default instance or an instance loaded from a file.
        """

    @abstractmethod
    def save_to_file(self, data: Any, fp: Path) -> None:
        """
        :param data: The data to save.
        :param fp: The filepath to save to; The file may not exist.
        """


################################################################################
# | JSON Data Manager |
################################################################################

class JSONDataManager(DataManager):

    def __init__(self, cls: Type[Any], data_name: str):
        super().__init__(cls, data_name)
        self._validate_cls()

    def get_data_filename(self) -> str:
        return f"{self.data_name.lower()}.json"

    def create_or_default(self, fp: Path | None) -> Any:

        try:
            if fp is None:
                return self.data_cls()

            with open(fp) as handle:
                data = json.load(handle)

                # This should never happen
                if not isinstance(data, dict):
                    raise SFEObjectLoadError(f"JSON loaded object was not a dict; '{fp}'")

                return self.data_cls(**data)

        except Exception as ex:  # noqa
            err = SFEObjectLoadError(ex) if not isinstance(ex, SFEBaseException) else ex
            err.add_note(f"Failed to create instance of type '{self.data_cls.__name__}'")
            err.add_note(f"JSON File path: '{fp.absolute() if fp is not None else 'None'}'")
            raise err

    def save_to_file(self, data: Any, fp: Path) -> None:

        if not fp.is_file():
            fp.parent.mkdir(parents=True, exist_ok=True)
            fp.touch()

        with fp.open('w', encoding='utf-8') as handle:
            try:
                json.dump(data, handle, indent=4, default=JSONDataManager._default_to_dict)  # type: ignore
            except Exception as ex:
                err = SFESerializeError(ex) if not isinstance(ex, SFEBaseException) else ex
                err.add_note(f"Failed to write JSON file: '{fp}'")
                raise err

    @staticmethod
    def _default_to_dict(obj: Any) -> Dict[str, Any]:
        if hasattr(obj, 'to_dict'):
            return obj.to_dict()
        else:
            raise SFESerializeError(
                f"Could not serialise type '{type(obj).__name__}' "
                f"as it does not expose a 'to_dict(obj: Any) -> Dict[str, Any]' method"
            )

    def _validate_cls(self):
        try:
            x = self.data_cls()
            as_json = json.dumps(x, default=JSONDataManager._default_to_dict)
            from_json = json.loads(as_json)

            if from_json is None:
                raise SFEInvalidDataClass(f"Failed to convert JSON; '{as_json}' to class instance")

        except Exception as ex:
            err = SFEInvalidDataClass(ex) if not isinstance(ex, SFEBaseException) else ex
            err.add_note(f"Failed to validate data class '{self.data_cls}'")
            raise err
