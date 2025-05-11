from unrealsdk import find_object
from unrealsdk.unreal import *

print("POST CHANGE TEST")

try:

    obj: UObject = find_object("Core.Package", "gd_Brick")
    with notify_changes():
        obj.ObjectFlags |= 0x4000

except Exception as ex:
    print(ex)
