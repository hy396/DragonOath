"""创建背包第一版的测试 ItemDefinition 资产。

该脚本只用于编辑器一次性资产生成，不属于运行时 Lua 或游戏逻辑。
"""

import unreal


DEFINITION_PATH = "/Game/DragonOath/Items/Definitions"


def request_tag(tag_name):
    """解析一个已经由 C++ 或配置注册的 GameplayTag。"""
    tag = unreal.GameplayTag()
    if not tag.import_text(tag_name):
        raise RuntimeError(f"GameplayTag 不存在或无法解析：{tag_name}")
    return tag


def load_icon():
    """优先使用项目已有纹理，找不到时使用引擎默认纹理作为占位图。"""
    candidates = [
        "/Game/Assets/Characters/Battlemage/T/body.body",
        "/Game/Assets/Characters/Battlemage/T/cloth1.cloth1",
        "/Game/Assets/Characters/Battlemage/T/weapon_1.weapon_1",
        "/Engine/EngineResources/DefaultTexture.DefaultTexture",
    ]
    for asset_path in candidates:
        icon = unreal.load_asset(asset_path)
        if icon:
            return icon
    raise RuntimeError("无法找到可用的 ItemDefinition 占位图标")


def make_fragment(fragment_class, outer, name):
    """创建挂在 ItemDefinition 上的 Instanced Fragment。"""
    return unreal.new_object(fragment_class, outer=outer, name=unreal.Name(name))


def set_common_definition(definition, display_name, description, item_type, rarity, max_stack, tags, sell_price):
    definition.set_editor_property("display_name", unreal.Text(display_name))
    definition.set_editor_property("description", unreal.Text(description))
    definition.set_editor_property("icon", load_icon())
    definition.set_editor_property("item_type", request_tag(item_type))
    definition.set_editor_property("rarity", request_tag(rarity))
    definition.set_editor_property("item_tags", unreal.GameplayTagContainer())
    item_tags = unreal.GameplayTagContainer()
    # GameplayTagContainer 的数组字段在 UE Python 中是只读，使用 UE 文本导入接口写入。
    if tags:
        exported_tags = ",".join(f'(TagName="{tag_name}")' for tag_name in tags)
        item_tags.import_text(f"(GameplayTags=({exported_tags}))")
    definition.set_editor_property("item_tags", item_tags)
    definition.set_editor_property("max_stack_size", max_stack)
    definition.set_editor_property("sort_priority", 0)
    definition.set_editor_property("sell_price", sell_price)

    inventory_fragment = make_fragment(unreal.DOItemFragment_Inventory, definition, "InventoryFragment")
    definition.set_editor_property("fragments", [inventory_fragment])


def create_definition(name, display_name, description, item_type, rarity, max_stack, tags, sell_price, setup=None):
    """创建或更新一个测试 ItemDefinition，并保存到内容目录。"""
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = unreal.load_asset(f"{DEFINITION_PATH}/{name}.{name}")
    if not asset:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", unreal.DOItemDefinition)
        asset = asset_tools.create_asset(name, DEFINITION_PATH, unreal.DOItemDefinition, factory)
    if not asset:
        raise RuntimeError(f"创建资产失败：{name}")

    set_common_definition(asset, display_name, description, item_type, rarity, max_stack, tags, sell_price)
    if setup:
        setup(asset)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


def add_equipment_fragment(asset, slot_tag, required_level, attributes, durability):
    """给测试装备添加装备 Fragment 和基础属性。"""
    fragments = list(asset.get_editor_property("fragments"))
    equipment_fragment = make_fragment(unreal.DOItemFragment_Equipment, asset, "EquipmentFragment")
    equipment_fragment.set_editor_property("equipment_slot_tag", request_tag(slot_tag))
    equipment_fragment.set_editor_property("required_level", required_level)
    equipment_fragment.set_editor_property("max_durability", durability)
    magnitudes = {}
    for tag_name, value in attributes.items():
        magnitudes[request_tag(tag_name)] = unreal.ScalableFloat(value)
    equipment_fragment.set_editor_property("base_attribute_magnitudes", magnitudes)
    fragments.append(equipment_fragment)
    asset.set_editor_property("fragments", fragments)


