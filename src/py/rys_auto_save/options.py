from mods_base.options import *

__all__ = [
    "_auto_save_enabled",
    "_auto_save_log",
    "_auto_save_type",
    "_auto_save_frequency",
]

_auto_save_enabled = BoolOption("Is Auto-Save Enabled?", True)
_auto_save_log = BoolOption("When auto-saving should we log to the console?", False)
_auto_save_type = SpinnerOption(
    choices=["To Savefile", "Into Autosave Directory"],
    identifier="Autosave destination",
    value="Into Autosave Directory",
    display_name="Autosave destination",
)

_auto_save_frequency = SliderOption(
    min_value=20.0,
    max_value=600.0,
    identifier="Autosave Frequency",
    value=60.0,
    display_name="Autosave frequency in seconds"
)
