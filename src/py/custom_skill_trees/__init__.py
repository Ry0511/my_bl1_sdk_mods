from __future__ import annotations
from typing import TYPE_CHECKING, cast, Any
from mods_base import build_mod, hook, keybind
from unrealsdk.hooks import Type
from unrealsdk.unreal import UObject, WrappedStruct, BoundFunction

if TYPE_CHECKING:
    from BL1.WillowGame import WillowGFxMovie


def _on_show_skill_tree(obj: WillowGFxMovie) -> None:
    obj.SingleArgInvokeS("skills.gotoAndStop", "custom")


@hook(hook_func="WillowGame.SkillTreeGFxHelper:ArtifactSelect", hook_type=Type.POST)
def hook_artifact_selected(
    obj: UObject,
    _1: WrappedStruct,
    _2: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _3: BoundFunction,
) -> None:
    _on_show_skill_tree(cast("WillowGFxMovie", obj.Movie))


@hook(hook_func="WillowGame.SkillTreeGFxHelper:Activate", hook_type=Type.POST)
def hook_skill_tree_shown(
    obj: UObject,
    _1: WrappedStruct,
    _2: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _3: BoundFunction,
) -> None:
    _on_show_skill_tree(cast("WillowGFxMovie", obj.Movie))


@hook(hook_func="WillowGame.SkillTreeGFxHelper:Init", hook_type=Type.POST)
def hook_skill_tree_init(
    obj: UObject,
    _1: WrappedStruct,
    _2: Any,  # pyright: ignore[reportExplicitAny, reportAny]
    _3: BoundFunction,
) -> None:
    _on_show_skill_tree(cast("WillowGFxMovie", obj.Movie))


def _on_enable() -> None:
    from .patch_flash import patch_flash

    patch_flash()


@keybind(identifier="Reload Flash File", key="F12")
def _reload_flash_file():
    _on_enable()


_ = build_mod(
    hooks=(hook_artifact_selected, hook_skill_tree_shown, hook_skill_tree_init),
    keybinds=(_reload_flash_file,),
    on_enable=_on_enable,
)
