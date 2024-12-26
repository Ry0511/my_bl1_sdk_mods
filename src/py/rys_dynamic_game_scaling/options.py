from mods_base.options import *

__all__ = [
    "_flat_scalar"
]

# noinspection PyArgumentList

_flat_scalar = SliderOption(
    identifier="Flat Scalar",
    description_title="Flat Scalar",
    description="Flat scalar value applied to game scaling points; So if the scaling point is +2 "
                "then the actual scale is +2 * S where S is this flat scalar. Note that the scaling"
                " formula is `L + K * S` where L is the current players Level, K is the scaling"
                " increment (the value is dependent on the scaling point common values are 1, 2, 3,"
                " and 4), and S is this flat scalar.",
    value=1.0,
    min_value=1.0,
    max_value=3.0,
    step=0.25,
    is_integer=False,
)
