from __future__ import annotations
from typing import TYPE_CHECKING, cast, Any

from mods_base import build_mod, hook, get_pc
from unrealsdk import find_enum, hooks
from unrealsdk.unreal import WrappedStruct, BoundFunction

from .skill_tree_builder import SKILL_TREE_BUILDER
from .flat_skill_tree import FlatSkillTreeLayout

if TYPE_CHECKING:
    from BL1.WillowGame import (
        StatusMenuExGFxMovie,
        WillowPlayerController,
    )
    from BL1.GFxUI import ASType
else:
    from unrealsdk import find_enum

    ASType = find_enum("ASType")
    ENavActivateAction = find_enum("ENavActivateAction")


def generate_layout_str() -> str:
    from .default_skills import generate_layout_from_skills

    pc = cast("WillowPlayerController", get_pc())
    my_tree = FlatSkillTreeLayout.from_skill_set(pc.PlayerClass.PlayerSkillSet)
    return generate_layout_from_skills(my_tree.action_skill, my_tree.skills)


def invoke_create_skill_tree_from_str(obj: StatusMenuExGFxMovie, layout: str) -> None:
    invoke = obj.Invoke
    invoke_args = cast("StatusMenuExGFxMovie.InvokeArgs", WrappedStruct(invoke.func))
    invoke_args.Method = "create_skill_tree_from_str"
    invoke_args.args.emplace_struct(Type=ASType.AS_String, S=layout)
    _ = invoke(invoke_args)


def _on_show_skill_tree(obj: StatusMenuExGFxMovie) -> None:
    obj.SingleArgInvokeS("skills.gotoAndStop", "custom")

    invoke_create_skill_tree_from_str(obj, generate_layout_str())

    if helper := obj.SkillHelper:
        helper.Flash_SendInitialSkillData()
        if nav_def := helper.CurrentNavDef:
            helper.HandleSelection(nav_def)

        try:
            from skill_tree_tweaks import MOD_INSTANCE as STT_MOD_INST
            from skill_tree_tweaks.hooks import apply_skill_tree_changes

            if STT_MOD_INST.is_enabled:
                apply_skill_tree_changes(obj)
        except ModuleNotFoundError:
            pass


@hook(  # pyright: ignore[reportArgumentType]
    hook_func="WillowGame.StatusMenuExGFxMovie:extSetCurrentScreen",
    hook_type=hooks.Type.POST_UNCONDITIONAL,
)
def hook_set_current_screen(
    obj: StatusMenuExGFxMovie,
    args: StatusMenuExGFxMovie.extSetCurrentScreenArgs,
    _2: Any,
    _3: BoundFunction,
) -> None:
    if args.ScreenName != "skills":
        return
    _on_show_skill_tree(obj)


@hook(  # pyright: ignore[reportArgumentType]
    hook_func="WillowGame.WillowPlayerController:LoadPlayerProfile",
    hook_type=hooks.Type.POST_UNCONDITIONAL,
)
def hook_profile_loaded(
    _1: WillowPlayerController,
    _2: WillowPlayerController.LoadPlayerProfileArgs,
    _3: Any,
    _4: BoundFunction,
) -> None:
    SKILL_TREE_BUILDER.activate()


def _on_enable() -> None:
    from .patch_flash import patch_flash

    patch_flash()
    SKILL_TREE_BUILDER.activate()


MOD_INSTANCE = build_mod(
    hooks=(
        hook_set_current_screen,
        hook_profile_loaded,
    ),
    on_enable=_on_enable,
    options=(*SKILL_TREE_BUILDER.options(),),
)
