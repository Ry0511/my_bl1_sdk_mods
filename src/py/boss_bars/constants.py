_boss_pawn_names: set[str] = {
    ## BASE GAME ###############################################
    "Skag3_Pawn",
    "Skag5_Pawn",
    "Skag6_Pawn",
    "Pawn_BanditCaptain2",
    "Pawn_BanditCaptain1",
    "AlphaSkag1_Pawn",
    "AlphaSkag2_Pawn",
    "BunkerBoss_Pawn",
    "Pawn_SledgeBoss",
    "AlphaSkag3_Pawn",
    "MidgetKing_Pawn",
    "Pawn_BanditCaptain3",
    "Pawn_Rakk2",
    "Spiderant1_Pawn",
    "Spiderant2_Pawn",
    "Pawn_Taylor",
    "Pawn_Jaynis",
    "Scythid2_Pawn",
    "Spiderant3_Pawn",
    "Spiderant4_Pawn",
    "Pawn_RakkHive",
    "Pawn_Reaver",
    "Pawn_Krom",
    "Pawn_BaronBoss",
    "Bodyguard1_Pawn",
    "Bodyguard2_Pawn",
    "Pawn_Master_McCloud",
    "Pawn_Tentacle_Main",
    ## DLC1 ####################################################
    ## DLC2 ####################################################
    ## DLC3 ####################################################
    "Pawn_MidgetMeatPop",
    "Pawn_Named1Assassin",
    "Pawn_Named2Assassin",
    "Pawn_Named3Assassin",
    "Pawn_Named4Assassin",
    "Pawn_Named5Assassin",
    "GiantCrabWorm_Pawn",
    "Pawn_CLAjax",
    "Pawn_Knoxx",
    "Pawn_Balance_Kyros",
    "Pawn_Balance_MasterMcCloud",
    "Pawn_Balance_Typhon",
    "Pawn_Balance_Named1",
    "Pawn_Balance_DumpsterDiver",
    "Pawn_Balance_MeatPop",
    "Pawn_Balance_MidgetLance",
    "Pawn_Balance_MiniSteve",
    "Pawn_Balance_TWrestler",
    "Pawn_Balance_Shank",
    "Pawn_Balance_MotorHead",
    "Pawn_Balance_Chaz",
    ## DLC4 ####################################################
}

# limited
_boss_vehicle_names: set[str] = {"Mad_Mel"}


def register_boss(name: str, is_vehicle_boss: bool) -> None:
    global _boss_pawn_names, _boss_vehicle_names
    if is_vehicle_boss:
        _boss_vehicle_names.add(name)
    else:
        _boss_pawn_names.add(name)


def boss_pawn_names() -> set[str]:
    return _boss_pawn_names


def boss_vehicle_names() -> set[str]:
    return _boss_vehicle_names
