from unrealsdk.unreal import UObject, UProperty

__all__ = [
    "export_text",
    "export_text_prop",
    "import_text",
]


def export_text(obj: UObject) -> str:
    """
    Exports the given object as a object definition string.
    :param obj: The object to export.
    :return: The exported object definition string.
    """


def export_text_prop(obj: UObject, prop: UProperty, index: int = 0) -> str:
    """
    Exports the provided property from the given object.
    :param obj: The object to export from.
    :param prop: The property to export.
    :param index: The array index if the property is an array.
    :return: The exported property value string.
    """


def import_text(obj: UObject, prop: UProperty, text: str) -> bool:
    """
    Imports the given value string into the given property.
    :param obj: The object which owns the property
    :param prop: The property to import the value into.
    :param text: The value string to import.
    :return: True if the import was successful, False otherwise.
    """
