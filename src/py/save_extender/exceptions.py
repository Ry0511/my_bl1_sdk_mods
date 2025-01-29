__all__ = [
    "SFEBaseException",
    "SFESerializeError",
    "SFEObjectLoadError",
    "SFEInvalidDataClass",
]


class SFEBaseException(Exception):
    pass


class SFESerializeError(SFEBaseException):
    pass


class SFEObjectLoadError(SFEBaseException):
    pass

class SFEInvalidDataClass(SFEBaseException):
    pass
