# 物品系统 Phase 4 开发记录

# 1. 系统功能说明

Phase 4 把装备复制拆成“Owner-only 完整状态”和“Pawn 级公开外观”两条通道：

- `UDOEquipmentComponent` 继续只向 Owner 复制完整装备实例、词缀、耐久、强化和 Revision。
- `UDOEquipmentPresentationComponent` 挂在 Pawn 上，向所有相关客户端复制轻量外观摘要。
- 公开摘要只包含 `SlotTag`、`AppearanceId`、`VariantTag`、`Tint` 和 `VisualRevision`。
- 装备、卸下、读档恢复、重生和重新 Possess 后会重建公开表现。
- FastArray Add/Change/Remove 在 `PostReplicatedReceive` 聚合后只广播一次。
- OperationResult 携带 `ClientOperationId`、Outcome、FailureReason 和权威 Revision。

私有字段永远不会通过公开表现泄露：`InstanceId`、词缀、耐久、强化等级、属性 GE Handle、AbilitySpecHandle 和装备属性数值均不复制给远端观察客户端。

# 2. C++ 架构说明

```text
PlayerState
  +-- UDOInventoryComponent       (COND_OwnerOnly)
  +-- UDOEquipmentComponent       (COND_OwnerOnly)
  +-- UDOItemQuickBarComponent    (Owner intent / snapshot)
  +-- UDOAbilitySystemComponent
        |
        +-- Pawn::UDOEquipmentPresentationComponent
              +-- FDOEquipmentPublicList (all relevant clients)
```

| 类型 | 职责 |
|---|---|
| `FDOEquipmentPublicEntry` | 公开外观值类型；不包含私有 ItemInstance。 |
| `FDOEquipmentPublicList` | Pawn 级 FastArray，聚合槽位变化。 |
| `UDOEquipmentPresentationComponent` | 构建、复制和广播公开外观；处理重生/Possess 生命周期。 |
| `UDOEquipmentComponent` | 权威装备状态和私有复制；变更后通知 Presentation。 |
| `FDOItemOperationResult` | 统一客户端操作回执和 Revision 对账。 |
| `UDOInventoryViewModel` | 依据 Result 清理 Pending，依据 Changed 刷新快照。 |

# 3. 蓝图编辑器配置教程

### 3.1 DataAsset 配置

在装备 `DOItemDefinition` 中添加 `DOItemFragment_EquipmentAppearance`：

- `AppearanceId`：表现注册表使用的稳定名称，不直接复制资源指针。
- `VariantTag`：例如武器形态、服饰版本或染色方案。
- `Tint`：公开染色值。

新增 `UDOEquipmentAppearanceRegistry` 作为项目级 `PrimaryDataAsset`：

- 每个 `AppearanceId + VariantTag` 只能有一条记录。
- `VisualActorClass` 用于复杂武器/服饰表现；`SkeletalMesh`、`StaticMesh` 和 `Materials` 用于简单网格表现。
- `AttachSocket` 与 `RelativeTransform` 只属于客户端表现，不进入 `FDOEquipmentPublicEntry`。
- 注册表引用使用软引用，客户端按 `PresentationChanged` 异步加载；服务器不依赖这些资源完成装备事务。

没有 Appearance Fragment 的装备仍可穿戴，远端只会收到对应槽位但为空外观摘要，表现层应使用默认模型。

### 3.2 GameplayTag 配置

- 外观变体使用 `Equipment.Appearance.*` 命名空间。
- UI 消息使用集中声明的 `Message.UI.Equipment.PresentationChanged`。
- 装备操作结果继续使用 `Message.UI.Equipment.OperationResult`。
- 不在蓝图中手写 Tag 字符串；新增 Tag 必须先加入 `DOGameplayTag.h/.cpp`。

### 3.3 GameplayAbility / GameplayEffect 约束

Phase 4 不改变 GA/GE 的授予语义。GA、GE 和属性仍由 Phase 3 的 EquipmentInstance 绑定到 PlayerState ASC；公开摘要只用于表现，不参与 GAS 计算。

### 3.4 Actor / Component 挂载

1. `ADOPlayerCharacter` 构造函数创建 `UDOEquipmentPresentationComponent`。
2. 不在 Pawn 上创建 `UDOEquipmentComponent`，权威组件仍属于 PlayerState。
3. `InitializeAbilitySystem()` 完成 ASC ActorInfo 初始化后调用 `RebuildPublicPresentation()`。
4. Presentation Component 的 `BeginPlay` 在服务器端尝试从 Owner PlayerState 重建摘要。

### 3.5 编辑器操作步骤

