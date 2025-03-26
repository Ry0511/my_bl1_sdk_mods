import argparse
from pathlib import Path

from unrealsdk.unreal import *
from unrealsdk import logging, load_package
from mods_base import build_mod, Library, hook, command, MODS_DIR

_AUTO_LOAD_LIST: Path = MODS_DIR / "_upk_auto_load_list.txt"
_loaded = False

if not _AUTO_LOAD_LIST.exists():
    _AUTO_LOAD_LIST.touch()
    _AUTO_LOAD_LIST.write_text("# Put each upk file on a separate line")


def _internal_load_pkg(file: str, flags=0, obj_flags=0) -> None:
    pkg: UObject | None = load_package(file, flags)
    if pkg is None:
        logging.warning(f"Failed to load package '{file}'")
    else:
        logging.info(f"Loaded Package: '{file}', '{pkg._path_name()}'")
        pkg.ObjectFlags |= obj_flags


@hook(hook_func="WillowGame.WillowGFxMenu:Start")
def hook_auto_load_upk_list(
    __obj: UObject,
    __args: WrappedStruct,
    __ret,
    __func: BoundFunction,
):
    if _loaded:
        return

    if not _AUTO_LOAD_LIST.is_file():
        logging.warning(f"Auto load list file not found as '{_AUTO_LOAD_LIST}'")
        return

    logging.info("[UPK_LOADER] ~ Loading packages from upk list")
    for line in _AUTO_LOAD_LIST.read_text().splitlines():
        line = line.strip()

        if line.startswith("#") or len(line) == 0:
            continue

        p = Path(line)

        if p.is_dir():
            for f in p.iterdir():
                if f.is_file():
                    _internal_load_pkg(str(f.name))
        else:
            _internal_load_pkg(str(p.name))


@command("load_package")
def cmd_load_package(args: argparse.Namespace) -> None:
    _internal_load_pkg(args.pkg, args.load_flags, args.obj_flags)


def _int_parse(p: str) -> int:
    if p.startswith("0x") or p.startswith("0X"):
        return int(p, 16)
    else:
        return int(p)


cmd_load_package.parser.add_argument("pkg", type=str)
cmd_load_package.parser.add_argument("--load_flags", type=_int_parse, default=0)
cmd_load_package.parser.add_argument("--obj_flags", type=_int_parse, default=0)


def _on_enable():
    print("My mod has been enabled")


build_mod(
    cls=Library,
    keybinds=[],
    hooks=[hook_auto_load_upk_list],
    commands=[cmd_load_package],
    options=[],
    on_enable=_on_enable
)
