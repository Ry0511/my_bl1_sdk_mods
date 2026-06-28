from mods_base import SETTINGS_DIR
from mods_base import build_mod

from .options import _auto_save_type, _auto_save_frequency
from .hooks import hook_auto_save


_ = build_mod(
    hooks=[hook_auto_save],
    options=[_auto_save_type, _auto_save_frequency],
    settings_file=SETTINGS_DIR / "rys_auto_save.json",
)
