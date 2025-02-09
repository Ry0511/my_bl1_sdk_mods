from typing import Tuple, Dict, Union, List
from enum import Enum

__all__ = [
    "GameRegion",
    "CompletionEffect",
]

# Pathname prefix for missions
_arid_mission_prefix = "Z0_Missions.Missions."
_dahl_mission_prefix = "I1_Missions.Missions."
_scrap_mission_prefix = "Z1_Missions.Missions."
_thor_mission_prefix = "Z2_Missions.Missions."


class GameRegion(Enum):
    Arid = "Arid"
    Dahl = "Dahl"
    Scrap = "Scrap"
    Thor = "Thor"
    DLC1 = "Jakobs Cove"
    DLC2 = "Underdome"
    DLC3 = "T-Bone Junction"
    DLC4 = "Tartarus Station"


CompletionEffect = Tuple[Union[str, GameRegion], float]

_gamestage_scaling_missions: Dict[str, List[CompletionEffect]] = {
    # Arid Missions
    "Z0_Missions.Missions.M_KillBandits": [(GameRegion.Arid, 1.0)],
    "Z0_Missions.Missions.M_BuyGrenades": [(GameRegion.Arid, 1.0)],
    "Z0_Missions.Missions.M_NineToesGetReward": [(GameRegion.Arid, 1.0)],
    "Z0_Missions.Missions.M_FixVSS": [(GameRegion.Arid, 2.0)],
    "Z0_Missions.Missions.M_SubstationKey": [(GameRegion.Arid, 2.0)],
    "Z0_Missions.Missions.M_KillSledge2": [
        (GameRegion.Arid, 2.0),
        (GameRegion.Dahl, 4.0),
        (GameRegion.Scrap, 4.0),
    ],
    "Z0_Missions.Missions.M_TravelToNH": [
        (GameRegion.DLC1, 2.0),
        (GameRegion.DLC2, 2.0),
        (GameRegion.DLC3, 2.0),
        (GameRegion.DLC4, 2.0),
    ],
    # Scrap
    "Z1_Missions.Missions.M_NewHavenGenerators": [
        (GameRegion.Arid, 2.0),
        (GameRegion.Dahl, 2.0),
        (GameRegion.Scrap, 3.0),
    ],
    "Z1_Missions.Missions.M_BridgesGetABeer": [(GameRegion.Scrap, 2.0)],
    "Z1_Missions.Missions.M_ThirdFragment": [
        (GameRegion.DLC1, 1.0),
        (GameRegion.DLC2, 1.0),
        (GameRegion.DLC3, 1.0),
        (GameRegion.DLC4, 1.0),
    ],
    "Z1_Missions.Missions.M_JaynisSecretRendezvous": [
        (GameRegion.Arid, 3.0),
        (GameRegion.Dahl, 3.0),
        (GameRegion.Scrap, 3.0),
    ],
    "Z1_Missions.Missions.M_JaynisUnintendedConseq": [
        (GameRegion.Scrap, 3.0),
        (GameRegion.Thor, 4.0),
        (GameRegion.DLC1, 1.0),
        (GameRegion.DLC2, 1.0),
        (GameRegion.DLC3, 1.0),
        (GameRegion.DLC4, 1.0),
    ],
    # Thor
    "Z2_Missions.Missions.M_FindEcho": [(GameRegion.Thor, 3.0)],
    "Z2_Missions.Missions.M_ReturnKey": [
        (GameRegion.Arid, 1.0),
        (GameRegion.Dahl, 1.0),
        (GameRegion.Scrap, 1.0),
        (GameRegion.Thor, 1.0),
        (GameRegion.DLC1, 1.0),
        (GameRegion.DLC2, 1.0),
        (GameRegion.DLC3, 1.0),
        (GameRegion.DLC4, 1.0),
    ],
}
