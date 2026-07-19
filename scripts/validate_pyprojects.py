import subprocess
import sys
from pathlib import Path

PY_ROOT = Path(__file__).parent.parent / "src" / "py"
args = list(map(lambda x: str(x), PY_ROOT.rglob("pyproject.toml")))

proc = subprocess.run([sys.executable, "_validate_pyproject.py", "file", *args], check=True)
proc.check_returncode()
