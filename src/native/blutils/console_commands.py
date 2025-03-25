from argparse import Namespace

from mods_base import command
from unrealsdk import logging
from unrealsdk.unreal import *
import blutils

__all__: [str] = [
    "cmd_set",
    "cmd_test",
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


@command("test")
def cmd_test(args: Namespace) -> None:
    try:
        from pathlib import Path
        f = Path(__file__).parent / "tests" / "import_object.txt"
        text = f.read_text(encoding="utf-8")

        xs = blutils.import_object("gd_weap_combat_shotgun.Title", text)
        for x in xs:
            logging.info(f"OBJ -> ", x)

    except ValueError as ex:
        logging.error(ex)
