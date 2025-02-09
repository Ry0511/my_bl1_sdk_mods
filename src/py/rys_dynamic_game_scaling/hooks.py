import mods_base
from mods_base import hook
from unrealsdk.hooks import Type
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction, WrappedArray

from unrealsdk import logging, find_object, load_package

__all__ = [
    "hook_commit_map_change",
    "hook_level_loaded",
    "hook_on_mission_complete",
]


@hook(hook_func="WillowGame.WillowGFxMenuHelperSaveGame:BeginGetSaveList")
def hook_commit_map_change(__obj: UObject, __args: WrappedStruct, __ret, __func: BoundFunction) -> None:
    logging.info("@WillowGFxMenuHelperSaveGame:BeginGetSaveList")

    load_package("com_ry05.upk")

    test_mission = find_object("WillowGame.MissionDefinition", "Z0_Missions.Missions.M_BuyGrenades")
    if test_mission is None or len(test_mission.OnCompleteBehaviors) != 2:
        logging.dev_warning("Z0_Missions and likely all missions are not valid/setup correctly; Aborting!")
        return

    custom_scaling_skills = [
        find_object("SkillDefinition", "com_ry05.Scaling_Arid.SkillDef"),
        find_object("SkillDefinition", "com_ry05.Scaling_DLC.SkillDef"),
        find_object("SkillDefinition", "com_ry05.Scaling_Scrap.SkillDef"),
        find_object("SkillDefinition", "com_ry05.Scaling_Thor.SkillDef")
    ]

    the_globals = find_object("GlobalsDefinition", "gd_globals.General.Globals")

    if any(map(lambda t: t is None, (the_globals, custom_scaling_skills))):
        logging.info("One or more of the expected objects is None; Aborting!")
        return

    # Little hack but we need to save a value to the players savefile
    weap_skills: UObject = the_globals.WeaponProficiencySkills
    modified = False

    for d in custom_scaling_skills:
        if d not in weap_skills:
            weap_skills.append(d)
            modified = True

    if not modified:
        return

    region_balance: WrappedArray[WrappedStruct] = the_globals.RegionBalanceData

    balance_defs: list[list[UObject]] = [
        [
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.X0_Arid_Scaling_PT1"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.X1_Dahl_Scaling_PT1"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.X2_Scrap_Scaling_PT1"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.X3_Thor_Scaling_PT1"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.XX0_DLC1_Scaling_PT1"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.XX2_DLC3_Scaling_PT1"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.XX3_DLC4_Scaling_PT1"),
        ],
        [
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.Y0_Arid_Scaling_PT2"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.Y1_Dahl_Scaling_PT2"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.Y2_Scrap_Scaling_PT2"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.Y3_Thor_Scaling_PT2"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.YY0_DLC1_Scaling_PT2"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.YY2_DLC3_Scaling_PT2"),
            find_object("WillowGame.GameBalanceDefinition", "com_ry05.Custom_Balance.YY3_DLC4_Scaling_PT2"),
        ]
    ]

    # Append Custom Scaling into each region
    for i, playthrough_balance_defs in enumerate(balance_defs):
        xs: WrappedArray = region_balance[i].BalanceDefinitions
        xs.clear()
        for d in playthrough_balance_defs:
            xs.append(d)

    the_globals.ObjectFlags |= 0x4000
    logging.info("@DynamicGameScaling; Package Contents Injected!")


@hook(hook_func="WillowGame.WillowPlayerController:SpawningProcessComplete", hook_type=Type.POST_UNCONDITIONAL)
def hook_level_loaded(obj: UObject, __args: WrappedStruct, __ret, __func: BoundFunction) -> None:
    logging.info("@WillowPlayerController:SpawningProcessComplete")
    skills = obj.PlayerSkills

    custom_scaling_skills = [
        find_object("SkillDefinition", "com_ry05.Scaling_Arid.SkillDef"),
        find_object("SkillDefinition", "com_ry05.Scaling_DLC.SkillDef"),
        find_object("SkillDefinition", "com_ry05.Scaling_Scrap.SkillDef"),
        find_object("SkillDefinition", "com_ry05.Scaling_Thor.SkillDef")
    ]

    for skill in skills:
        d = skill.Definition
        if d not in custom_scaling_skills:
            continue
        logging.info(f"@ScalingByRegion=( '{skill.Grade}', '{d._path_name()}' )")


@hook(hook_func="WillowGame.MissionTracker:GlobalCompleteMission", hook_type=Type.POST_UNCONDITIONAL)
def hook_on_mission_complete(__obj: UObject, args: WrappedStruct, ret, __func: BoundFunction):
    if not ret:
        return

    from . import utils, data

    m: UObject = args.InMission
    pname = str(m._path_name())
    scaling = data.scaling_data()

    logging.info(f"Mission Completed: '{m.MissionName}'; '{pname}'")

    # 100% a better way lol; TODO: Clean this shit up
    if pname.startswith(utils._arid_mission_prefix):
        scaling.region_scaling[utils.GameRegion.Arid.value].awesome_level += 0.33

    elif pname.startswith(utils._dahl_mission_prefix):
        scaling.region_scaling[utils.GameRegion.Dahl.value].awesome_level += 0.33

    elif pname.startswith(utils._scrap_mission_prefix):
        scaling.region_scaling[utils.GameRegion.Dahl.value].awesome_level += 0.33
        scaling.region_scaling[utils.GameRegion.Scrap.value].awesome_level += 0.33

    elif pname.startswith(utils._thor_mission_prefix):
        scaling.region_scaling[utils.GameRegion.Arid.value].awesome_level += 0.33
        scaling.region_scaling[utils.GameRegion.Dahl.value].awesome_level += 0.33
        scaling.region_scaling[utils.GameRegion.Scrap.value].awesome_level += 0.33
        scaling.region_scaling[utils.GameRegion.Thor.value].awesome_level += 0.33

    if pname not in utils._gamestage_scaling_missions:
        return

    pc = mods_base.get_pc()
    cur_level = pc.PlayerReplicationInfo.ExpLevel

    for region, value in utils._gamestage_scaling_missions[pname]:
        if isinstance(region, utils.GameRegion):
            region = region.value

        # TODO: This may eventually be possible
        if region not in scaling.region_scaling:
            logging.dev_warning(f"Skipping Region Scaling for: '{region}'")
            continue

        # Assign to the region the new scaling value if it is larger
        reg: data.ScalingModifier = scaling.region_scaling[region]
        new_value = cur_level + value
        if new_value > reg.gamestage.cur_:
            reg.gamestage.assign(new_value)
