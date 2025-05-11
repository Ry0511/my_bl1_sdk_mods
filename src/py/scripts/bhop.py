import unrealsdk  # type: ignore
from typing import Any, Type
from unrealsdk import logging, make_struct, find_class  # type: ignore
from unrealsdk.hooks import Type, add_hook, remove_hook, Block  # type: ignore
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction, UScriptStruct, UArrayProperty, UFunction, WeakPointer  # type: ignore
from mods_base import hook, get_pc, ENGINE  # type: ignore
import os
import math

# get_pc().Pawn.SetHealth(10)

# get_pc().SkillCooldownPool.Data.SetCurrentValue(0.0000000)

CurrentLocation: WrappedStruct = make_struct("Vector")
CheckLocation: WrappedStruct = make_struct("Vector")

# def CreatePopulationActor(obj: UObject, args: WrappedStruct, ret: any, func: BoundFunction):
#    print("CreatePopulationActor")
#    return
# remove_hook("WillowGame.PopulationFactoryBalancedAIPawn:CreatePopulationActor", Type.PRE, "CreatePopulationActor")
# add_hook("WillowGame.PopulationFactoryBalancedAIPawn:CreatePopulationActor", Type.PRE, "CreatePopulationActor", CreatePopulationActor)
#
# def RestorePopulatedAIPawn(obj: UObject, args: WrappedStruct, ret: any, func: BoundFunction):
#    print("RestorePopulatedAIPawn")
#    return
# remove_hook("WillowGame.PopulationFactoryBalancedAIPawn:RestorePopulatedAIPawn", Type.PRE, "RestorePopulatedAIPawn")
# add_hook("WillowGame.PopulationFactoryBalancedAIPawn:RestorePopulatedAIPawn", Type.PRE, "RestorePopulatedAIPawn", RestorePopulatedAIPawn)
#


FakeWeapon: WeakPointer = None


def CanBHop(PawnToCheck) -> bool:
    global FakeWeapon
    if not FakeWeapon or not FakeWeapon():
        NewWeapon = get_pc().Spawn(find_class("WillowWeapon"))
        FakeWeapon = WeakPointer(NewWeapon)

    global CurrentLocation, CheckLocation
    PawnLocation = PawnToCheck.Location

    CurrentLocation.X = PawnLocation.X
    CurrentLocation.Y = PawnLocation.Y
    CurrentLocation.Z = PawnLocation.Z - PawnToCheck.GetCollisionHeight()

    CheckLocation.X = PawnLocation.X
    CheckLocation.Y = PawnLocation.Y
    CheckLocation.Z = PawnLocation.Z - 1000

    # from tracelib

    TraceInfo = FakeWeapon().CalcWeaponFire(StartTrace=CurrentLocation, EndTrace=CheckLocation)[0]

    return TraceInfo.HitActor and CurrentLocation.Z - TraceInfo.HitLocation.Z < 25


# @hook("WillowGame.WillowPlayerPawn:CanStuckJump", Type.PRE)
def CanStuckJump(obj: UObject, __args: WrappedStruct, __ret: Any, __func: BoundFunction) -> tuple[Type[Block], bool] | None:
    if obj.Physics == 2 and CanBHop(obj):
        return Block, True


remove_hook("WillowGame.WillowPlayerPawn:CanStuckJump", Type.PRE, "CanStuckJump")
add_hook("WillowGame.WillowPlayerPawn:CanStuckJump", Type.PRE, "CanStuckJump", CanStuckJump)


def magnitude(self) -> float:
    """Return the magnitude of this vector."""
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)


def test(obj: UObject, args: WrappedStruct, ret: any, func: BoundFunction):
    if not get_pc().Pawn:
        return

    speed = magnitude(get_pc().Pawn.Velocity)

    value = str(int(speed))
    TextToDraw = f"Speed: {value}"
    canvas = args.Canvas
    canvas.Font = unrealsdk.find_object("Font", "ui_fonts.font_willowbody_18pt")
    white = unrealsdk.make_struct("Color", R=255, G=255, B=255, A=255)
    canvas.SizeX = 10
    canvas.SizeY = 10
    canvas.DrawColor = white
    canvas.DrawText(TextToDraw, True, 1.7, 1.7)

    return


remove_hook("Engine.GameViewportClient:PostRender", Type.POST, "test")
add_hook("Engine.GameViewportClient:PostRender", Type.POST, "test", test)
