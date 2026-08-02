from __future__ import annotations
from typing import TYPE_CHECKING, Any
from dataclasses import dataclass

from unrealsdk import logging
from unrealsdk.unreal import BoundFunction, WeakPointer
from unrealsdk.hooks import Type
from mods_base import build_mod, hook

from .constants import boss_pawn_names, boss_vehicle_names

if TYPE_CHECKING:
    from BL1.WillowGame import (
        WillowAIPawn,
        PopulationFactoryBalancedAIPawn,
        PopulationFactoryWillowVehicle,
        WillowVehicle,
        WillowGameViewportClient,
    )


@dataclass
class BossBarEntity:
    actor: WeakPointer[WillowAIPawn | WillowVehicle] | None

    def get(self) -> WillowAIPawn | WillowVehicle | None:
        return self.actor() if self.actor is not None else None

    def invalidate(self) -> None:
        self.actor = None


_spawned_bosses: list[BossBarEntity] = list()
_unique_names: set[str] = set()


@hook(  # pyright: ignore[reportArgumentType]
    "WillowGame.PopulationFactoryBalancedAIPawn:CreatePopulationActor",
    Type.POST_UNCONDITIONAL,
)
def hook_create_willow_ai(
    obj: PopulationFactoryBalancedAIPawn,
    args: PopulationFactoryBalancedAIPawn.CreatePopulationActorArgs,
    ret: WillowAIPawn | None,
    func: BoundFunction,
) -> None:
    global _unique_names, _spawned_bosses
    if ret is None or ret.ObjectArchetype is None:
        return

    name = str(ret.ObjectArchetype.Name)
    if name not in _unique_names:
        _unique_names.add(name)
        if name in boss_pawn_names():
            _spawned_bosses.append(BossBarEntity(actor=WeakPointer(ret)))
        logging.info(f"[SPAWN_AI] ~ {ret.ObjectArchetype.Name}")


@hook("WillowGame.WillowAIPawn:Died")  # pyright: ignore[reportArgumentType]
def hook_ai_pawn_died(
    obj: WillowAIPawn,
    args: WillowAIPawn.DiedArgs,
    ret: Any,
    func: BoundFunction,
) -> None:
    logging.info(f"[ACTOR_DIED] ~ {obj.ObjectArchetype.Name}")


@hook(  # pyright: ignore[reportArgumentType]
    "WillowGame.PopulationFactoryWillowVehicle:CreatePopulationActor",
    Type.POST_UNCONDITIONAL,
)
def hook_create_willow_vehicle(
    obj: PopulationFactoryWillowVehicle,
    args: PopulationFactoryWillowVehicle.CreatePopulationActorArgs,
    ret: WillowVehicle | None,
    func: BoundFunction,
) -> None:
    global _unique_names, _spawned_bosses
    if ret is None or ret.ObjectArchetype is None:
        return

    name = str(ret.ObjectArchetype.Name)
    if name not in _unique_names:
        _unique_names.add(name)
        if name in boss_vehicle_names():
            _spawned_bosses.append(BossBarEntity(actor=WeakPointer(ret)))
        logging.info(f"[SPAWN_VEHICLE] ~ {ret.ObjectArchetype.Name}")


@hook("WillowGame.WillowVehicle:Died")  # pyright: ignore[reportArgumentType]
def hook_willow_vehicle_died(
    obj: WillowVehicle,
    args: WillowVehicle.DiedArgs,
    ret: Any,
    func: BoundFunction,
) -> None:
    logging.info(f"[VEHICLE_DIED] ~ {obj.ObjectArchetype.Name}")


@hook(  # pyright: ignore[reportArgumentType]
    "WillowGame.WillowGameViewportClient:PostRender",
    Type.POST_UNCONDITIONAL,
)
def hook_render_boss_bars(
    obj: WillowGameViewportClient,
    args: WillowGameViewportClient.PostRenderArgs,
    ret: Any,
    func: BoundFunction,
) -> None:
    global _spawned_bosses

    if (canvas := args.Canvas) is None:
        return

    x: float = 80
    y: float = 48

    text_height = canvas.Font.GetMaxCharHeight()
    tex = canvas.DefaultTexture
    BAR_WIDTH = 128
    BAR_HEIGHT = 9

    for entity in _spawned_bosses:
        if (actor := entity.get()) is None:
            continue

        _, name = actor.GetTargetName("")
        cur_hp = actor.GetHealth()
        max_hp = actor.GetMaxHealth()

        if cur_hp <= 0 or actor.IsDead():
            entity.invalidate()
            continue

        canvas.SetPos(x, y)
        canvas.SetDrawColor(255, 255, 255, 255)
        canvas.DrawText(f"{name}", False, 1.0, 1.0)
        y += text_height

        canvas.SetPos(x, y)
        canvas.SetDrawColor(255, 0, 0, 255)
        canvas.DrawRect(BAR_WIDTH * (cur_hp / max_hp), BAR_HEIGHT, tex)

        canvas.SetPos(x, y)
        canvas.SetDrawColor(200, 200, 200, 255)
        canvas.DrawBox(BAR_WIDTH, BAR_HEIGHT)
        y += text_height


_ = build_mod(
    hooks=(  # pyright: ignore[reportUnknownArgumentType]
        hook_create_willow_ai,
        hook_ai_pawn_died,
        hook_create_willow_vehicle,
        hook_willow_vehicle_died,
        hook_render_boss_bars,
    )
)
