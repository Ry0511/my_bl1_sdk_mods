#
# This does not need to be a PythonSDK mod lol; Can and might move it into ./Plugins
#

from unrealsdk import logging
from mods_base import build_mod
from .object_explorer import get_version, start, stop

build_mod(
    on_enable=lambda: start(),
    on_disable=lambda: stop(),
    auto_enable=False
)

logging.info(f"Object Explorer {get_version()} Loaded")
