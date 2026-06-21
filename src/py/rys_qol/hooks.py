from __future__ import annotations
from typing import Any, TYPE_CHECKING, cast
from functools import cmp_to_key

from unrealsdk import construct_object, logging
from unrealsdk.hooks import Type
from unrealsdk.unreal import (
    UObject,
    WrappedStruct,
    BoundFunction,
)
from mods_base import hook, get_pc

from .registration_list import compare_outpost
from .options import should_sort_fast_travels, fix_auto_mission_select

if TYPE_CHECKING:
    from BL1.WillowGame import (
        WillowPlayerController,
        WillowCheatManager,
        MissionDefinition,
        EMissionStatus,
        MissionTracker,
        MissionStatus,
        RegistrationStationGFxMovie,
        WaypointDefinition,
        WillowGameReplicationInfo,
        PlayerProfile,
    )

__all__: tuple[str, ...] = (
    "on_player_loaded",
    "hook_fix_auto_mission_select",
    "hook_sort_fast_travels",
    "hook_sort_fast_travels_2",
    "hook_indent_fast_travel_tab_changed",
    "hook_fast_travel_indent",
)


def _get_current_mission() -> (
    tuple[EMissionStatus | int, MissionDefinition | None] | None
):
    pc = cast(WillowPlayerController, get_pc())
    data = pc.MissionPlaythroughData[pc.GetCurrentPlaythrough()]
    active: MissionDefinition | None = data.ActiveMission
    for x in data.MissionList:
        if x.MissionDef == active:
            return x.Status, active
    return None


# Index + Mission Name
_mission_on_load: tuple[int, str] | None = None


@hook(
    hook_func="WillowGame.WillowPlayerController:SpawningProcessComplete",
    hook_type=Type.POST,
)
def on_player_loaded(
    obj: UObject,
    _args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _func: BoundFunction,
) -> None:
    # This removes the 30s timer required for saving
    obj = cast(WillowPlayerController, obj)
    obj.ClientSetProfileLoaded()

    if obj.CheatManager is None:
        obj.CheatManager = cast(
            WillowCheatManager, construct_object("WillowGame.WillowCheatManager", obj)
        )

    if not fix_auto_mission_select.value:
        return

    pc: WillowPlayerController = cast(WillowPlayerController, get_pc())

    global _mission_on_load

    mission = _get_current_mission()

    if mission is None or mission[1] is None:
        _mission_on_load = None
    else:
        _, d = mission
        index = pc.GetMissionIndexForMission(d)
        if index == -1:
            _mission_on_load = None
        elif d is not None:
            _mission_on_load = index, str(d.MissionName)


@hook(
    hook_func="WillowGame.WillowPlayerController:SetActiveMission", hook_type=Type.POST
)
def hook_fix_auto_mission_select(
    obj: UObject,
    args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _func: BoundFunction,
) -> None:
    obj = cast(WillowPlayerController, obj)
    if not fix_auto_mission_select.value:
        return

    pc: WillowPlayerController = cast(WillowPlayerController, get_pc())
    assert pc.WorldInfo is not None and pc.WorldInfo.Game is not None
    tracker = cast(MissionTracker, pc.WorldInfo.Game.MissionTracker)
    in_mission: MissionDefinition = args.InMission  # pyright: ignore[reportAny]
    in_status = EMissionStatus(tracker.GetMissionStatus(in_mission))
    invalid_states = (EMissionStatus.MS_Redeemed, EMissionStatus.MS_Complete)

    if in_status not in invalid_states:
        return

    data = pc.MissionPlaythroughData[pc.GetCurrentPlaythrough()]

    global _mission_on_load
    if _mission_on_load is None:
        return

    index, name = _mission_on_load
    if 0 <= index < len(data.MissionList):
        data_mission: MissionStatus = cast(MissionStatus, data.MissionList[index])
        assert data_mission.MissionDef is not None
        if data_mission.MissionDef.MissionName == name:
            data.ActiveMission = data_mission.MissionDef
            obj.UpdateLCDMissionStatus()
            obj.PulseWaypoints()
            return  # We don't want to modify _mission_on_load if its value is valid

    _mission_on_load = None


@hook(hook_func="WillowGame.WillowPlayerController:ApplyVisitedTeleporterData")
def hook_sort_fast_travels(
    _obj: UObject,
    args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _func: BoundFunction,
) -> None:
    # NOTE: This sorts as the profile/save is loaded
    if not should_sort_fast_travels.value:
        return

    xs = cast(PlayerProfile, args.Profile).VisitedTeleporters
    xs.sort(key=cmp_to_key(compare_outpost))


