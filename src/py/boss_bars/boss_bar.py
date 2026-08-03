from __future__ import annotations
from typing import TYPE_CHECKING, Protocol
from collections.abc import Sequence
from dataclasses import dataclass
from enum import StrEnum

if TYPE_CHECKING:
    from BL1.Engine import Canvas


@dataclass
class ClampedValue:
    cur_val: float
    max_val: float

    def __post_init__(self) -> None:
        self.cur_val = max(min(self.max_val, self.cur_val), 0)
        self.max_val = max(self.max_val, 1)

    def normalised(self) -> float:
        return self.cur_val / self.max_val


@dataclass
class EntityState:
    name: str
    health: ClampedValue
    shield: ClampedValue | None = None


# fmt: off
DEBUG_ENTITY_STATE = (
    EntityState(name="Test the Invincible", health=ClampedValue(25, 100)),
    EntityState(name="Test the Invincible", health=ClampedValue(50, 100)),
    EntityState(name="Test the Invincible", shield=(ClampedValue(5, 100)), health=ClampedValue(75, 100)),
    EntityState(name="Test the Invincible", shield=(ClampedValue(100, 100)), health=ClampedValue(100, 100)),
    EntityState(name="Test the Invincible", shield=(ClampedValue(66, 100)), health=ClampedValue(10, 100)),
)
# fmt: on


class IBossBarStrategy(Protocol):
    def draw(self, canvas: Canvas, entities: Sequence[EntityState]) -> None: ...


class BossBarStrategy(StrEnum):
    Minimal = "Minimal"
    Decorative = "Decorative"


def create_boss_bar_strategy(strategy: BossBarStrategy) -> IBossBarStrategy:
    match strategy:
        case BossBarStrategy.Minimal:
            from .minimal_boss_bar import MinimalBossBar

            return MinimalBossBar()

        case BossBarStrategy.Decorative:
            raise NotImplementedError(strategy)
