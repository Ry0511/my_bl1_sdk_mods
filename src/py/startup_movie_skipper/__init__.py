from unrealsdk import find_all
from unrealsdk.hooks import Type
from mods_base import hook


@hook("Engine.GameViewportClient:Tick", Type.POST, immediately_enable=True)
def hook_skip_to_main_menu(_1, _2, _3, _4) -> None:
    for obj in find_all("WillowGame.WillowGFxMoviePressStart"):
        if obj.ObjectFlags & (0x400 | 0x200):
            continue
        obj.extContinue()

    hook_skip_to_main_menu.disable()
