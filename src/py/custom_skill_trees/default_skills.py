from __future__ import annotations
from collections.abc import Iterable
from typing import TYPE_CHECKING, cast
from itertools import chain

from unrealsdk import find_class
from mods_base import ENGINE  # pyright: ignore[reportAssignmentType]
from unrealsdk.unreal import UObject

if TYPE_CHECKING:
    from BL1.Core import Object
    from BL1.WillowGame import SkillDefinition

    ENGINE: Object

#
# Two indexing paradigms exist for the way data is laid out - the flash side tends to prefer row
# major but unreal script and subsequently the python side prefer the branch major layout.
#
## Branch Major ###########################
# 00,01,  07,08,  14,15
# 02,03,  09,10,  16,17
# 04,05,  11,12,  18,19
#   06,     13,     20
## Row Major ########################
# 00,01,  02,03,  04,05
# 06,07,  08,09,  10,11
# 12,13,  14,15,  16,17
#   18,     19,     20
#######################################

# fmt: off
INDEX_MAPPING_ROW_MAJOR    = (0,1,6,7,12,13,18,2,3,8,9,14,15,19,4,5,10,11,16,17,20)
INDEX_MAPPING_BRANCH_MAJOR = (0,1,7,8,14,15,2,3,9,10,16,17,4,5,11,12,18,19,6,13,20)
# fmt: on

#
# Default skills in ascending position order i.e., icon4, icon5, icon6, ...
#

ALL_ROLAND_SKILLS = (
    "gd_skills2_roland.support.impact",
    "gd_skills2_roland.infantry.sentry",
    "gd_skills2_roland.support.defense",
    "gd_skills2_roland.support.stockpile",
    "gd_skills2_roland.medic.fitness",
    "gd_skills2_roland.medic.aidstation",
    "gd_skills2_roland.infantry.scattershot",
    "gd_skills2_roland.infantry.metalstorm",
    "gd_skills2_roland.support.quickcharge",
    "gd_skills2_roland.support.barrage",
    "gd_skills2_roland.medic.overload",
    "gd_skills2_roland.medic.cauterize",
    "gd_skills2_roland.infantry.refire",
    "gd_skills2_roland.infantry.assault",
    "gd_skills2_roland.support.grenadier",
    "gd_skills2_roland.support.deploy",
    "gd_skills2_roland.medic.revive",
    "gd_skills2_roland.medic.grit",
    "gd_skills2_roland.infantry.guidedmissile",
    "gd_skills2_roland.support.supplydrop",
    "gd_skills2_roland.medic.stat",
)

ALL_MORDECAI_SKILLS = (
    "gd_skills2_mordecai.sniper.focus",
    "gd_skills2_mordecai.sniper.caliber",
    "gd_skills2_mordecai.rogue.swiftstrike",
    "gd_skills2_mordecai.rogue.swipe",
    "gd_skills2_mordecai.gunslinger.deadly",
    "gd_skills2_mordecai.gunslinger.guncrazy",
    "gd_skills2_mordecai.sniper.smirk",
    "gd_skills2_mordecai.sniper.killer",
    "gd_skills2_mordecai.rogue.fasthands",
    "gd_skills2_mordecai.rogue.outforblood",
    "gd_skills2_mordecai.gunslinger.lethalstrike",
    "gd_skills2_mordecai.gunslinger.riotousremedy",
    "gd_skills2_mordecai.sniper.loaded",
    "gd_skills2_mordecai.sniper.carrioncall",
    "gd_skills2_mordecai.rogue.aerialimpact",
    "gd_skills2_mordecai.rogue.ransack",
    "gd_skills2_mordecai.gunslinger.predator",
    "gd_skills2_mordecai.gunslinger.hairtrigger",
    "gd_skills2_mordecai.sniper.trespass",
    "gd_skills2_mordecai.rogue.birdofprey",
    "gd_skills2_mordecai.gunslinger.relentless",
)

ALL_LILITH_SKILLS = (
    "gd_skills2_lilith.controller.diva",
    "gd_skills2_lilith.controller.striking",
    "gd_skills2_lilith.elemental.quicksilver",
    "gd_skills2_lilith.elemental.spark",
    "gd_skills2_lilith.assassin.slayer",
    "gd_skills2_lilith.assassin.silentresolve",
    "gd_skills2_lilith.controller.innerglow",
    "gd_skills2_lilith.controller.dramaticentrance",
    "gd_skills2_lilith.elemental.resilience",
    "gd_skills2_lilith.elemental.radiance",
    "gd_skills2_lilith.assassin.enforcer",
    "gd_skills2_lilith.assassin.hitandrun",
    "gd_skills2_lilith.controller.hardtoget",
    "gd_skills2_lilith.controller.girlpower",
    "gd_skills2_lilith.elemental.venom",
    "gd_skills2_lilith.elemental.intuition",
    "gd_skills2_lilith.assassin.highvelocity",
    "gd_skills2_lilith.assassin.blackout",
    "gd_skills2_lilith.controller.mindgames",
    "gd_skills2_lilith.elemental.phoenix",
    "gd_skills2_lilith.assassin.phasestrike",
)

