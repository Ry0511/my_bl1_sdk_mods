import argparse
import fnmatch
from pathlib import Path
from typing import Iterable


def print_args(args: argparse.Namespace):
    max_len = max([len(k) for k, _ in args._get_kwargs()])
    print("# ARGS ", "#" * 73)
    for k, v in args._get_kwargs():
        print(f"  {k:>{max_len}} -> {v}")
    print("#" * 80)


def parse_args(parser: argparse.ArgumentParser, abort: bool = False) -> argparse.Namespace | None:
    try:
        return parser.parse_args()
    except Exception as err:
        print("[FAILED TO PARSE ARGS]")
        print(err)
        print(err.__traceback__)
        if abort:
            exit(-1)
    return None


def parse_dir(raw_path: str) -> Path:
    p = Path(raw_path)
    if not p.is_dir():
        raise argparse.ArgumentTypeError(f"Not a directory: '{p}'")
    return p.absolute().resolve()


def parse_non_empty_dir(raw_path: str) -> Path:
    p = parse_dir(raw_path)
    if len(list(p.iterdir())) <= 0:
        raise argparse.ArgumentTypeError(f"Directory is empty: '{p}'")
    return p.absolute().resolve()


def parse_file(raw_path: str, glob: str | None = None) -> Path:
    p = Path(raw_path)
    if not p.is_file():
        raise argparse.ArgumentTypeError(f"Not a file: '{p}'")

    if glob is not None and not fnmatch.fnmatch(p.name, glob):
        raise argparse.ArgumentTypeError(f"Non matching file: '{p}'")

    return p.absolute().resolve()


def parse_path_exists(raw_path: str) -> Path:
    p = Path(raw_path)
    if not p.exists():
        raise argparse.ArgumentTypeError(f"Path does not exist: '{p}'")
    return p.absolute().resolve()