1. 在装备 DataAsset 中添加 Appearance Fragment 并填写 `AppearanceId`、Variant 和 Tint。
2. 在 `Content/DragonOath/Items/Appearance` 右键创建 `DOEquipmentAppearanceRegistry`，为每个外观填写唯一的 `AppearanceId`、可选 `VariantTag`、模型/材质软引用、挂点和相对变换。
3. 在 Player Character Blueprint 的 `EquipmentPresentationComponent` 属性中指定该注册表。
4. 在表现 Blueprint 中绑定 `OnPresentationChangedBP`，按 `ChangedSlotTags` 调用 `GetPublicEntry` 和 `ResolveAppearance`，再更新模型/材质；不要读取 PlayerState Owner-only 数组。
5. 在远端观察客户端运行 PIE，确认只能读取 Pawn Presentation Component。
6. 模拟重生、重新 Possess 和装备替换，确认摘要重新生成。

### 3.6 数据流

```text
服务器 EquipmentList 变化
        -> EquipmentComponent::RebuildPublicPresentation
        -> Presentation FastArray MarkArrayDirty
        -> 复制到相关 Pawn 客户端
        -> PostReplicatedReceive 聚合槽位
        -> PresentationChanged 消息
        -> 角色表现/装备 UI 局部刷新
```

# 4. 测试流程

### 单机/自动化

- 验证装备组件完整快照仍为 Owner-only。
- 验证公开摘要只包含五类字段，删除槽位使用 `PreReplicatedRemove` 仍能通知。
- 验证相同外观重复重建不会产生无意义 Revision；实际外观变化递增 `VisualRevision`。

### 网络 PIE

1. 启动 Listen Server + 1 Client，必要时增加第二个观察客户端。
2. 服务器穿戴/卸下装备。
3. Owner Client 检查完整装备 UI；观察客户端检查模型/材质表现。
4. 使用 Network Emulation 模拟 100-200 ms 延迟，确认 Result 先到或 FastArray 先到都不会让 UI 长期 Pending。
5. 重生和重新 Possess 后确认公开外观与 PlayerState 权威装备一致。

### GAS

确认公开摘要变化不会直接施加或移除 GE；属性变化只能来自服务器 ASC 的 EquipmentInstance 生命周期。

# 5. 常见错误

| 现象 | 原因 | 处理 |
|---|---|---|
| 远端看到词缀/耐久 | Widget 读取了 Owner-only EquipmentComponent | 远端表现只读 Presentation Component。 |
| 卸下后模型不消失 | 公开 FastArray 没有标脏或没有 PreRemove 缓存槽位 | 使用 `MarkArrayDirty` 和 `PreReplicatedRemove`。 |
| 重生后外观为空 | ASC/Pawn 初始化后没有调用 `RebuildPublicPresentation` | 保持 `InitializeAbilitySystem()` 的重建调用。 |
| 异步模型串到下一件装备 | 回调没有校验 `VisualRevision`/AppearanceId | 资源回调必须验证请求版本后再写入表现。 |
| 公开数据包含实例 GUID | 将完整 `FDOEquippedItemEntry` 放入 Pawn 复制数组 | 只复制 `FDOEquipmentPublicEntry`。 |

## 验证状态

Phase 4 C++ 公开摘要、外观注册查询和生命周期代码已完成，并通过静态验收：私有 EquipmentList 使用 `COND_OwnerOnly`，Pawn 公开通道只包含 `SlotTag/AppearanceId/VariantTag/Tint/VisualRevision`，Add/Change/Remove 在 `PostReplicatedReceive` 聚合，删除在 `PreReplicatedRemove` 缓存槽位，`InitializeAbilitySystem` 会触发重建。`DragonOath.Equipment.AppearanceRegistry` 已在 `-DDC-ForceMemoryCache` 下命令行 1/1 通过，覆盖精确变体、空变体回退和未知外观。仍需在编辑器中创建并配置 Registry、绑定 `OnPresentationChangedBP`，再完成 Listen Server + Owner Client + 观察客户端的实际外观回归，以及重生/重新 Possess 和延迟对账验证。完整 Editor 构建目前被既有 VRM4U Unity 重复符号问题阻塞；本阶段未修改 VRM4U 或 UnLua。

## 6. 本轮补充

- 新增 `DragonOath.ItemSystem.Network.ReplicationContract`，通过反射检查 Inventory/Equipment 的完整状态为 `COND_OwnerOnly`，Pawn Presentation 的摘要为公开复制。
- 新增 `DragonOath.ItemSystem.Network.PublicAppearanceBoundary`，锁定公开外观结构不包含 `InstanceId`、`Affixes` 和 `CurrentDurability`。
- 以上测试是网络契约自动化，不替代 Listen Server + Owner Client + 观察客户端的真实 PIE 验证；PIE 仍按蓝图待办执行。
- 最新命令行回归 `DragonOath.ItemSystem.Network` 为 2/2 通过，日志：`Saved/Logs/Codex_Phase23_Network_Rerun2.log`。此前测试伪造复制条件触发的 UE5.8 `CoreNet.h:340` 断言已通过改用 `FRepLayout::CreateFromClass` 查询最终复制条件修复。
