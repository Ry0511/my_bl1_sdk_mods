from typing import Dict
from dataclasses import field

from save_extender import sfe_dataclass, PersistentData
from .utils import GameRegion

__all__ = [
    "BoundedFloat",
    "ScalingModifier",
    "ScalingData",
    "scaling_data",
]


@sfe_dataclass
class BoundedFloat:
    min_: float = 0.0
    max_: float = 72.0
    cur_: float = 0.0

    def __post_init__(self):
        self.clamp()

    def clamp(self):
        self.cur_ = min(max(self.min_, self.cur_), self.max_)

    def scale(self, value: float, auto_clamp=True):
        self.cur_ *= value
        if auto_clamp:
            self.clamp()

    def assign(self, value, auto_clamp=True):
        self.cur_ = value
        if auto_clamp:
            self.clamp()


@sfe_dataclass
class ScalingModifier:
    gamestage: BoundedFloat
    awesome_level: float = 0.0


def _default_dict_factory() -> Dict[str, ScalingModifier]:
    return {
        GameRegion.Arid.value: ScalingModifier(BoundedFloat(3.0, 72.0)),
        GameRegion.Dahl.value: ScalingModifier(BoundedFloat(18.0, 72.0)),
        GameRegion.Scrap.value: ScalingModifier(BoundedFloat(23.0, 72.0)),
        GameRegion.Thor.value: ScalingModifier(BoundedFloat(29.0, 72.0)),
        GameRegion.DLC1.value: ScalingModifier(BoundedFloat(15.0, 72.0)),
        GameRegion.DLC2.value: ScalingModifier(BoundedFloat(10.0, 72.0)),
        GameRegion.DLC3.value: ScalingModifier(BoundedFloat(15.0, 72.0)),
        GameRegion.DLC4.value: ScalingModifier(BoundedFloat(15.0, 72.0)),
    }


@sfe_dataclass
class ScalingData:
    version: tuple[int, int] = field(default=(1, 0))
    region_scaling: Dict[str, ScalingModifier] \
        = field(default_factory=_default_dict_factory)


_dgs_save_data: PersistentData[ScalingData] | None = None


def scaling_data() -> ScalingData:
    return _dgs_save_data.value
