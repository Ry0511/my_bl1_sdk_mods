from typing import Union
from functools import cmp_to_key

from unrealsdk import construct_object, logging
from unrealsdk.hooks import Type
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction, WrappedArray, UNameProperty
from mods_base import hook

from .registration_list import compare_outpost

from .options import _should_sort_fast_travels, _fix_auto_mission_select

__all__: [str] = [
    "on_player_loaded",
    "hook_fix_auto_mission_select",
    "hook_sort_fast_travels",
    "hook_sort_fast_travels_2",
    "hook_indent_fast_travel_tab_changed",
    "hook_fast_travel_indent",
]

# Index + Mission Name
_mission_on_load: Union[tuple[int, str], None] = None


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

    if not _fix_auto_mission_select.value:
        return

    from .willow_player_helper import get_current_mission, get_mission_index
    global _mission_on_load

    mission = get_current_mission()

    if mission is None or mission[1] is None:
        _mission_on_load = None
    else:
        _, d = mission
        index = get_mission_index(d)
        if index == -1:
            _mission_on_load = None
        else:
            _mission_on_load = index, str(d.MissionName)


@hook(hook_func="WillowGame.WillowPlayerController:SetActiveMission", hook_type=Type.POST)
def hook_fix_auto_mission_select(
        obj: UObject,
        args: WrappedStruct,
        __ret: any,
        __func: BoundFunction,
) -> None:
    if not _fix_auto_mission_select.value:
        return

    from .willow_player_helper import get_mission_tracker, EMissionStatus, get_mission_playthrough_data

    tracker = get_mission_tracker()
    in_mission = args.InMission
    in_status = EMissionStatus(tracker.GetMissionStatus(in_mission))
    invalid_states = (EMissionStatus.MS_Redeemed, EMissionStatus.MS_Complete)

    if in_status not in invalid_states:
        return

    data = get_mission_playthrough_data()

    global _mission_on_load
    if _mission_on_load is None:
        return

    index, name = _mission_on_load
    if 0 <= index < len(data.MissionList):
        data_mission = data.MissionList[index]
        if data_mission.MissionDef.MissionName == name:
            data.ActiveMission = data_mission.MissionDef
            obj.UpdateLCDMissionStatus()
            obj.PulseWaypoints()
            return  # We don't want to modify _mission_on_load if its value is valid

    _mission_on_load = None


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

    # Changes the rate at which OnTick is called; N times a second.
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

    # Hacky but if we detect an indented item then we can skip this
    for loc in helper.Locations:
        if loc.DisplayName.startswith("    "):
            return

    # Once the location data is available we can revert back to normal tick-rate
    obj.TickRateSeconds = 1.0

    from .registration_list import get_level_name_from_outpost_path
    from .willow_player_helper import get_globals, get_current_mission, EMissionStatus
    lookup = get_globals().GetRegistrationStationLookup()
    cur_mission = get_current_mission()


    def is_active_mission_location(location: UNameProperty) -> bool:
        nonlocal lookup
        nonlocal cur_mission

        if cur_mission is None:
            return False

        status, mission = cur_mission
        waypoint: WrappedStruct | None = None

        if status is EMissionStatus.MS_ReadyToTurnIn:
            waypoint = mission.TurnInWaypointDefinition
        else:
            waypoint = mission.TargetWaypointDefinition

        if waypoint is None:
            return False

        active_loc = str(waypoint.PersistentLevelName)
        path_name = lookup.GetPathName(location)
        lvl_name = get_level_name_from_outpost_path(path_name)
        return lvl_name is not None and lvl_name == active_loc


    locations: WrappedArray[WrappedStruct] = helper.Locations

    header_outposts = ["Fyrestone", "JakobsCove", "Coliseum",
                       "TBoneJunc", "TartarusStation", "Oasis"]

    from .registration_list import ALL_WITHOUT_CHECKPOINTS
    outpost_count = len(ALL_WITHOUT_CHECKPOINTS)
    for i, loc in enumerate(locations):
        if i > outpost_count:
            helper.SendLocationData()
            return  # Since we push unknown entries to the bottom we can short-circuit here
        clean_name = loc.DisplayName.lstrip(f' *').rstrip()
        if str(loc.OutpostName) not in header_outposts:

            if is_active_mission_location(loc.OutpostName):
                loc.DisplayName = f"  * {clean_name}"
            else:
                loc.DisplayName = f"    {clean_name}"
        elif is_active_mission_location(loc.OutpostName):
            loc.DisplayName = f"* {clean_name}"

    helper.SendLocationData()

    # Leaving this here since this can be used to color the text but its kinda buggy
    # obj.SingleArgInvokeS(f"teleport.selections.inicon1.gotoAndStop", "arrow")  # Shows the arrow
    # for i in range(1, 18):
    #     s = obj.GetVariableString(f"teleport.selections.loc{i}.text")
    #     if not str(s).startswith("  * "):
    #         continue
    #     obj.SetVariableString(f"teleport.selections.loc{i}.htmlText",
    #                           f"<font color=\"#043565\">{s}</font>")


@hook(hook_func="WillowGame.RegistrationStationGFxMovie:extSetTab", hook_type=Type.POST)
def hook_indent_fast_travel_tab_changed(
        obj: UObject,
        args: WrappedStruct,
        __ret: any,
        __func: BoundFunction,
) -> None:
    if not _should_sort_fast_travels.value:
        return

    if str(args.NewTab).lower() == "teleport":
        obj.TickRateSeconds = 1.0 / 80.0
