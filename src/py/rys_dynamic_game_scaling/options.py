from mods_base.options import *

__all__ = [
    "_enable_aw_level",
]

_enable_aw_level = BoolOption(
    "rdgs_enable_aw_level",
    display_name="Enable Awesome Level Scaling",
    description="If enabled then the awesome level increments will be applied increasing loot "
                "quality for completing sidequests",
    value=False,
)
