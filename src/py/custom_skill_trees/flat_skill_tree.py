from __future__ import annotations
from typing import TYPE_CHECKING, cast
from enum import IntEnum
from itertools import chain

from unrealsdk import find_class
from mods_base import ENGINE

if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.WillowGame import (
        PlayerSkillSetDefinition,
        SkillDefinition,
    )


class PlayerCharacter(IntEnum):
    Roland = 0
    Mordecai = 1
    Lilith = 2
    Brick = 3


class FlatSkillTreeLayout:
    action_skill: SkillDefinition
    skills: dict[SkillDefinition, int]

    def __init__(
        self,
        action_skill: SkillDefinition,
        skill_set: dict[SkillDefinition, int],
    ):
        self.action_skill = action_skill
        self.skills = skill_set

    @staticmethod
    def from_skill_set(skill_set: PlayerSkillSetDefinition):
        d: dict[SkillDefinition, int] = {}
        for l, m, r in zip(
            skill_set.LeftBranch.Tiers,
            skill_set.MiddleBranch.Tiers,
            skill_set.RightBranch.Tiers,
        ):
            for i, skill in enumerate(chain(l.Skills, m.Skills, r.Skills)):
                d[skill] = i
        assert skill_set.ActionSkill is not None
        return FlatSkillTreeLayout(skill_set.ActionSkill, d)

    @staticmethod
    def from_char(char: PlayerCharacter):
        from .default_skills import ALL_SKILLS, ACTION_SKILLS

        skill_list = ALL_SKILLS[char.value]
        engine = cast("Object", ENGINE)

        d: dict[SkillDefinition, int] = {}
        for i, skill_ref in enumerate(skill_list):
            skill = engine.DynamicLoadObject(skill_ref, find_class("SkillDefinition"))
            assert skill is not None, f"could not load skill: {skill_ref}"
            d[cast("SkillDefinition", skill)] = i

        action_skill = engine.DynamicLoadObject(
            ACTION_SKILLS[char.value], find_class("SkillDefinition")
        )
        assert action_skill is not None, (
            f"could not load action skill: {ACTION_SKILLS[char.value]}"
        )
        return FlatSkillTreeLayout(cast("SkillDefinition", action_skill), d)
