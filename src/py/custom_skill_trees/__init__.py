# pyright: reportOptionalMemberAccess=false
# pyright: reportAny=false
# pyright: reportExplicitAny=false

from __future__ import annotations
from typing import TYPE_CHECKING, cast, Any

from mods_base import build_mod, hook, keybind, get_pc
from unrealsdk import find_enum
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction

from .flat_skill_tree import FlatSkillTreeLayout, PlayerCharacter

if TYPE_CHECKING:
    from BL1.WillowGame import StatusMenuExGFxMovie, WillowPlayerController
    from BL1.GFxUI import ASType
else:
    from unrealsdk import find_enum

    ASType = find_enum("ASType")
    ENavActivateAction = find_enum("ENavActivateAction")


def generate_layout_str() -> str:
    pc = cast("WillowPlayerController", get_pc())
    assert pc.PlayerClass.PlayerSkillSet is not None
    my_tree = FlatSkillTreeLayout.from_skill_set(pc.PlayerClass.PlayerSkillSet)
    layout = ""

    def _outermost(obj: UObject) -> UObject:
        outermost: UObject | None = obj
        while outermost.Outer is not None:  # pyright: ignore[reportUnnecessaryComparison]
            outermost = outermost.Outer
        return outermost  # pyright: ignore[reportUnreachable]

    skill_mapping: dict[str, tuple[str, FlatSkillTreeLayout]] = {
        "gd_skills2_roland": (
            "R",
            FlatSkillTreeLayout.from_char(PlayerCharacter.Roland),
        ),
        "gd_skills2_mordecai": (
            "M",
            FlatSkillTreeLayout.from_char(PlayerCharacter.Mordecai),
        ),
        "gd_skills2_lilith": (
            "L",
            FlatSkillTreeLayout.from_char(PlayerCharacter.Lilith),
        ),
        "gd_skills2_brick": (
            "B",
            FlatSkillTreeLayout.from_char(PlayerCharacter.Brick),
        ),
    }

    ref_char, _ = skill_mapping[(str(_outermost(my_tree.action_skill).Name.lower()))]
    layout += ref_char

    for skill in my_tree.skills:
        ref_char, tree = skill_mapping[(str(_outermost(skill).Name.lower()))]
        index = tree.skills[skill]
        layout += f"{ref_char}{chr(ord('a') + index)}"

    # example: RBiRdRrRuLdMgBcBjMhBuRfBkMnLmRgReLoLaBtRlLr
    # default skill trees are predictable: RRaRbRcRd ...
    assert len(layout) == 43, "invalid skill tree layout string"
    return layout


def _on_show_skill_tree(obj: StatusMenuExGFxMovie) -> None:
    obj.SingleArgInvokeS("skills.gotoAndStop", "custom")

    invoke = cast(BoundFunction, obj.Invoke)
    invoke_args = WrappedStruct(invoke.func)
    invoke_args.Method = "create_skill_tree_from_str"
    invoke_args.args.emplace_struct(Type=ASType.AS_String, S=generate_layout_str())
    _ = invoke(invoke_args)


@hook(hook_func="WillowGame.StatusMenuExGFxMovie:extSetCurrentScreen")
def hook_set_current_screen(
    obj: UObject,
    args: WrappedStruct,
    _2: Any,
    _3: BoundFunction,
) -> None:
    if args.ScreenName != "skills":
        return

    _on_show_skill_tree(cast("StatusMenuExGFxMovie", obj))


def _on_enable() -> None:
    from .patch_flash import patch_flash

    patch_flash()


@keybind(identifier="Reload Flash File", key="F12")
def _reload_flash_file():
    _on_enable()


_ = build_mod(
    hooks=(hook_set_current_screen,),
    keybinds=(_reload_flash_file,),
    on_enable=_on_enable,
)
