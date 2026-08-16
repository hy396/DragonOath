# 背包装备系统优化：Phase 2 蓝图待办

## C++ 已完成

- FastArray 使用 `PreReplicatedRemove` 缓存删除前的 `InstanceId/SlotTag`，并在 `PostReplicatedReceive` 聚合 Add/Change/Remove 通知。
- Inventory、Equipment、QuickBar 增加统一 OperationResult：`OperationId + Outcome + FailureReason + Revision`。
- ViewModel 不再因任意 Changed 消息清空整个域的 Pending，改为按 OperationId 清理并增加 5 秒超时。
- 复杂消耗品上下文由服务器写入 OperationId，Ability 提交/取消都会结束操作。
- `DOInventorySortConfig` 可选；未创建时沿用 Definition `SortPriority`。

## 编辑器/蓝图需要确认

1. 背包、装备、快捷栏 Widget 的请求必须保留 ViewModel 生成的 `ClientOperationId`。
2. UI 监听 `Message.UI.Inventory.OperationResult`、`Message.UI.Equipment.OperationResult`、`Message.UI.ItemQuickBar.OperationResult`，Success/Failure/NoOp/Cancelled 均关闭 Pending。
3. 不在蓝图中监听 Changed 后批量清除 Pending；Changed 只刷新快照。
4. 若需要自定义排序，在 `/Game/DragonOath/Items/Inventory` 创建 `DA_InventorySortConfig` 并配置 Tag 权重；不创建也可运行。
5. 复杂消耗品 Ability 只能读取 `UDOItemUseContext`，最终调用 C++ `CommitItemUse`，不自行修改库存或伪造 OperationId。

6. （可选）在 `/Game/DragonOath/Items/Inventory` 创建 `DA_InventorySortConfig`，按 ItemType、EquipmentSlot、Rarity 配置排序权重；不创建时使用 C++ 默认排序。
7. 装备替换或批量操作的蓝图入口应保持单个事务边界，不在中间帧依赖 Changed 消息刷新 UI；以最终 OperationResult 和 Changed 快照为准。

## Phase 2 验证

- [x] `DragonOath.Inventory` 自动化测试 9/9 通过。
- [x] `DragonOath.Equipment` 自动化测试 4/4 通过（包含 `SourceGrantLifecycle`）。
- [ ] PIE Listen Server + 1 Client 验证删除最后一项时 Owner UI 刷新。
- [ ] PIE 验证非法请求、NoOp 和 Ability 取消不会残留 Pending。
