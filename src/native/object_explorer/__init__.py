#
# This does not need to be a PythonSDK mod lol; Can and might move it into ./Plugins
#

from unrealsdk import logging
from mods_base import build_mod
from .object_explorer import get_version, initialise, terminate

build_mod(
    on_enable=lambda: initialise(),
    on_disable=lambda: terminate(),
    auto_enable=False
)

logging.info(f"Object Explorer {get_version()} Loaded")
