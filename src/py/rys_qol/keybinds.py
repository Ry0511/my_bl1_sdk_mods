import mods_base
from mods_base import keybind, EInputEvent, get_pc

import unrealsdk
from unrealsdk.unreal import WrappedStruct

__all__: [str] = [
    "on_save_position",
    "on_restore_position",
    "on_quit_without_saving",
    "on_toggle_ghost",
    "on_toggle_hlq_noclip",
    "on_make_op",
]


################################################################################
# | UTILITY |
################################################################################

def display_hud_message(title: str, msg: str, color=(255, 255, 255, 255), duration: float = 1.5):
    pc = get_pc()

    hud = pc.myHUD
    if hud is None:
        return

    hud_movie = hud.GetHUDMovie()
    if hud_movie is None:
        return

    color_struct = unrealsdk.make_struct(
        "Core.Object.Color",
        B=color[0],
        G=color[1],
        R=color[2],
        A=color[3]
    )

    hud_movie.AddTrainingText(0, msg, title, duration, color_struct, "", False, 0.0, None, True)


################################################################################
# | SAVE RESTORE POSITION |
################################################################################

# Will probably add more slots
_saved_location: tuple[float, float, float] | None = None  # XYZ
_saved_rotation: tuple[int, int, int] | None = None  # PYR


def is_player_available() -> bool:
    pc = mods_base.get_pc()
    return pc is not None and pc.Pawn is not None


@keybind(identifier="Save Position", key="F7", event_filter=None)
def on_save_position(event: EInputEvent):
    if event is EInputEvent.IE_Released or not is_player_available():
        return

    global _saved_location
    global _saved_rotation
    wpc = get_pc()
    pawn = wpc.Pawn

    pos: WrappedStruct = pawn.Location
    _saved_location = (pos.X, pos.Y, pos.Z)

    rotation = wpc.CalcViewRotation
    _saved_rotation = (rotation.Pitch, rotation.Yaw, rotation.Roll)

    display_hud_message("Rys QoL", "Position Saved!")


@keybind(identifier="Restore Position", key="F8", event_filter=EInputEvent.IE_Released)
def on_restore_position():
    global _saved_location
    global _saved_rotation

    if not is_player_available() or None in (_saved_location, _saved_rotation):
        return

    wpc = get_pc()
    pos = unrealsdk.make_struct(
        "Core.Object.Vector",
        X=_saved_location[0],
        Y=_saved_location[1],
        Z=_saved_location[2]
    )

    rot = unrealsdk.make_struct(
        "Core.Object.Rotator",
        Pitch=_saved_rotation[0],
        Yaw=_saved_rotation[1],
        Roll=_saved_rotation[2],
    )

    pawn = wpc.Pawn
    wpc.ClientSetPawnLocation(pawn, pos, rot)
    wpc.ClientSetRotation(rot)

    display_hud_message("Rys QoL", "Position Restored!")


################################################################################
# | QUIT WITHOUT SAVING |
################################################################################


@keybind(identifier="Quit Without Saving", key="F9")
def on_quit_without_saving():
    if not is_player_available():
        return

    wpc = get_pc()
    wpc.DestroyOnlineGame(False)
    wpc.ConsoleCommand("open menumap")


################################################################################
# | CHEAT FEATURES |
################################################################################

@keybind(
    identifier="Toggle Ghost Mode",
    description="Gives noclip not sure how it differs from HLQ"
                " maybe enemies don't see you?",
)
def on_toggle_ghost():
    if not is_player_available():
        return

    display_hud_message("Rys QoL", "Toggle Ghost Mode!")
    get_pc().ServerToggleGhost()


@keybind(
    identifier="Toggle HLQ Noclip",
    description="Gives noclip (its faster than ghost mode)",
)
def on_toggle_hlq_noclip():
    if not is_player_available():
        return

    display_hud_message("Rys QoL", "Toggle HLQ Noclip!")
    get_pc().ServerToggleHLQ()


@keybind(
    identifier="Make me overpowered",
    description="Makes the user level 69 and gives them some good gear;"
                " This deletes your inventory.",
)
def on_make_op():
    if not is_player_available():
        return

    display_hud_message("Rys QoL", "Making Player Max Level & Giving Gear!")
    get_pc().ServerBalanceMe(100, 100)
