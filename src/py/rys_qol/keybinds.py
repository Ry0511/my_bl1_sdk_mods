from __future__ import annotations
from typing import TYPE_CHECKING, cast

from mods_base import keybind, EInputEvent, get_pc
from unrealsdk import make_struct

if TYPE_CHECKING:
    from BL1.WillowGame import WillowPlayerController, WillowHUD

__all__: tuple[str, ...] = (
    "on_save_position",
    "on_restore_position",
    "on_quit_without_saving",
    "on_toggle_hlq_noclip",
    "on_make_op",
)


################################################################################
# | UTILITY |
################################################################################


def display_hud_message(
    title: str,
    msg: str,
    color: tuple[int, int, int, int] = (255, 255, 255, 255),
    duration: float = 1.5,
):
    pc = cast("WillowPlayerController", get_pc())

    hud = cast("WillowHUD | None", pc.myHUD)
    if hud is None:
        return

    hud_movie = hud.GetHUDMovie()
    if hud_movie is None:
        return

    hud_movie.AddTrainingText(
        0,
        msg,
        title,
        duration,
        make_struct(
            "Core.Object.Color", B=color[0], G=color[1], R=color[2], A=color[3]
        ),
        "",
        False,
        0.0,
        None,
        True,
    )


################################################################################
# | SAVE RESTORE POSITION |
################################################################################

# Will probably add more slots
_saved_location: tuple[float, float, float] | None = None  # XYZ
_saved_rotation: tuple[int, int, int] | None = None  # PYR


def is_player_available() -> bool:
    return cast("WillowPlayerController", get_pc()).Pawn is not None


@keybind(identifier="Save Position", key="F7", event_filter=None)
def on_save_position(event: EInputEvent):
    if event is EInputEvent.IE_Released or not is_player_available():
        return

    global _saved_location
    global _saved_rotation
    wpc = cast("WillowPlayerController", get_pc())
    assert wpc.Pawn is not None
    pos = wpc.Pawn.Location
    _saved_location = (pos.X, pos.Y, pos.Z)

    rotation = wpc.CalcViewRotation
    _saved_rotation = (rotation.Pitch, rotation.Yaw, rotation.Roll)

    display_hud_message("Rys QoL", "Position Saved!")


@keybind(identifier="Restore Position", key="F8", event_filter=EInputEvent.IE_Released)
def on_restore_position():
    global _saved_location
    global _saved_rotation

    if (
        not is_player_available()
        or None in (_saved_location, _saved_rotation)
        or _saved_location is None
        or _saved_rotation is None
    ):
        return

    wpc = cast("WillowPlayerController", get_pc())
    pos = make_struct(
        "Core.Object.Vector",
        X=_saved_location[0],
        Y=_saved_location[1],
        Z=_saved_location[2],
    )

    rot = make_struct(
        "Core.Object.Rotator",
        Pitch=_saved_rotation[0],
        Yaw=_saved_rotation[1],
        Roll=_saved_rotation[2],
    )

    pawn = wpc.Pawn
    wpc.ClientSetPawnLocation(pawn, pos, rot)
    wpc.ClientSetRotation(rot, True)

    display_hud_message("Rys QoL", "Position Restored!")


################################################################################
# | QUIT WITHOUT SAVING |
################################################################################


@keybind(identifier="Quit Without Saving", key="F9")
def on_quit_without_saving():
    if not is_player_available():
        return

    wpc = cast("WillowPlayerController", get_pc())
    wpc.DestroyOnlineGame(False)
    _ = wpc.ConsoleCommand("open menumap", False)


################################################################################
# | CHEAT FEATURES |
################################################################################


@keybind(
    identifier="Toggle HLQ Noclip",
    description="Gives noclip (its faster than ghost mode)",
)
def on_toggle_hlq_noclip():
    if not is_player_available():
        return

    display_hud_message("Rys QoL", "Toggle HLQ Noclip!")
    cast("WillowPlayerController", get_pc()).ServerToggleHLQ()


@keybind(
    identifier="Make me overpowered",
    description="Makes the user level 69 and gives them some good gear;"
    + " This deletes your inventory.",
)
def on_make_op():
    if not is_player_available():
        return

    display_hud_message("Rys QoL", "Making Player Max Level & Giving Gear!")
    cast("WillowPlayerController", get_pc()).ServerBalanceMe(100, 999)
