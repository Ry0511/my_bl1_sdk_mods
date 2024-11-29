import argparse
import fnmatch
import tomllib
from pathlib import Path
from types import SimpleNamespace


def print_args(args: argparse.Namespace):
    max_len = max([len(k) for k, _ in args._get_kwargs()])
    print("[ARGS]:")
    for k, v in args._get_kwargs():
        print(f"  {k:>{max_len}} -> {v}")


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


def parse_toml_file(p: Path) -> SimpleNamespace:
    def convert_to_namespace(d: dict) -> SimpleNamespace:
        ns = SimpleNamespace()


        def get_or(name: str, default: any) -> any:
            node = ns
            for n in name.split("."):
                if not hasattr(node, n):
                    return default
                node = getattr(node, n)
            return node


        ns.get_or = get_or


        def require(name: str | list[str], exit_code=-1, throw=False):
            if type(name) is not str:
                for n in name:
                    try:
                        require(n, exit_code=exit_code, throw=True)
                    except Exception:  # noqa
                        if throw:
                            raise Exception()
                        print(f"toml file is missing required fields: {name}")
                        exit(exit_code)
                return

            if get_or(name, None) is None:
                if throw:
                    raise Exception()
                print(f"toml file is missing required field: '{name}'")
                exit(exit_code)


        ns.require = require

        for k, v in d.items():
            if type(v) is dict:
                ns.__setattr__(k, convert_to_namespace(v))
            else:
                ns.__setattr__(k, v)
        return ns


    with open(p, "rb") as f:
        return convert_to_namespace(tomllib.load(f))
