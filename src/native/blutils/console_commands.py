from argparse import Namespace

from mods_base import command
from unrealsdk import logging
from unrealsdk.unreal import *
import blutils

__all__: [str] = [
    "cmd_set",
    "cmd_obj_set",
    "cmd_test_obj_set",
]


@command("sset")
def cmd_set(args: Namespace) -> None:
    try:
        obj: UObject = blutils.find_object(args.obj, True)
        prop: UProperty = obj.Class._find_prop(args.prop)

        if not blutils.import_text(obj, prop, args.value, args.import_flags, args.obj_flags):
            raise ValueError(f"Failed to set property with set string: '{args.value}'")
        else:
            logging.info(f"Successfully set property on object '{obj._path_name()}'")

    except ValueError as ex:
        ex.add_note(repr(args))
        logging.error(ex)


cmd_set.parser.add_argument("obj", type=str)
cmd_set.parser.add_argument("prop", type=str)
cmd_set.parser.add_argument("value", type=str)
cmd_set.parser.add_argument("--import_flags", type=int, default=0x0)
cmd_set.parser.add_argument("--obj_flags", type=int, default=0x4000)
