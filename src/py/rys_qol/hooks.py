from unrealsdk import construct_object, logging
from unrealsdk.hooks import Type
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction, WrappedArray, UNameProperty
from mods_base import hook

from functools import cmp_to_key
from .registration_list import compare_outpost

from .options import _should_sort_fast_travels

__all__: [str] = [
    "on_player_loaded",
    "hook_sort_fast_travels",
    "hook_sort_fast_travels_2",
    "hook_fast_travel_indent",
]


@hook(
    hook_func="WillowGame.WillowPlayerController:SpawningProcessComplete",
    hook_type=Type.POST,
)
def on_player_loaded(
        obj: UObject,
        __args: WrappedStruct,
        __ret: any,
        __func: BoundFunction,
) -> None:
    # This removes the 30s timer required for saving
    obj.ClientSetProfileLoaded()

    if obj.CheatManager is None:
        obj.CheatManager = construct_object("WillowGame.WillowCheatManager", obj)


@hook(hook_func="WillowGame.WillowPlayerController:ApplyVisitedTeleporterData")
def hook_sort_fast_travels(
        __obj: UObject,
        __args: WrappedStruct,
        __ret: any,
        __func: BoundFunction,
) -> None:
    # NOTE: This sorts as the profile/save is loaded
    if not _should_sort_fast_travels.value:
        return

    xs: WrappedArray[UNameProperty] = __args.Profile.VisitedTeleporters
    xs.sort(key=cmp_to_key(compare_outpost))


_has_applied_indents = False  # This is to avoid constantly modifying the list


@hook(hook_func="WillowGame.RegistrationStationGFxMovie:HandleOpen")
def hook_sort_fast_travels_2(
        obj: UObject,
        __args: WrappedStruct,
        __ret: any,
        __func: BoundFunction,
) -> None:
    # This sorts everytime you open the Fast Travel station
    if not _should_sort_fast_travels.value:
        return

    wp = obj.WPlayerOwner

    if wp is None or wp.ActivatedTeleportersList is None:
        logging.warning("WPlayerOwner is None or ActivatedTeleportersList is None;"
                        " Fast Travel may not be sorted or indented")
        return

    # Since this is shared/replicated for multiplayer we need to modify its source if we want a
    # sane ordering.
    locations = wp.WorldInfo.GRI.FastTravelLocations
    locations.sort(key=cmp_to_key(compare_outpost))

    logging.info("Queued Fast Travel Indenting")
    global _has_applied_indents
    _has_applied_indents = False
    # Changes the rate at which OnTick is called; N times a second here.
    obj.TickRateSeconds = 1.0 / 80.0


@hook(hook_func="WillowGame.RegistrationStationGFxMovie:OnTick")
def hook_fast_travel_indent(
        obj: UObject,
        __args: WrappedStruct,
        __ret: any,
        __func: BoundFunction,
) -> None:
    if not _should_sort_fast_travels.value:
        return

    helper = obj.FastTravelHelper

    if helper is None or not obj.FastTravelEnabled:
        return

    if len(helper.Locations) == 0:
        return

    # Once the location data is available we can revert back to normal tick-rate
    obj.TickRateSeconds = 1.0
    global _has_applied_indents
    if _has_applied_indents:
        return

    locations: WrappedArray[WrappedStruct] = helper.Locations

    header_outposts = ["Fyrestone", "JakobsCove", "Coliseum", "TBoneJunc", "TartarusStation", "Oasis"]
    from .registration_list import ALL_WITHOUT_CHECKPOINTS
    outpost_count = len(ALL_WITHOUT_CHECKPOINTS)
    for i, loc in enumerate(locations):
        if i > outpost_count:
            return  # Since we push unknown entries to the bottom we can short-circuit here
        if str(loc.OutpostName) not in header_outposts:
            loc.DisplayName = f"    {loc.DisplayName.strip()}"

    helper.SendLocationData()
    logging.info("Applied Fast Travel Indents")
    _has_applied_indents = True
