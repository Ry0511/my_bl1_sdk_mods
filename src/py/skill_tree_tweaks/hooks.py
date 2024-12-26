from mods_base import hook
from unrealsdk import logging
from unrealsdk.hooks import Type
from unrealsdk.unreal import *

from .helpers import *

__all__ = [
    "hook_skill_tree_changed1",
    "hook_skill_tree_changed2",
    "hook_skill_tree_init",
    "hook_skill_tree_selection_after",
]


################################################################################
# | AUGMENTED SKILL INDICATOR FIX |
################################################################################


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


################################################################################
# | SKILL DESCRIPTION FIX |
################################################################################


@hook(hook_func="WillowGame.SkillTreeGFxHelper:Flash_SetCurrentSelectionDescription", hook_type=Type.POST)
def hook_skill_tree_selection_after(
        obj: UObject,
        args: WrappedStruct,
        __ret,
        __func: BoundFunction
) -> None:
    movie = obj.Movie

    if movie is None:
        return

    new_nav = args.NewNavDef
    skill_index = obj.GetSkillIndexForNavDef(new_nav)
    skill_def = obj.GetSkillDefForSkillIndex(skill_index)

    if skill_index == -1 or skill_def is None:
        return

    augmented_skills = get_augmented_skills_dict(get_current_class_mod())
    skill = obj.WPCOwner.PlayerSkills[skill_index]
    is_unlocked = is_skill_unlocked(skill)

    desc = f"<font size=\"14\">{str(skill_def.SkillDescription).strip()}</font>"
    current_grade_desc = ""
    next_grade_desc = ""

    skill_augment = 0 if skill_def not in augmented_skills else augmented_skills[skill_def]

    current_grade = skill.Grade
    augmented_grade = current_grade + skill_augment


    def get_description(grade: int) -> str:
        nonlocal obj
        nonlocal skill_def
        return (
            obj.GetSkillDescriptionForGrade(
                skill_def,
                grade,
                obj.WPCOwner,
                False
            )
            .strip()
        )


    # Current Grade Effects
    if skill_def in augmented_skills or is_unlocked:
        the_grade = augmented_grade

        if current_grade == 0 and augmented_grade > 0:  # Skill is augmented but not invested into
            current_grade_desc = (
                f"\n<font color=\"#EF3054\" size=\"12\">"
                f"Current Grade\n"
                f"This skill is not active; Invest a skill point to unlock it!\n"
                f"</font>"
            )
        elif current_grade > 0:  # Skill is invested into and maybe augmented
            colour = "#23CE6B" if current_grade >= skill_def.MaxGrade else "#F4D35E"
            current_grade_desc = (
                f"\n<font color=\"{colour}\" size=\"12\">"
                f"Current Grade\n"
                f"{get_description(the_grade)}\n"
                f"</font>"
            )

    # Next Grade Effects
    if skill.Grade < skill_def.MaxGrade:
        the_grade = augmented_grade + 1
        next_grade_desc = (
            f"\n<font color=\"#0ACDFF\" size=\"12\">"
            f"Next Grade\n"
            f"{get_description(the_grade)}\n"
            f"</font>"
        )

    movie.SetVariableString(
        "skills.description.htmlText",
        f"{desc}\n{current_grade_desc}{next_grade_desc}"
    )
