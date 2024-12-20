from enum import Enum
from typing import *
from unrealsdk.unreal import *
from mods_base import get_pc

__all__ = [
    "get_playthrough_index",
    "get_willow_hud",
    "get_globals",
    "get_mission_tracker",
    "get_world_info",
    "get_wgi",
    "get_wgri",
    "get_wpri",
    "EMissionStatus",
    "get_mission_playthrough_data",
    "get_mission_index",
    "get_current_mission",
]


def pc():
    return get_pc()


def get_willow_hud() -> UObject:
    return pc().myHUD


def get_playthrough_index() -> int:
    """ :return: Current Playthrough Index; Literal index"""
    return pc().GetCurrentPlaythrough()


def get_globals() -> UObject:
    """ :return: WillowGlobals """
    return pc().GetWillowGlobals()


def get_mission_tracker() -> UObject:
    """ :return: MissionTracker """
    return pc().WorldInfo.Game.MissionTracker


def get_world_info() -> UObject:
    """ :return: WorldInfo """
    return pc().WorldInfo


def get_wgi() -> UObject:
    """ :return: WillowGameInfo """
    return pc().WorldInfo.Game


def get_wgri() -> UObject:
    """ :return: WillowGameReplicationInfo """
    return pc().WorldInfo.GRI


def get_wpri() -> UObject:
    """ :return: WillowPlayerReplicationInfo """
    return pc().PlayerReplicationInfo


class EMissionStatus(Enum):
    MS_NotStarted = 0
    MS_Active = 1
    MS_ReadyToTurnIn = 2
    MS_Complete = 3
    MS_Redeemed = 4
    MS_MAX = 5


def get_mission_playthrough_data() -> WrappedStruct:
    return pc().MissionPlaythroughData[get_playthrough_index()]


def get_mission_index(mission_def: UObject) -> int:
    """ :return: Mission index of -1 if not found """
    return pc().GetMissionIndexForMission(mission_def)


def get_current_mission() -> Union[tuple[EMissionStatus, UObject], None]:
    """ :return: None or tuple of Mission Status and MissionDefinition """
    info = get_mission_playthrough_data()
    active = info.ActiveMission

    for x in info.MissionList:
        if x.MissionDef == active:
            return EMissionStatus(x.Status), active

    return None
