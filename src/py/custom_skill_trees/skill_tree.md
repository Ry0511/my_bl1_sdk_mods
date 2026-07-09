# Borderlands Original

The skill tree in BL1 is a static flash file containing all the main characters. Adding a new
character with custom flash content requires a new flash frame. We create a new frame called 'custom'
that has placeholder nodes that can be substituted by the skill tree.

## Skill Tree Structure

The skill tree layout is composed of three main elements:

1. Selection (sel1, .. sel24)
2. Cells (cell1, .. cell24)
3. Icon (icon1, .. icon24)

- The selection is the highlight on hover there are two types, one for kill skills and one for regular skills
  - sel1 is the artifact selection
  - sel2 is the action skill selection
  - sel3 does not exist/is skipped
  - sel4 is the first skill in branch one
- Cells are the frames around the icons
- Icons are the skill icons that are direct images

## Action Script 2 Idioms

Translation Matrices encode translations along the X,Y axes in twips which are 1/20th of a unit in
pixels. So a translation along X,Y of 439,3317 is 21.95,165.85 in pixels.