def set_inventory_rules(asset, can_discard=True, can_sell=True, unique=False, bind_on_pickup=False):
    """修改通用背包 Fragment 的服务器规则。"""
    fragments = list(asset.get_editor_property("fragments"))
    for fragment in fragments:
        if fragment.get_class().get_name() == "DOItemFragment_Inventory":
            fragment.set_editor_property("b_can_discard", can_discard)
            fragment.set_editor_property("b_can_sell", can_sell)
            fragment.set_editor_property("b_unique", unique)
            fragment.set_editor_property("b_bind_on_pickup", bind_on_pickup)
    asset.set_editor_property("fragments", fragments)


def add_consumable_fragment(asset, effect_class):
    """给测试药水绑定 DragonOath 自己的即时 GameplayEffect。"""
    fragments = list(asset.get_editor_property("fragments"))
    consumable_fragment = make_fragment(unreal.DOItemFragment_Consumable, asset, "ConsumableFragment")
    consumable_fragment.set_editor_property("use_gameplay_effect", effect_class)
    fragments.append(consumable_fragment)
    asset.set_editor_property("fragments", fragments)


def main():
    unreal.EditorAssetLibrary.make_directory(DEFINITION_PATH)

    create_definition(
        "DA_Item_HealthPotion_Small",
        "小型生命药水",
        "恢复少量生命值的测试消耗品。",
        "Item.Type.Consumable",
        "Item.Rarity.Common",
        20,
        ["Item.Category.Potion"],
        5,
        lambda asset: add_consumable_fragment(asset, unreal.DOItemHealthPotionEffect),
    )
    create_definition(
        "DA_Item_ManaPotion_Small",
        "小型法力药水",
        "恢复少量法力值的测试消耗品。",
        "Item.Type.Consumable",
        "Item.Rarity.Common",
        20,
        ["Item.Category.Potion"],
        5,
        lambda asset: add_consumable_fragment(asset, unreal.DOItemManaPotionEffect),
    )
    create_definition(
        "DA_Item_Material_Iron",
        "精炼铁块",
        "用于装备强化的测试材料。",
        "Item.Type.Material",
        "Item.Rarity.Common",
        99,
        ["Item.Category.EnhancementMaterial"],
        2,
    )
    create_definition(
        "DA_Item_Quest_Key",
        "古旧钥匙",
        "任务物品测试条目，不允许丢弃。",
        "Item.Type.Quest",
        "Item.Rarity.Uncommon",
        1,
        [],
        0,
        lambda asset: set_inventory_rules(asset, can_discard=False, can_sell=False, bind_on_pickup=True),
    )

    equipment_specs = [
        ("DA_Item_Equipment_Head_01", "试作头盔", "Equipment.Slot.Head", "Item.Category.Armor", 10, {"Data.Equipment.DefensePower": 8.0}, 60),
        ("DA_Item_Equipment_Chest_01", "试作胸甲", "Equipment.Slot.Chest", "Item.Category.Armor", 10, {"Data.Equipment.DefensePower": 15.0, "Data.Equipment.MaxHealth": 25.0}, 80),
        ("DA_Item_Equipment_Hands_01", "试作手套", "Equipment.Slot.Hands", "Item.Category.Armor", 10, {"Data.Equipment.AttackSpeed": 0.05}, 45),
        ("DA_Item_Equipment_Feet_01", "试作战靴", "Equipment.Slot.Feet", "Item.Category.Armor", 10, {"Data.Equipment.MoveSpeed": 20.0}, 45),
        ("DA_Item_Equipment_Accessory_01", "试作护符", "Equipment.Slot.Accessory", "Item.Category.Accessory", 12, {"Data.Equipment.CriticalRating": 6.0}, 30),
        ("DA_Item_Equipment_Weapon_01", "试作长枪", "Equipment.Slot.Weapon", "Item.Category.Weapon", 10, {"Data.Equipment.AttackPower": 20.0}, 70),
    ]
    for name, display_name, slot_tag, category_tag, level, attributes, durability in equipment_specs:
        create_definition(
            name,
            display_name,
            "装备系统测试物品；装备不改变角色外观。",
            "Item.Type.Equipment",
            "Item.Rarity.Rare",
            1,
            [category_tag],
            20,
            lambda asset, slot=slot_tag, req=level, attrs=attributes, dura=durability: add_equipment_fragment(asset, slot, req, attrs, dura),
        )

    unreal.log("背包测试 ItemDefinition 资产创建完成：10 个。")


main()
