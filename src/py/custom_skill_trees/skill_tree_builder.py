from __future__ import annotations
from typing import TYPE_CHECKING, cast
from itertools import chain
from collections.abc import Sequence
from pathlib import Path

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
from ui_utils import OptionBox, OptionBoxButton, TrainingBox
from unrealsdk import logging, find_class

from .default_skills import (
    SKILL_MAPPING,
    ALL_ROLAND_SKILLS,
    ALL_KILL_SKILLS,
    INDEX_MAPPING_ROW_MAJOR,
    INDEX_MAPPING_BRANCH_MAJOR,
    create_skill_look_up_table,
    generate_layout_from_paths,
)

if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.WillowGame import (
        WillowPlayerController,
        PlayerSkillSetDefinition,
        SkillSetBranchData,
        SkillDefinition,
    )

    ENGINE: Object

SKILL_LOOKUP_TABLE = create_skill_look_up_table()
SKILL_LOOKUP_TABLE_INVERSE = {v: k for k, v in SKILL_LOOKUP_TABLE.items()}
ALL_SKILL_NAMES = list(SKILL_LOOKUP_TABLE.values())
DEFAULT_SKILL_NAME = ALL_SKILL_NAMES[0]

KEEP_ALIVE = WillowObjectFlags.KEEP_ALIVE  # pyright: ignore[reportUnknownMemberType, reportAttributeAccessIssue, reportUnknownVariableType]


def _create_default_layout(branch: str) -> NestedOption:
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
        branch,
        children=(
            *_create_tier("Tier 1"),
            *_create_tier("Tier 2"),
            *_create_tier("Tier 3"),
            SpinnerOption(
                "Capstone", choices=ALL_SKILL_NAMES, value=DEFAULT_SKILL_NAME
            ),
        ),
    )


class BranchBuilder:
    screen: NestedOption

    def __init__(self, name: str):
        self.screen = _create_default_layout(name)

    def all_skills(self) -> Sequence[SpinnerOption]:
        return cast("Sequence[SpinnerOption]", self.screen.children)


@ButtonOption("Load Defaults")
def _init_from_character(_: ButtonOption) -> None:
    def _on_select(_: OptionBox, opt: OptionBoxButton) -> None:
        SKILL_TREE_BUILDER.init_from(SKILL_MAPPING[opt.name])
        SKILL_TREE_BUILDER.activate()

    box = OptionBox(
        title="Load Preset",
        message="Select a character to default initialise from",
        buttons=tuple(OptionBoxButton(k) for k in SKILL_MAPPING.keys()),
        on_select=_on_select,
    )
    box.show()


@ButtonOption("Load Preset")
def _select_preset_file(_: ButtonOption) -> None:
    from .preset_file import PRESET_DIR, PresetFile

    def _load_preset_file(_: OptionBox, btn: OptionBoxButton) -> None:
        try:
            file = PresetFile(PRESET_DIR / btn.name)
            for i, opt in enumerate(SKILL_TREE_BUILDER.all_skills()):
                opt.value = SKILL_LOOKUP_TABLE[file.skills[i]]
            SKILL_TREE_BUILDER.activate()
        except (FileNotFoundError, ValueError) as ex:
            TrainingBox(
                title="Error",
                message=f"Failed to load preset file: {ex} - check console for more details",
                min_duration=1,
            ).show()

    PresetFile.create_example_file()
    choices: tuple[Path, ...] = tuple(
        sorted(
            (k for k in PRESET_DIR.rglob("*.txt")),
            key=lambda x: x.name,
        )
    )

    OptionBox(
        title="Select Preset File",
        buttons=tuple(OptionBoxButton(name=str(k.name)) for k in choices),
        on_select=_load_preset_file,
    ).show()


class SkillTreeBuilder:
    is_enabled: BoolOption
    left: BranchBuilder
    middle: BranchBuilder
    right: BranchBuilder

    def __init__(self):

        def _on_change(opt: BoolOption, _2: bool) -> None:
            if opt.value:
                self.activate()

        self.is_enabled = BoolOption(
            "Use Custom Tree",
            description="When enabled this will apply the below settings to any loaded character. "
            + "This will/may require a skill point reset and a restart for things to properly sync.\n\n"
            + " Do note that using the same skill multiple times will break things. i.e., multiple "
            + "Metal Storms is actually just one metal storm in N places; This applies to every skill.\n\n"
            + "enabling this makes it incompatible with any other mod that modifies with the skill tree",
            value=False,
            on_change_while_enabled=_on_change,
        )
        self.left = BranchBuilder("Left Branch")
        self.middle = BranchBuilder("Middle Branch")
        self.right = BranchBuilder("Right Branch")
        self.init_from(ALL_ROLAND_SKILLS)

    def activate(self, skill_set: PlayerSkillSetDefinition | None = None) -> None:
        from . import is_enabled

        if not self.is_enabled.value or not is_enabled():
            return

        pc = cast("WillowPlayerController", get_pc())
        force_reload = False
        if (
            skill_set is None
            and pc is not None  # pyright: ignore[reportUnnecessaryComparison]
            and pc.PlayerClass is not None  # pyright: ignore[reportUnnecessaryComparison]
        ):
            skill_set = pc.PlayerClass.PlayerSkillSet
            force_reload = True

        if skill_set is None:
            return
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

        if force_reload:
            from . import force_reload_player

            force_reload_player()

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
                _select_preset_file,
                self.left.screen,
                self.middle.screen,
                self.right.screen,
            ),
        )


SKILL_TREE_BUILDER = SkillTreeBuilder()
