"""创建背包页面所需的最小蓝图资产。

当前 Phase 5 的背包内容由 UDOInventoryScreen::RebuildWidget 创建 C++ Slate
面板，因此这里只创建页面蓝图外壳和 PlayerController 配置蓝图，不创建空的
物品格/装备格 WBP，避免形成一套未接线的重复 UMG 架构。
"""

import unreal
import json
from pathlib import Path

try:
    from editor_toolset.toolsets.object import ObjectTools
except ImportError:
    ObjectTools = None


UI_PATH = "/Game/DragonOath/BP/UI"
PLAYER_PATH = "/Game/BP/Player"
UI_RESOURCE_PATHS = (
    "/Game/DragonOath/UI/Inventory/Frames",
    "/Game/DragonOath/UI/Inventory/Materials",
    "/Game/DragonOath/UI/Inventory/Fonts",
    "/Game/DragonOath/UI/Inventory/Audio",
    "/Game/DragonOath/UI/Inventory/Preview",
)
INVENTORY_SCREEN_NAME = "WBP_InventoryScreen"
PLAYER_CONTROLLER_NAME = "BP_DOPlayerController"
PROJECT_ROOT = Path(__file__).resolve().parent.parent
FRAME_RESOURCE_PATH = "/Game/DragonOath/UI/Inventory/Frames"
FRAME_SOURCES = (
    ("T_DO_Inventory_PanelBackdrop", PROJECT_ROOT / "ArtSource/DragonOath/Inventory/Frames/T_DO_Inventory_PanelBackdrop.png"),
    ("T_DO_Inventory_EquipmentSlot_Horizontal", PROJECT_ROOT / "ArtSource/DragonOath/Inventory/Frames/T_DO_Inventory_EquipmentSlot_Horizontal_Cropped.png"),
)


def create_widget_blueprint(name, package_path, parent_class):
    """创建或加载一个 Widget Blueprint。"""
    asset_path = f"{package_path}/{name}.{name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        return existing, False

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(name, package_path, unreal.WidgetBlueprint, factory)
    if not asset:
        raise RuntimeError(f"创建 Widget Blueprint 失败：{asset_path}")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset, True


def create_controller_blueprint(name, package_path, parent_class):
    """创建或加载 PlayerController Blueprint。"""
    asset_path = f"{package_path}/{name}.{name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        return existing, False

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(name, package_path, unreal.Blueprint, factory)
    if not asset:
        raise RuntimeError(f"创建 PlayerController Blueprint 失败：{asset_path}")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset, True


def configure_ui_texture(texture, asset_path):
    """统一设置 UI 纹理参数；编辑器工具不可用时保留导入结果并给出提示。"""
    configured = False
    if ObjectTools:
        try:
            ObjectTools.set_properties(texture, json.dumps({
                "compressionSettings": "TC_EditorIcon",
                "lODGroup": "TEXTUREGROUP_UI",
                "mipGenSettings": "TMGS_NoMipmaps",
            }))
            configured = True
        except (AttributeError, TypeError, RuntimeError, ValueError):
            configured = False
    if not configured:
        unreal.log_warning(f"未能自动写入 UI 纹理参数，请在编辑器检查：{asset_path}")
    unreal.EditorAssetLibrary.save_loaded_asset(texture)


def import_ui_texture(asset_name, source_path):
    """导入一张 UI PNG；已有资产不覆盖，避免脚本覆盖美术后续手工调优。"""
    asset_path = f"{FRAME_RESOURCE_PATH}/{asset_name}.{asset_name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        configure_ui_texture(existing, asset_path)
        return existing, False
    if not source_path.is_file():
        unreal.log_warning(f"未找到 UI 源图，跳过导入：{source_path}")
        return None, False

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source_path))
    task.set_editor_property("destination_path", FRAME_RESOURCE_PATH)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    texture = unreal.load_asset(asset_path)
    if not texture:
        raise RuntimeError(f"UI 纹理导入失败：{source_path}")

    configure_ui_texture(texture, asset_path)
    return texture, True


def main():
    unreal.EditorAssetLibrary.make_directory(UI_PATH)
    unreal.EditorAssetLibrary.make_directory(PLAYER_PATH)
    for resource_path in UI_RESOURCE_PATHS:
        unreal.EditorAssetLibrary.make_directory(resource_path)

    imported_frames = []
    for frame_name, source_path in FRAME_SOURCES:
        texture, imported = import_ui_texture(frame_name, source_path)
        if texture:
            imported_frames.append(f"{frame_name}={'新导入' if imported else '已存在'}")

    inventory_screen_parent = unreal.load_class(None, "/Script/DragonOath.DOInventoryScreen")
    player_controller_parent = unreal.load_class(None, "/Script/DragonOath.DOPlayerController")
    if not inventory_screen_parent:
        raise RuntimeError("找不到 /Script/DragonOath.DOInventoryScreen，请先编译 DragonOath 模块")
    if not player_controller_parent:
        raise RuntimeError("找不到 /Script/DragonOath.DOPlayerController，请先编译 DragonOath 模块")

    inventory_screen, screen_created = create_widget_blueprint(
        INVENTORY_SCREEN_NAME,
        UI_PATH,
        inventory_screen_parent,
    )
    player_controller, controller_created = create_controller_blueprint(
        PLAYER_CONTROLLER_NAME,
        PLAYER_PATH,
        player_controller_parent,
    )

    if not inventory_screen.generated_class():
        raise RuntimeError("WBP_InventoryScreen 没有有效的 GeneratedClass")
    if not player_controller.generated_class():
        raise RuntimeError("BP_DOPlayerController 没有有效的 GeneratedClass")

    unreal.log(
        "背包 UI 蓝图资产完成：WBP_InventoryScreen=%s (%s)，BP_DOPlayerController=%s (%s)；"
        "页面内容继续由 C++ Slate 构建。请在 BP_DOPlayerController 的 Class Defaults 中"
        "手动将‘背包界面类’设为 WBP_InventoryScreen。"
        % (
            "新建" if screen_created else "已存在",
            inventory_screen.get_path_name(),
            "新建" if controller_created else "已存在",
            player_controller.get_path_name(),
        )
    )
    if imported_frames:
        unreal.log("背包 UI 纹理资源：" + "，".join(imported_frames))


main()
