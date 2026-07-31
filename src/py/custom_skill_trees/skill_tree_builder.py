from __future__ import annotations
from typing import TYPE_CHECKING, cast
from itertools import chain
from collections.abc import Sequence
from dataclasses import dataclass, field

from mods_base import (
    get_pc,
    BoolOption,
    GroupedOption,
    NestedOption,
    SpinnerOption,
    ButtonOption,
    ENGINE,  # pyright: ignore[reportAssignmentType]
    WillowObjectFlags,
)
from ui_utils import OptionBox, OptionBoxButton
from unrealsdk import logging, find_class

from .default_skills import (
    SKILL_MAPPING,
    ALL_ROLAND_SKILLS,
    ALL_KILL_SKILLS,
    create_skill_look_up_table,
    generate_layout_from_paths,
)

if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.WillowGame import (
        WillowPlayerController,
        SkillSetBranchData,
        SkillDefinition,
    )

    ENGINE: Object

SKILL_LOOKUP_TABLE = create_skill_look_up_table()
SKILL_LOOKUP_TABLE_INVERSE = {v: k for k, v in SKILL_LOOKUP_TABLE.items()}
ALL_SKILL_NAMES = list(SKILL_LOOKUP_TABLE.values())
DEFAULT_SKILL_NAME = ALL_SKILL_NAMES[0]

KEEP_ALIVE = WillowObjectFlags.KEEP_ALIVE  # pyright: ignore[reportUnknownMemberType, reportAttributeAccessIssue, reportUnknownVariableType]

# fmt: off
#######################################
# 00,01,  07,08,  14,15
# 02,03,  09,10,  16,17
# 04,05,  11,12,  18,19
#   06,     13,     20
#######################################
# 00,01,  02,03,  04,05
# 06,07,  08,09,  10,11
# 12,13,  14,15,  16,17
#   18,     19,     20
#######################################
INDEX_MAPPING_ROW_MAJOR    = (0,1,6,7,12,13,18,2,3,8,9,14,15,19,4,5,10,11,16,17,20)
INDEX_MAPPING_BRANCH_MAJOR = (0,1,7,8,14,15,2,3,9,10,16,17,4,5,11,12,18,19,6,13,20)
# fmt: on


def _create_default_layout() -> NestedOption:
    def _create_tier(tier: str):
        return (
            SpinnerOption(
                f"{tier} Left", choices=ALL_SKILL_NAMES, value=DEFAULT_SKILL_NAME
            ),
            SpinnerOption(
                f"{tier} Right", choices=ALL_SKILL_NAMES, value=DEFAULT_SKILL_NAME
            ),
        )

    return NestedOption(
        "Branch Data",
        children=(
            *_create_tier("Tier 1 | "),
            *_create_tier("Tier 2 | "),
            *_create_tier("Tier 3 | "),
            SpinnerOption(
                "Capstone", choices=ALL_SKILL_NAMES, value=DEFAULT_SKILL_NAME
            ),
        ),
    )


@dataclass
class BranchBuilder:
    name: str
    screen: NestedOption = field(default_factory=_create_default_layout)

    def __post_init__(self):
        self.screen.display_name = self.name

    def all_skills(self) -> Sequence[SpinnerOption]:
        return cast("Sequence[SpinnerOption]", self.screen.children)


@ButtonOption("Load Defaults")
def _init_from_character(_: ButtonOption) -> None:
    def _on_select(_: OptionBox, opt: OptionBoxButton) -> None:
        SKILL_TREE_BUILDER.init_from(SKILL_MAPPING[opt.name])

    box = OptionBox(
        title="Load Preset",
        message="Select a character to default initialise from",
        buttons=tuple(OptionBoxButton(k) for k in SKILL_MAPPING.keys()),
        on_select=_on_select,
    )
    box.show()


class SkillTreeBuilder:
    is_enabled: BoolOption
    left: BranchBuilder
    middle: BranchBuilder
    right: BranchBuilder

    def __init__(self):
        self.is_enabled = BoolOption(
            "Use Custom Tree",
            value=False,
            on_change_while_enabled=lambda s, _: self.activate(s.value),
        )
        self.left = BranchBuilder("Left Branch")
        self.middle = BranchBuilder("Middle Branch")
        self.right = BranchBuilder("Right Branch")
        self.init_from(ALL_ROLAND_SKILLS)

    def activate(self, is_enabled: bool | None = None) -> None:
        if is_enabled is None and not self.is_enabled.value:
            return

        pc = cast("WillowPlayerController", get_pc())

        if pc is None or pc.PlayerClass is None:  # pyright: ignore[reportUnnecessaryComparison]
            return

        skill_set = pc.PlayerClass.PlayerSkillSet
        skill_set.CombatSkills.clear()
        skill_set.InstinctSkillAugmentations.clear()

        def _set_branch(src: BranchBuilder, dest: SkillSetBranchData) -> None:
            index = 0
            all_skills = tuple(src.all_skills())
            cls = find_class("SkillDefinition")

            for tier in dest.Tiers:
                for i in range(0, len(tier.Skills)):
                    obj_path = SKILL_LOOKUP_TABLE_INVERSE[all_skills[index].value]
                    obj = cast(
                        "SkillDefinition",
                        ENGINE.DynamicLoadObject(obj_path, cls),
                    )
                    obj.ObjectFlags |= KEEP_ALIVE
                    tier.Skills[i] = obj
                    index += 1

                    if obj_path in ALL_KILL_SKILLS:
                        skill_set.InstinctSkillAugmentations.append(obj)
                    else:
                        skill_set.CombatSkills.append(obj)

        _set_branch(self.left, skill_set.LeftBranch)
        _set_branch(self.middle, skill_set.MiddleBranch)
        _set_branch(self.right, skill_set.RightBranch)
        pc.OnPlayerClassChange()

    def all_skills(self) -> chain[SpinnerOption]:
        return chain(
            self.left.all_skills(),
            self.middle.all_skills(),
            self.right.all_skills(),
        )

    def init_from(self, skills: Sequence[str]):
        all_skills = tuple(self.all_skills())
        for i, j in enumerate(INDEX_MAPPING_ROW_MAJOR):
            all_skills[i].value = SKILL_LOOKUP_TABLE[skills[j]]
        logging.misc(f"init_from - {self.layout_str()}")

    def layout_str(self, action_skill: str | None = None) -> str:
        all_skills = tuple(self.all_skills())
        return generate_layout_from_paths(
            action_skill=action_skill,
            paths=(
                SKILL_LOOKUP_TABLE_INVERSE[all_skills[i].value]
                for i in INDEX_MAPPING_BRANCH_MAJOR
            ),
        )

    def options(self):
        yield GroupedOption(
            "Skill Tree Config",
            children=(
                self.is_enabled,
                _init_from_character,
                self.left.screen,
                self.middle.screen,
                self.right.screen,
            ),
        )


SKILL_TREE_BUILDER = SkillTreeBuilder()
