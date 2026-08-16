# 背包装备系统优化：Phase 4 蓝图待办

## C++ 已完成

- Pawn 增加 `UDOEquipmentPresentationComponent`。
- 公开 FastArray 只复制槽位、AppearanceId、VariantTag、Tint、VisualRevision。
- 完整 Inventory/Equipment 状态继续 Owner-only。
- 外观在装备变更、读档恢复、重生和重新 Possess 后重建。
- 新增 `Message.UI.Equipment.PresentationChanged`，FastArray 变化聚合后再通知。

## 编辑器/蓝图需要完成

1. 给装备 DataAsset 添加 `DOItemFragment_EquipmentAppearance`。
2. 在 `Content/DragonOath/Items/Appearance` 创建 `DOEquipmentAppearanceRegistry`，为每个 `AppearanceId + VariantTag` 配置模型、材质、表现 Actor、挂点和相对变换。
3. 在 Player Character Blueprint 的 `EquipmentPresentationComponent` 指定 Registry。
4. 角色表现 Blueprint 只读取 `EquipmentPresentationComponent`，绑定 `OnPresentationChangedBP`，不要访问远端 PlayerState 的 Owner-only EquipmentList。
5. 异步加载模型/材质时保存并校验 AppearanceId、VariantTag 和 VisualRevision。
6. 确认 Player Character 继承 C++ 类后已有 PresentationComponent，不要重复添加同名组件。

## 网络 PIE 验证

- [ ] Listen Server + Owner Client + 观察客户端能看到正确公开外观。
- [ ] 观察客户端看不到 InstanceId、词缀、耐久、强化和属性数值。
- [ ] 装备、卸下、替换、重生、重新 Possess 后外观最终一致。
- [ ] 模拟延迟时 Result 先到/复制先到都能最终对账。
- [ ] 删除最后一件装备时远端模型正确卸载。
- [ ] 运行 `DragonOath.ItemSystem.Network.ReplicationContract` 与 `PublicAppearanceBoundary`，确认复制边界契约未回归。

## 不要做

- 不要把完整 `FDOEquippedItemEntry` 复制到 Pawn。
- 不要在外观 Blueprint 中施加或移除 GameplayEffect。
- 不要把模型资源硬引用写入 ItemInstance 网络数据。