@hook(hook_func="WillowGame.RegistrationStationGFxMovie:HandleOpen")
def hook_sort_fast_travels_2(
    obj: UObject,
    _args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _func: BoundFunction,
) -> None:
    obj = cast(RegistrationStationGFxMovie, obj)
    # This sorts everytime you open the Fast Travel station
    if not should_sort_fast_travels.value:
        return

    wp = obj.WPlayerOwner

    if wp is None:
        logging.warning(
            "WPlayerOwner is None or ActivatedTeleportersList is None;"
            + " Fast Travel may not be sorted or indented"
        )
        return

    # Since this is shared/replicated for multiplayer, we need to modify its source if we want a
    # sane ordering.
    assert wp.WorldInfo is not None
    assert wp.WorldInfo.GRI is not None
    locations = cast(WillowGameReplicationInfo, wp.WorldInfo.GRI).FastTravelLocations
    locations.sort(key=cmp_to_key(compare_outpost))

    # Changes the rate at which OnTick is called; N times a second.
    obj.TickRateSeconds = 1.0 / 80.0


@hook(hook_func="WillowGame.RegistrationStationGFxMovie:OnTick")
def hook_fast_travel_indent(
    obj: UObject,
    _args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportAny, reportExplicitAny]
    _func: BoundFunction,
) -> None:
    obj = cast(RegistrationStationGFxMovie, obj)
    if not should_sort_fast_travels.value:
        return

    helper = obj.FastTravelHelper

    if helper is None or not obj.FastTravelEnabled:
        return

    if len(helper.Locations) == 0:
        return

    # Hacky but if we detect an indented item then we can skip this
    for loc in helper.Locations:
        if loc.DisplayName.startswith("    "):
            return

    # Once the location data is available we can revert back to normal tick-rate
    obj.TickRateSeconds = 1.0

    from .registration_list import get_level_name_from_outpost_path

    pc = cast(WillowPlayerController, get_pc())
    globals = pc.GetWillowGlobals()
    assert globals is not None
    lookup = globals.GetRegistrationStationLookup()
    cur_mission = _get_current_mission()

    def is_active_mission_location(location: str | None) -> bool:
        nonlocal lookup
        nonlocal cur_mission

        if cur_mission is None:
            return False

        status, mission = cur_mission
        waypoint: WaypointDefinition | None = None
        assert mission is not None

        if status is EMissionStatus.MS_ReadyToTurnIn:
            waypoint = mission.TurnInWaypointDefinition
        else:
            waypoint = mission.TargetWaypointDefinition

        if waypoint is None:
            return False

        active_loc = str(waypoint.PersistentLevelName)
        assert lookup is not None
        path_name = lookup.GetPathName(location)
        lvl_name = get_level_name_from_outpost_path(path_name)
        return lvl_name is not None and lvl_name == active_loc

    locations = helper.Locations

    header_outposts = [
        "Fyrestone",
        "JakobsCove",
        "Coliseum",
        "TBoneJunc",
        "TartarusStation",
        "Oasis",
    ]

    from .registration_list import ALL_WITHOUT_CHECKPOINTS

    outpost_count = len(ALL_WITHOUT_CHECKPOINTS)
    for i, loc in enumerate(locations):
        if i > outpost_count:
            helper.SendLocationData()
            return  # Since we push unknown entries to the bottom we can short-circuit here
        clean_name = loc.DisplayName.lstrip(" *").rstrip()
        if str(loc.OutpostName) not in header_outposts:
            if is_active_mission_location(loc.OutpostName):
                loc.DisplayName = f"  * {clean_name}"
            else:
                loc.DisplayName = f"    {clean_name}"
        elif is_active_mission_location(loc.OutpostName):
            loc.DisplayName = f"* {clean_name}"

    helper.SendLocationData()


@hook(hook_func="WillowGame.RegistrationStationGFxMovie:extSetTab", hook_type=Type.POST)
def hook_indent_fast_travel_tab_changed(
    obj: UObject,
    args: WrappedStruct,
    _ret: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _func: BoundFunction,
) -> None:
    obj = cast(RegistrationStationGFxMovie, obj)
    if not should_sort_fast_travels.value:
        return

    if cast(str, args.NewTab).lower() == "teleport":
        obj.TickRateSeconds = 1.0 / 80.0
