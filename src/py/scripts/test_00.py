from unrealsdk.hooks import *
from unrealsdk.unreal import *

print("Test remove_hook & add_hook")


def hook_func(obj: UObject, args: WrappedStruct, _ret, func: BoundFunction):
    print(f"[SERVER]: {func.func._path_name()}; Obj: {obj._path_name()}")

    node: UField = args._type.Children
    while node is not None and isinstance(node, UProperty):
        node: UProperty
        print(f" > '{node.Offset_Internal}', '{node.ElementSize}', '{node._path_name()}'")
        node = node.Next
    print(args)


def unhook_and_hook_this_fucker(fn: str):
    remove_hook(fn, Type.PRE, f"TEST00_{fn}")
    add_hook(fn, Type.PRE, f"TEST00_{fn}", hook_func)


unhook_and_hook_this_fucker("WillowGame.WillowPlayerController:ServerPlayEchoCall")
unhook_and_hook_this_fucker("WillowGame.WillowPlayerController:ClientPlayEchoCall")

unhook_and_hook_this_fucker("WillowGame.DialogManager:RequestSpeechComponent")
unhook_and_hook_this_fucker("WillowGame.DialogManager:HearSoundFinished")

unhook_and_hook_this_fucker("WillowGame.EmergencyTeleportOutpost:UpdateCollideAsEncroacher")
unhook_and_hook_this_fucker("WillowGame.EmergencyTeleportOutpost:ReplicatedEvent")
unhook_and_hook_this_fucker("WillowGame.EmergencyTeleportOutpost:InitializeFromDefinition")
unhook_and_hook_this_fucker("WillowGame.EmergencyTeleportOutpost:GetInstanceData")

unhook_and_hook_this_fucker("WillowGame.WillowHUD:PostBeginPlay")
unhook_and_hook_this_fucker("WillowGame.WillowHUD:SetWPRI")
unhook_and_hook_this_fucker("WillowGame.WillowHUD:ConvertInputHudCoords")
unhook_and_hook_this_fucker("WillowGame.WillowHUD:PostBeginPlay")

unhook_and_hook_this_fucker("WillowGame.WillowGlobals:AddInteractiveObject")
unhook_and_hook_this_fucker("WillowGame.WillowGlobals:RemoveInteractiveObject")
