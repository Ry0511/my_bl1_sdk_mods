import argparse

from typing import cast

from unrealsdk import find_all, find_class, logging
from unrealsdk.unreal import UClass, UFunction, WrappedArray, WrappedStruct
from mods_base import command, get_pc
from object_explorer import get_addr
from text_mods import export_text, import_text

# Dump object to object definition string
pc = get_pc()
obj_def: str = export_text(pc)
print(f"=== {pc} ===\n{obj_def}\n=== END ===\n")

@command("getfuncaddr")
def cmd_get_addr(ns: argparse.Namespace):
    cls: UClass = find_class(ns.cls)
    fn: UFunction = cast(UFunction, cls._find(f"{ns.func}"))
    print(f"Function'{str(cls.Name)}::{str(fn.Name)}', Addr='{get_addr(fn)}'")


cmd_get_addr.parser.add_argument("cls", type=str)
cmd_get_addr.parser.add_argument("func", type=str)
cmd_get_addr.disable()
cmd_get_addr.enable()
