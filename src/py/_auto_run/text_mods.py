from text_mods import export_text, export_text_prop, import_text
from mods_base import get_pc

pc = get_pc()

print(export_text_prop(pc, pc.Class._find_prop("DrawScale")))
print(export_text_prop(pc, pc.Class._find_prop("DrawScale3D")))

from pathlib import Path

wpc_file_dump = Path(__file__).parent / "wpc_dump.txt"
wpc_file_dump.write_text(export_text(pc), encoding="utf-8")
print("WPC Dumped to file!")

if import_text(
    pc,
    pc.Class._find_prop("Location"),
    "(X=416.167694,Y=5000,Z=-437.315857)"
):
    print("Import text successful")
