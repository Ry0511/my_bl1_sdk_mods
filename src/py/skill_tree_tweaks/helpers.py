from enum import Enum
from typing import Callable

from unrealsdk import find_class, logging
from unrealsdk.unreal import *
from mods_base import get_pc

__all__ = [
    "get_current_class_mod",
    "get_modified_skills",
    "get_augmented_skills_dict",
    "is_skill_unlocked",
    "ESkillBranch",
    "for_each_skill",
]


def get_current_class_mod() -> UObject | None:
    wp = get_pc()

    if wp is None:
        return None

    inventory = wp.GetPawnInventoryManager()

    if inventory is None:
        return None

    return inventory.GetCurrentCommDeck()


def get_modified_skills(com_deck: UObject | None) -> list[tuple[UObject, int]]:
    """
    :param com_deck: The class mod to get the affected skills from
    :return: List of tuples of (SkillDef, Modified Value) Value is truncated to an integer
    """
    xs = list()
    if com_deck is None:
        return xs

    resolver = find_class("WillowGame.SkillAttributeContextResolver")
    for i in range(0, len(com_deck.AttributeSlots)):
        slot = com_deck.AttributeSlots[i]
        modify = slot.AttributeToModify

        if modify is None:
            continue

        for chain in reversed(modify.ContextResolverChain):
            if chain.Class != resolver:
                continue

            skill = chain.AssociatedSkill

            if skill is None:
                s = str(chain.SkillName)
                logging.dev_warning(f"Comdeck associated skill is None; SkillName='{s}'")
                continue  # Just assuming this case is never true

            xs.append((chain.AssociatedSkill, int(slot.ComputedModifierValue)))
            break

    return xs


def get_augmented_skills_dict(class_mod: UObject | None) -> dict[UObject, int]:
    if class_mod is None:
        return {}

    modified = get_modified_skills(class_mod)
    d = {}
    for s, v in modified:
        if s not in d:
            d[s] = v
        else:
            d[s] += v

    return d


def is_skill_unlocked(player_skill: WrappedStruct) -> bool:
    # function bool IsSkillUnlocked(PlayerSkill Skill):
    #   local bool Unlocked;
    #
    #   // End:0x9F
    #   if(int(Skill.SkillTreeBranch) != int(4)) {
    #       Unlocked = WPCOwner.SkillTreeBranches[int(Skill.SkillTreeBranch)].bUnlocked
    #       && WPCOwner.SkillTreeBranches[int(Skill.SkillTreeBranch)].Tiers[Skill.SkillTreeTierIndex].bUnlocked;
    #   }
    #   return Unlocked;

    if player_skill.SkillTreeBranch == 4:
        return False

    branch = player_skill.SkillTreeBranch
    tier = player_skill.SkillTreeTierIndex
    branches = get_pc().SkillTreeBranches
    return branches[int(branch)].bUnlocked and branches[int(branch)].Tiers[int(tier)].bUnlocked


class ESkillBranch(Enum):
    SKILLBRANCH_First = 0
    SKILLBRANCH_Left = 1
    SKILLBRANCH_Middle = 2
    SKILLBRANCH_Right = 3
    SKILLBRANCH_None = 4
    SKILLBRANCH_MAX = 5


def for_each_skill(fn: Callable[[UObject, WrappedStruct, UObject], None]) -> None:
    """
    Invokes the callable for each skill in the players skill tree; Providing the Player Controller,
    PlayerSkill struct, and the SkillTreeNavDefinition object.
    :param fn: The callable to invoke.
    """
    wpc = get_pc()

    player_skills = wpc.PlayerSkills
    skill_tree_skills = (
        ESkillBranch.SKILLBRANCH_Left,
        ESkillBranch.SKILLBRANCH_Middle,
        ESkillBranch.SKILLBRANCH_Right
    )

    layout_def = wpc.PlayerClass.PlayerSkillSet.SkillTreeLayout


    def get_nav_def_for_skill(player_skill) -> UObject | None:
        nonlocal layout_def
        nav_def = layout_def.GetNavDef(
            player_skill.SkillTreeBranch,
            player_skill.SkillTreeTierIndex,
            player_skill.SkillTreeEntryIndex
        )
        return nav_def


    for skill in player_skills:
        if ESkillBranch(skill.SkillTreeBranch) not in skill_tree_skills:
            continue

        skill_def = skill.Definition

        if skill_def is None:
            logging.dev_warning("Skilltree skill has None for its definition?")
            continue

        nav_def = get_nav_def_for_skill(skill)
        if nav_def is None:
            logging.dev_warning(f"Nav definition for skill '{skill_def.SkillName}' is None?")
            continue

        fn(wpc, skill, nav_def)
