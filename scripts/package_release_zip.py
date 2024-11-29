import fnmatch
from pathlib import Path
import types
from zipfile import ZIP_DEFLATED, ZipFile


def package_dir_into_zip(
        dir_path: Path,
        zip_file: ZipFile,
        base_path: str = "",
        exclude_globs: list[str] = []  # noqa
):
    for root, _, files in dir_path.walk():
        for f in files:
            if not any(fnmatch.fnmatch(f, glob) for glob in exclude_globs):
                zip_file.write(root / f, arcname=Path(base_path) / f)


if __name__ == "__main__":
    import argparse
    import common_utils

    parser = argparse.ArgumentParser()
    parser.add_argument("--dir-to-zip", required=True, type=common_utils.parse_non_empty_dir)
    parser.add_argument("--out", required=True, type=Path)
    args = common_utils.parse_args(parser, abort=True)

    common_utils.print_args(args)

    xs = list(args.dir_to_zip.glob("*.toml"))

    if len(xs) > 1 or len(xs) == 0:
        print(f"Error trying to find toml file; '{xs}'")
        exit(-1)

    ns: types.SimpleNamespace = common_utils.parse_toml_file(xs[0])
    ns.require(["packager.safe_name", "project.version"])

    proj = ns.project
    packager = ns.packager

    zip_file_out = args.out / f"{packager.safe_name}-{proj.version}.zip"

    with ZipFile(zip_file_out, "w", ZIP_DEFLATED, compresslevel=9) as zip_file:
        package_dir_into_zip(
            args.dir_to_zip,
            zip_file,
            packager.get_or("safe_dir", packager.safe_name),
            packager.get_or("exclude_globs", [])
        )
