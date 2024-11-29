from pathlib import Path


def package_directories(
        in_paths: list[Path],
        zip_out: Path
):
    # TODO: Implement
    return


if __name__ == "__main__":
    import argparse
    import common_utils

    parser = argparse.ArgumentParser()
    parser.add_argument("in_paths", nargs="+", type=common_utils.parse_path_exists)
    parser.add_argument("zip_out", type=Path)
    parser.add_argument("--dry-run", type=bool, default=True)
    args = common_utils.parse_args(parser, abort=True)

    if not args.dry_run:
        package_directories(*args._get_kwargs())
    else:
        common_utils.print_args(args)
