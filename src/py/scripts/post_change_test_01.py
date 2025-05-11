from mods_base import get_pc

from unrealsdk import find_object
from unrealsdk.unreal import *

print("POST CHANGE TEST")

try:
    obj: UObject = get_pc()

except Exception as ex:
    print(ex)
