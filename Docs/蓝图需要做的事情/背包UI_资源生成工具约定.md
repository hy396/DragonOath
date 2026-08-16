# 背包 UI 资源生成工具约定

## 工具

背包 UI 的新位图统一使用项目工具 `Asset/gpt_image2_edit_api.py`，本项目不调用内置 `image_gen`。

工具使用 `gpt-image-2`。API Base URL 从 `C:/Users/幻雨/.codex/config.toml` 读取，API Key 从 `C:/Users/幻雨/.codex/auth.json` 的 `OPENAI_API_KEY` 读取。

API Key 不得写入项目源码、脚本参数、日志、资产或文档。项目只保存生成后的 PNG 和导入后的 UE 资产。

## 当前资源

- 源图：`ArtSource/DragonOath/Inventory/Frames/T_DO_Inventory_PanelBackdrop.png`
- 源图：`ArtSource/DragonOath/Inventory/Frames/T_DO_Inventory_EquipmentSlot.png`
- 源图：`ArtSource/DragonOath/Inventory/Frames/T_DO_Inventory_EquipmentSlot_Horizontal_Cropped.png`
- UE 资产：`/Game/DragonOath/UI/Inventory/Frames/T_DO_Inventory_PanelBackdrop`
- UE 资产：`/Game/DragonOath/UI/Inventory/Frames/T_DO_Inventory_EquipmentSlot`
- UE 资产：`/Game/DragonOath/UI/Inventory/Frames/T_DO_Inventory_EquipmentSlot_Horizontal`

## 变更流程

1. 先使用 `gpt_image2_edit_api.py` 生成或编辑源 PNG。
2. 检查尺寸、透明区域、文字和 UI 九切需求。
3. 使用 `Tools/CreateInventoryUIAssets.py` 导入或复用 UE 纹理资产。
4. 在 `FDOInventoryStyle` 中通过软路径引用资源，动态物品图标继续异步加载。
