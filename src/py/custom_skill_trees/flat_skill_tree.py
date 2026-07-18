from __future__ import annotations
from typing import TYPE_CHECKING, cast
from enum import IntEnum
from itertools import chain

from unrealsdk import find_class
from mods_base import ENGINE

if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.WillowGame import PlayerSkillSetDefinition, SkillDefinition


class PlayerCharacter(IntEnum):
    Roland = 0
    Mordecai = 1
    Lilith = 2
    Brick = 3


class FlatSkillTreeLayout:
    skills: dict[SkillDefinition, int]

    def __init__(self, skill_set: dict[SkillDefinition, int]):
        self.skills = skill_set

    @staticmethod
    def from_skill_set(skill_set: PlayerSkillSetDefinition):
        d: dict[SkillDefinition, int] = {}
        for l, m, r in zip(  # noqa: E741
            skill_set.LeftBranch.Tiers,
            skill_set.MiddleBranch.Tiers,
            skill_set.RightBranch.Tiers,
        ):
            for i, skill in enumerate(chain(l.Skills, m.Skills, r.Skills)):
                d[skill] = i
        return FlatSkillTreeLayout(d)

    @staticmethod
    def from_char(char: PlayerCharacter):
        from .default_skills import (
            ALL_ROLAND_SKILLS,
            ALL_MORDECAI_SKILLS,
            ALL_LILITH_SKILLS,
            ALL_BRICK_SKILLS,
        )

        skill_lists = (
            ALL_ROLAND_SKILLS,
            ALL_MORDECAI_SKILLS,
            ALL_LILITH_SKILLS,
            ALL_BRICK_SKILLS,
        )

        skill_list = skill_lists[char.value]

        d: dict[SkillDefinition, int] = {}
        for i, skill_ref in enumerate(skill_list):
            skill = cast("Object", ENGINE).DynamicLoadObject(
                skill_ref, find_class("SkillDefinition")
            )
            d[cast("SkillDefinition", skill)] = i

        return FlatSkillTreeLayout(d)
