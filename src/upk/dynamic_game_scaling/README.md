# NOTES

> READ ALL OF THESE CAREFULLY
- This was created before I extended the PythonSDK to support BL1
- DLC2 (Underdome) has not been modified at all
- Incompatible with any UPK mod that modifies the same files as this one
- I have not fully tested this mod yet.

1. This adds new values to your save-file from my testing it works fine however keep a
   backup of your save if you are worried.

2. Updates to the gamestage should take effect immediately, that is, you do not need to
   save & quit to get the scaling to apply.

3. These scaling points are subjective my main goal is to keep the player underleveled but not
   always underleveled. Additionally, I wanted to allow the player to complete basically any DLC
   before leaving Fyrestone. So the base level for DLC1, DLC3, and DLC4 is level 10 and this will
   not change/scale until either leaving Fyrestone or you complete the final mission of the DLC.

4. Missing a scaling point; If you add this mod after you have completed certain missions
   then you will not get the custom scaling for those completed missions. Meaning you will
   get the default minimums. However, you can edit the scaling in WillowTree just set them to
   your level and that should be fine.

5. It might or will be possible to scale backwards, that is, no checks are made to ensure we
   assign to a value greater than the current. So it might be possible to end up rushing the
   missions and resulting in lower level areas.

6. Because this modifies the gamestage for each map the rescue claptraps missions can start giving
   you grenade mods much earlier than before; Default SDU range is 1 to 33 so once you are above
   that level you will start getting grenade mods.

8. Scaling/Balance values are shared between PT1 and PT2 so scaling PT2 will also scale PT1 and vice-versa.

9. Any missions you have collected or are ready to turn in will be affected when you turn in a mission that
   scales the are that the mission is tied to. Those missions you have picked up or are ready to turn in
   will scale up as well. So if you want to delay turning in a mission for better rewards you can this is
   just how the rewards for those areas work not something I added.

10. You will see 'Player is now Level X' when you turn in a mission that scales this is because
    fundamentally the values being serialised are just custom weapon proficiencies lol.

11. This has not been fully tested and almost certainly has bugs/issues that need to be resolved.

# Dependencies

1. [Startup File for Mods](https://www.nexusmods.com/borderlands/mods/73)
2. [Editable Basemaps](https://www.nexusmods.com/borderlands/mods/56)

# Installation

Replace `/CookedPC/` with the one provided in this mod or use a mod manager (I don't any so can't help you).

# Scaling Definitions

| Area                          | Min (Gamestage) | Max (Gamestage) | Scaling Region |
|-------------------------------|-----------------|-----------------|----------------|
| Arid Area                     | 4               | 70              | Arid Area      |
| Dahl Headlands                | 18              | 70              | Scrap          |
| Rust Commons                  | 18              | 70              | Scrap          |
| Thor (Salt Flats and onwards) | 29              | 70              | Thor           |
| DLC                           | 10              | 70              | DLC            |

- Note: The Underdome DLC is unaffected.

### Main Story Scaling

| Area      | Mission                             | Scaling To |
|-----------|-------------------------------------|------------|
| Arid Area | Blinding Nine-Toes                  | Level + 1  |
| Arid Area | Got Grenades?                       | Level + 1  |
| Arid Area | Nine-Toes: Time To Collect          | Level + 1  |
| Arid Area | Bone Head's Theft                   | Level + 2  |
| Arid Area | Sledge: The Mine Key (B4 Roid Rage) | Level + 2  |
| Arid Area | Sledge: Battle For The Badlands     | Level + 2  |
| Scrap     | Sledge: Battle For The Badlands     | Level + 4  |
| DLC       | Leaving Fyrestone                   | Level + 2  |
| Scrap     | Power To The People                 | Level + 3  |
| Arid Area | Power To The People                 | Level + 2  |
| Scrap     | Hair Of The Dog                     | Level + 2  |
| DLC       | The Next Piece                      | Level + 1  |
| Scrap     | Jaynistown: Secret Rendezvous       | Level + 3  |
| Arid Area | Jaynistown: Secret Rendezvous       | Level + 3  |
| Scrap     | Jaynistown: Unintended Consequences | Level + 2  |
| Thor      | Jaynistown: Unintended Consequences | Level + 4  |
| DLC       | Jaynistown: Unintended Consequences | Level + 1  |
| Thor      | Find The Echo Command Console       | Level + 3  |
| Arid Area | Bring The Vault Key To Tannis       | Level + 1  |
| Scrap     | Bring The Vault Key To Tannis       | Level + 1  |
| Thor      | Bring The Vault Key To Tannis       | Level + 1  |
| DLC       | Bring The Vault Key To Tannis       | Level + 1  |

### DLC Scaling Points

| DLC | Mission                                        | Scaling To |
|-----|------------------------------------------------|------------|
| 1   | Ned's undead, baby, Ned's undead               | Level + 2  |
| 3   | Loot Larceny                                   | Level + 2  |
| 4   | Helping is its own reward... Wait No it isn't! | Level + 2  |
