from mods_base.options import SpinnerOption, SliderOption

__all__ = [
    "_auto_save_type",
    "_auto_save_frequency",
]

_auto_save_type = SpinnerOption(
    choices=["To Savefile", "Into Autosave Directory"],
    identifier="Save",
    description="where should we save the game to? - to savefile is normal game behaviour, "
    + "to auto-save directory will save as AutoSave/SaveXXXX.sav",
    value="Into Autosave Directory",
)

_auto_save_frequency = SliderOption(
    min_value=20.0,
    max_value=600.0,
    step=5.0,
    identifier="Frequency (s)",
    description="how often should we autosave in seconds i.e., 30 means save the game every 30 seconds",
    value=60.0,
)
