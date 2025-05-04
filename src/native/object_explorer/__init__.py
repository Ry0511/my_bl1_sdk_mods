#
# This does not need to be a PythonSDK mod lol; Can and might move it into ./Plugins
#

from mods_base import build_mod
from .object_explorer import initialise, terminate

build_mod(
    on_enable=lambda: initialise(),
    on_disable=lambda: terminate(),
    auto_enable=False
)
