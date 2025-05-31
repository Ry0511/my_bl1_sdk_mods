from unrealsdk import find_object, find_class
from unrealsdk.unreal import UObject, WrappedArray
from unrealsdk.hooks import Type, Block, prevent_hooking_direct_calls
from mods_base import hook, get_pc

# class=ItemPool
# native static final function bool SpawnBalancedInventoryFromPool(
#   ItemPoolDefinition         Definition
#   int                        GameStage
#   int                        AwesomeLevel
#   Object                     ContextSource
#   out array<WillowInventory> SpawnedInventory
#   optional float             OuterPoolChance = 1.0000000
# );

print("=== CUSTOM LOOT STARTING ===")


@hook(hook_func="WillowGame.MissionDefinition:GetItemRewardForPlayer")
def hook_force_no_item(*_) -> (Block, None):
    return Block, None


hook_force_no_item.disable()
hook_force_no_item.enable()


@hook(hook_func="WillowGame.MissionTracker:GrantMissionRewards", hook_type=Type.POST_UNCONDITIONAL)
def hook_on_mission_item(_1, args, *_):
    with prevent_hooking_direct_calls():
        object_path = "gd_itempools.Creatures.Creature_Boss_Items"
        obj = find_object("ItemPoolDefinition", object_path)

        item_pool_cls = find_class("WillowGame.ItemPool").ClassDefaultObject
        print(f" > ItemPool: {item_pool_cls.Class.Name}: '{item_pool_cls.Name}'")

        if item_pool_cls is None:
            print("Item pool not found")
            return None

        spawned_items: WrappedArray[UObject] | None = None

        def _spawn_item() -> bool:
            nonlocal spawned_items
            if spawned_items is not None:
                spawned_items.clear()

            retval, xs = item_pool_cls.SpawnBalancedInventoryFromPool(obj, 69, 100, get_pc(), [])

            if not retval or xs is None:
                raise ValueError("Failed to spawn item")

            spawned_items = xs

            return any(map(lambda x: x.RarityLevel >= 70, spawned_items))

        count = 10
        while not _spawn_item() and count > 0:
            count -= 1

        for item in spawned_items:
            print(f"Giving '{item.GenerateHumanReadableName()}'")
            if (wpawn := args.InWPC.GetInventoryPawn()) is not None:
                item.GiveTo(wpawn, False)

        return None


hook_on_mission_item.disable()
hook_on_mission_item.enable()
