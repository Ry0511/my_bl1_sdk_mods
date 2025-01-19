
from package_tools import get_launch_args

def launched_as_editor() -> bool:
    args = get_launch_args()
    return args is not None and "-editor" in args.lower().split()
