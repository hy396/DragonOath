# 物品系统 Phase 2 开发记录

# 1. 系统功能说明

Phase 2 解决 InventoryComponent 的事务通知和网络增量边界问题：

- UE 5.8 FastArray 删除前缓存 `InstanceId/SlotTag`，在 `PostReplicatedReceive` 聚合一帧内的 Add/Change/Remove。
- 服务器事务只在最终提交时递增一次 Revision；客户端接收复制时只发布本地刷新消息，不修改权威 Revision。
- Inventory、Equipment、QuickBar 都返回 `ClientOperationId + Outcome + FailureReason + AuthoritativeRevision`。
- Success、Failure、NoOp、Cancelled 都能精确结束 ViewModel 的 Pending；长时间没有回执时由 5 秒超时兜底。
- 复杂消耗品由服务器创建使用上下文，Ability 提交、失败或取消时回收同一个操作。

# 2. C++ 架构说明

```text
客户端请求(OperationId)
        |
        v
Inventory/Equipment/QuickBar Component
        |
FastArray + 事务 Revision
        |
PostReplicatedReceive 聚合回调
        |
GameplayMessageRouter
        |
InventoryViewModel / QuickBarViewModel
```

### 新增类

- `FDOItemOperationResult`：统一物品域操作回执。
- `FDOInventoryMutationChangeSet`：纯值类型变更集，记录变更和删除的实例 ID。
- `FDOInventoryMutation`：无 World、无 RPC 的槽位唯一性校验和变更集规范化辅助算法。
- `UDOInventorySortConfig`：可选的 DataAsset 排序权重配置，当前排序仍兼容 Definition 的 `SortPriority`。

### 修改职责

- `FDOInventoryList/FDOEquipmentList`：只负责 FastArray 接收期收集变更，不直接触发多次 UI 刷新。
- `UDOInventoryComponent/UDOEquipmentComponent`：负责服务器提交 Revision 和统一操作结果。
- `UDOInventoryViewModel/UDOItemQuickBarViewModel`：只按 OperationId 清理 Pending，Changed 消息只刷新快照。
- `UDOItemUseContext`：保存服务器生成的复杂道具 OperationId；蓝图不提供可信操作号。

# 3. 蓝图编辑器配置教程

### GameplayTag

不需要手写 Tag。C++ 已集中注册：

- `Message.UI.Inventory.OperationResult`
- `Message.UI.Equipment.OperationResult`
- `Message.UI.ItemQuickBar.OperationResult`

蓝图只需继续使用现有组件请求节点；不要在 Widget 中自行递增 Revision 或清理全部 Pending。

### DataAsset

可选创建：

- 路径：`/Game/DragonOath/Items/Inventory/DA_InventorySortConfig`
- 类型：`DOInventorySortConfig`
- `ItemTypeWeights`：按 ItemType Tag 配置排序权重。
- `EquipmentSlotWeights`：按 Equipment Slot Tag 配置排序权重。

未创建时继续使用 `UDOItemDefinition.SortPriority` 和 C++ 默认顺序。

### UI/Widget

1. 保持现有 `UDOInventoryScreen`、Slate 面板和 QuickBar Widget。
2. 将成功、失败、NoOp 结果都视为一次终态刷新，不要只监听 `OperationFailed`。
3. 拖拽、排序、丢弃和装备请求必须保留并回传 ViewModel 生成的 OperationId。
4. 不在蓝图中直接修改库存数量、FastArray 或 GameplayEffect。

### 编辑器操作步骤

打开 UE 编辑器

↓

打开背包页面对应的 Widget/Slate 容器

↓

确认请求节点连接到 `UDOInventoryViewModel` 或组件 Request 接口

↓

确认 Changed 只触发刷新，OperationResult 触发 Pending 结束

↓

保存并在 PIE Listen Server + 1 Client 中验证

# 4. 测试流程

### 单机自动化

运行 `DragonOath.Inventory`：`MutationInvariant`、`RevisionAndNoOp`、`MoveSplitSortDiscard`、`UseQuickBarAndSave` 以及 Phase 1 回归测试。

运行 `DragonOath.Equipment`：`Transaction`、`SaveAndRestore`、`SnapshotValidation`。

### 网络测试

1. PIE 选择 Listen Server + 1 Client。
2. Owner 客户端移动、拆分、排序、丢弃最后一个物品。
3. 观察 Owner UI 只在最终 FastArray 接收后刷新一次。
4. 非法槽位、错误 InstanceId 和重复快捷栏绑定必须返回终态回执且不改变数量、GUID 集合或 Revision。
5. 远端客户端不能读取 Owner-only Inventory/Equipment 完整实例。

### GAS 测试

1. 复杂消耗品 Ability 启动后不立即扣除库存。
2. Ability 调用 `CommitItemUse` 后扣除一个实例并返回 Success。
3. Ability 被取消后返回 Cancelled，ViewModel 不残留 Pending。

# 5. 常见错误

- 仍实现 `PostReplicatedRemove`：UE 5.8 不会调用，必须使用 `PreReplicatedRemove` + `PostReplicatedReceive`。
- 在 FastArray 回调里递增 Revision：客户端不能修改权威版本。
- Changed 消息中清理整个域的 Pending：必须等待对应 OperationId 的 Result。
- NoOp 没有回执：移动到原槽、重复排序、重复快捷栏绑定也必须返回 NoOp。
- 蓝图自行传入复杂道具 OperationId：只接受服务器创建的 `UDOItemUseContext`。
- 直接在 Widget 中扣数量或施加 GE：所有结果必须回到服务器组件和 GAS 流程。

## 6. 验证结果

- DragonOath 模块源文件已编译并链接。
- `DragonOath.Inventory`：11/11 通过（包含 `SortConfig`、`MutationRevisionBatch`、真实 SaveGame Archive 往返和 QuickBar 零库存恢复）。
- `DragonOath.Equipment`：7/7 通过（包含 `AffixSpec`、`DurabilityRuntimeState`、`AppearanceRegistry`、`SnapshotValidation`、`SourceGrantLifecycle`）。
- 最新回归日志：`Saved/Logs/Codex_Phase23_Inventory_Rerun.log`、`Saved/Logs/Codex_Phase23_Equipment_Rerun.log`。
- 完整 Editor 构建仍被现有 VRM4U Unity 符号重定义阻塞，按用户要求未修改 VRM4U。

## 7. 本轮补充与验证

- `UDOInventorySortConfig` 已接入 `TrySortInventory`，支持按 ItemType、EquipmentSlot、Rarity 覆盖默认排序权重；未配置时继续使用 C++ 默认值和 Definition.SortPriority。
- Inventory 新增 `BeginMutation/EndMutation`，装备替换等跨组件事务可把多次条目变化合并为一次 Changed 消息和一次 Revision 提交。
- 新增自动化覆盖：`DragonOath.Inventory.SortConfig`、`DragonOath.Inventory.MutationRevisionBatch`。
- 本项目 DragonOath 模块源码已完成编译；完整 Editor 构建仍被外部 VRM4U 插件 `VRM4ULoader` 的既有 Unity 重复符号/声明错误阻塞，本阶段未修改 VRM4U 或 UnLua。
- 本轮重新执行 `DragonOath.Inventory` 后 11/11 通过；复杂消耗品启动失败会清理 Pending，OperationResult 会携带服务器 Revision。
