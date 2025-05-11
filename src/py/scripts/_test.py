from unrealsdk import *
from unrealsdk.hooks import *

print(find_class("Core.ArrayProperty"))
print(find_class("Core.ClassProperty"))
print(find_class("Core.StructProperty"))
print(find_class("Core.ObjectProperty"))
print(find_class("Core.NameProperty"))
print(find_class("Core.StrProperty"))
print(find_class("Core.IntProperty"))
print(find_class("Core.FloatProperty"))
print(find_class("Core.BoolProperty"))
print(find_class("Core.ByteProperty"))
print(find_class("Core.ComponentProperty"))
print(find_class("Core.InterfaceProperty"))
print(find_class("Core.MapProperty"))
print(find_class("Core.DelegateProperty"))

# def _impl_tick(_1, _2, _3, _4) -> None:
#     print("GameViewportClient:Tick")
#
#
# add_hook("Engine.GameViewportClient:Tick", Type.PRE, "object_explorer_tick_fn", _impl_tick)
# remove_hook("Engine.GameViewportClient:Tick", Type.PRE, "object_explorer_tick_fn")
