from mods_base import hook
from unrealsdk.unreal import *
from unrealsdk.hooks import Type
from unrealsdk import logging

from .helpers import *

__all__ = [
    "hook_skill_tree_changed1",
    "hook_skill_tree_changed2",
    "hook_skill_tree_init",
]


def apply_skill_tree_changes(movie: UObject):
    modified_skills = get_modified_skills(get_current_class_mod())


    def apply_skill_flash(
            __wpc: UObject,
            player_skill: WrappedStruct,
            skill_nav_def: UObject
    ) -> None:
        nonlocal movie
        nonlocal modified_skills
        skill = player_skill.Definition

        cur_skill_val = player_skill.Grade
        max_skill_val: int = skill.MaxGrade
        skill_augment: int = 0

        for s, v in modified_skills:
            if s == skill:
                skill_augment = v
                break  # NOTE: If there are multiple augments to a single skill this will fail

        p = f"skills.{skill_nav_def.SkillClipName}"
        is_unlocked = is_skill_unlocked(player_skill)

        # Classmod effects augments this skill
        if skill_augment > 0:
            state = "augmented" if is_unlocked else "augmented_locked"
            movie.SingleArgInvokeS(f"{p}.gotoAndStop", state)
            # movie.SingleArgInvokeF(f"{p}.gotoAndStop", 5.0)

        movie.SetVariableBool(f"{p}.points.html", True)

        if is_unlocked:
            movie.SetVariableString(
                f"{p}.points.htmlText",
                f"{cur_skill_val + skill_augment}"
                f"/{max_skill_val + skill_augment}"
            )
        else:
            movie.SetVariableString(f"{p}.points.htmlText", "")


    for_each_skill(apply_skill_flash)


@hook(hook_func="WillowGame.SkillTreeGFxHelper:ArtifactSelect", hook_type=Type.POST)
def hook_skill_tree_changed1(
        obj: UObject,
        __args: WrappedStruct,
        __ret,
        __func: BoundFunction,
) -> None:
    apply_skill_tree_changes(obj.Movie)


@hook(hook_func="WillowGame.SkillTreeGFxHelper:Activate", hook_type=Type.POST)
def hook_skill_tree_changed2(
        obj: UObject,
        __args: WrappedStruct,
        __ret,
        __func: BoundFunction,
) -> None:
    apply_skill_tree_changes(obj.Movie)


@hook(hook_func="WillowGame.SkillTreeGFxHelper:Init", hook_type=Type.POST)
def hook_skill_tree_init(
        obj: UObject,
        __args: WrappedStruct,
        __ret,
        __func: BoundFunction,
) -> None:
    apply_skill_tree_changes(obj.Movie)