ALL_BRICK_SKILLS = (
    "gd_skills2_brick.brawler.ironfist",
    "gd_skills2_brick.brawler.endlessrage",
    "gd_skills2_brick.tank.hardened",
    "gd_skills2_brick.tank.safeguard",
    "gd_skills2_brick.tank.endowed",
    "gd_skills2_brick.blaster.rapidreload",
    "gd_skills2_brick.brawler.stinglikeabee",
    "gd_skills2_brick.brawler.heavyhanded",
    "gd_skills2_brick.tank.bash",
    "gd_skills2_brick.tank.juggernaut",
    "gd_skills2_brick.blaster.revenge",
    "gd_skills2_brick.blaster.wideload",
    "gd_skills2_brick.brawler.prizefighter",
    "gd_skills2_brick.brawler.shortfuse",
    "gd_skills2_brick.tank.payback",
    "gd_skills2_brick.tank.diehard",
    "gd_skills2_brick.blaster.liquidate",
    "gd_skills2_brick.blaster.castiron",
    "gd_skills2_brick.brawler.bloodsport",
    "gd_skills2_brick.tank.unbreakable",
    "gd_skills2_brick.blaster.masterblaster",
)

ACTION_SKILLS = (
    "gd_skills2_roland.action.a_deployscorpio",
    "gd_skills2_mordecai.action.a_launchbloodwing",
    "gd_skills2_lilith.action.a_phasewalk",
    "gd_skills2_brick.action.a_berserk",
)

ALL_SKILLS = (
    ALL_ROLAND_SKILLS,
    ALL_MORDECAI_SKILLS,
    ALL_LILITH_SKILLS,
    ALL_BRICK_SKILLS,
)

SKILL_MAPPING = {
    "Roland": ALL_ROLAND_SKILLS,
    "Mordecai": ALL_MORDECAI_SKILLS,
    "Lilith": ALL_LILITH_SKILLS,
    "Brick": ALL_BRICK_SKILLS,
}

ALL_KILL_SKILLS: set[str] = {
    "gd_skills2_roland.infantry.metalstorm",
    "gd_skills2_roland.support.quickcharge",
    "gd_skills2_roland.support.grenadier",
    "gd_skills2_roland.medic.stat",
    "gd_skills2_mordecai.sniper.killer",
    "gd_skills2_mordecai.rogue.ransack",
    "gd_skills2_mordecai.gunslinger.relentless",
    "gd_skills2_mordecai.gunslinger.riotousremedy",
    "gd_skills2_lilith.controller.girlpower",
    "gd_skills2_lilith.elemental.intuition",
    "gd_skills2_lilith.elemental.phoenix",
    "gd_skills2_lilith.assassin.enforcer",
    "gd_skills2_brick.brawler.heavyhanded",
    "gd_skills2_brick.tank.juggernaut",
    "gd_skills2_brick.blaster.revenge",
    "gd_skills2_brick.blaster.masterblaster",
}


def create_skill_look_up_table() -> dict[str, str]:
    lut: dict[str, str] = dict()
    cls = find_class("SkillDefinition")
    for skill_ref in chain.from_iterable(ALL_SKILLS):
        skill = cast("SkillDefinition", ENGINE.DynamicLoadObject(skill_ref, cls))
        lut[skill_ref] = skill.SkillName
    return lut


def generate_layout_from_skills(
    action_skill: SkillDefinition | None,
    skills: Iterable[SkillDefinition],
) -> str:
    from .flat_skill_tree import FlatSkillTreeLayout, PlayerCharacter

    def _outermost(obj: UObject) -> UObject:
        outer: UObject = obj
        while outer.Outer is not None:  # pyright: ignore[reportUnnecessaryComparison]
            outer = outer.Outer
        return outer  # pyright: ignore[reportUnreachable]

    # fmt: off
    mapping = {
        "gd_skills2_roland": ("R", FlatSkillTreeLayout.from_char(PlayerCharacter.Roland)),
        "gd_skills2_mordecai": ("M", FlatSkillTreeLayout.from_char(PlayerCharacter.Mordecai)),
        "gd_skills2_lilith": ("L", FlatSkillTreeLayout.from_char(PlayerCharacter.Lilith)),
        "gd_skills2_brick": ("B", FlatSkillTreeLayout.from_char(PlayerCharacter.Brick)),
    }
    # fmt: on

    layout = ""

    if action_skill is not None:
        ref_char, _ = mapping[(str(_outermost(action_skill).Name.lower()))]
        layout += ref_char

    for skill in skills:
        assert skill is not None
        ref_char, skill_tree = mapping[_outermost(skill).Name.lower()]
        layout += f"{ref_char}{chr(ord('a') + skill_tree.skills[skill])}"

    return layout


def generate_layout_from_paths(
    action_skill: str | None,
    paths: Iterable[str],
) -> str:
    cls = find_class("SkillDefinition")
    return generate_layout_from_skills(
        (
            cast("SkillDefinition", ENGINE.DynamicLoadObject(action_skill, cls))
            if action_skill is not None
            else None
        ),
        (cast("SkillDefinition", ENGINE.DynamicLoadObject(p, cls)) for p in paths),
    )
