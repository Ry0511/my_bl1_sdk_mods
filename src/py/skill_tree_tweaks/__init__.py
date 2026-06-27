from mods_base import SETTINGS_DIR
from mods_base import build_mod, BoolOption

from .hooks import (
    hook_skill_tree_changed1,
    hook_skill_tree_changed2,
    hook_skill_tree_init,
    hook_skill_tree_selection_after,
)

from .flash_loader import FlashOption, patch_flash_objects


OPT_LIMIT_MAX_POINTS = BoolOption(
    "Limit Max Skill Points",
    description="Limits the max skill points to the max skill value;"
    + " i.e., 3/7 shows as 3/5 if you have a +2 augment."
    + "\n\nEnabled by default as that aligns with the other games",
    value=True,
)

# noinspection PyArgumentList
_ = build_mod(
    hooks=[
        hook_skill_tree_changed1,
        hook_skill_tree_changed2,
        hook_skill_tree_init,
        hook_skill_tree_selection_after,
    ],
    options=[OPT_LIMIT_MAX_POINTS],
    on_enable=lambda: patch_flash_objects(FlashOption.Default),
    on_disable=lambda: patch_flash_objects(FlashOption.BaseGame),
    settings_file=SETTINGS_DIR / "skill_tree_ui_tweaks.json",
)
