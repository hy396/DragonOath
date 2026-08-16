# 物品系统 Phase 3 开发记录

# 1. 系统功能说明

Phase 3 将装备从“槽位 + 一个属性 GE”扩展为可追踪的 GAS 来源。每一件已穿戴装备拥有一个服务器侧 `UDOEquipmentInstance`，负责保存该装备创建的属性 GE、AbilitySpec、GameplayEffect 和 GrantedTags 句柄。

本阶段完成：

- `UDOAbilitySet` 支持装备授予 Ability、GameplayEffect 和 GrantedTags。
- `UDOAbilitySystemComponent::GiveDOAbilitySetForSource` 将 `SourceObject` 写入 AbilitySpec 和 GameplayEffect Context。
- `UDOEquipmentComponent` 为每件装备创建独立 `UDOEquipmentInstance`。
- 卸下装备只移除该实例持有的句柄，不调用全局 `ClearDOAbilities()`，职业技能不会被误删。
- 装备替换、存档恢复和失败回滚都经过同一套实例化/撤销路径。
- 装备属性继续通过原生 `UDOEquipmentAttributeEffect` 和 SetByCaller Builder 进入 PlayerState ASC。
- 装备实例 `Affixes` 会按 `Data.Equipment.*` 标签合并到属性 Spec，实例词缀与 Definition 基础属性在服务器端统一计算。
- `UDOEquipmentLayout` 支持按 DataAsset 配置可用装备槽，装备 Fragment 可用 `CompatibleSlotQuery` 表达兼容查询。
- `CurrentDurability` 为零时装备仍可保留在槽位和外观通道，但属性 GE 与装备 Ability 不生效；服务器修复耐久后通过 `SetEquippedDurability` 重建来源状态。

明确边界：`UDOEquipmentInstance` 是运行时对象，不复制、不存档；可存档的仍是 `FDOItemInstanceRecord` 和槽位快照。

# 2. C++ 架构说明

```text
UDOEquipmentComponent (PlayerState)
        |
        +-- FDOEquipmentList / FDOEquippedItemEntry (Owner-only FastArray)
        +-- UDOEquipmentInstance (服务器侧 UObject)
                    |
                    +-- AttributeEffectHandle
                    +-- FDOAbilitySetGrantedHandles
                                      |
                                      v
                         UDOAbilitySystemComponent (PlayerState ASC)
```

| 类型 | 职责 |
|---|---|
| `FDOItemInstanceRecord` | 可复制/可存档的动态物品值；不持有 GAS 运行时句柄。 |
| `UDOItemDefinition` | 静态 PrimaryDataAsset，提供装备 Fragment、属性和可选 `EquipmentAbilitySet`。 |
| `UDOEquipmentComponent` | 槽位校验、穿戴/卸下/替换事务、实例生命周期和 OperationResult。 |
| `UDOEquipmentInstance` | 单件装备 GAS 来源；保存实例快照、槽位、属性 GE Handle、授予句柄。 |
| `UDOAbilitySet` | 数据驱动的 GA、GE、GrantedTags 集合。 |
| `UDOAbilitySystemComponent` | 服务器权威授予/撤销来源化 GAS 内容。 |

# 3. 蓝图编辑器配置教程

### 3.1 DataAsset 配置

### 3.1 ItemDefinition

在 `DOItemDefinition` 的 `Fragments` 中添加 `DOItemFragment_Equipment`：

1. `EquipmentSlotTag` 选择 `Equipment.Slot.*`。
2. `RequiredLevel` 和 `RequiredProfessionQuery` 填写穿戴条件。
3. 在 `AttributeModifiers` 填写攻击、防御、生命、法力、暴击、命中、闪避、攻速、移速和吸血。
4. 新资产不要继续填写 `BaseAttributeMagnitudes`；旧字段只作为迁移期兼容读取。
5. 需要装备技能或被动时，将 `EquipmentAbilitySet` 指向对应 `DA_AbilitySet`。

### 3.2 Equipment AbilitySet

创建 `DA_AbilitySet_Equipment_*`，配置：

- `GrantedAbilities.AbilityId`：使用 `Ability.Id.*` Tag。
- `AbilityClass`：必须是 `UDOGameplayAbility` 的非抽象蓝图子类。
- `InitialLevel`：装备授予等级，通常为 `1`。
- `TriggerType/InputTag/EventTag`：按项目 AbilitySet 规范配置。
- `GrantedGameplayEffects`：装备被动 GE；持续 GE 应自行配置有效的 DurationPolicy。
- `GrantedTags`：装备穿戴期间的标签，例如武器类型或护甲状态。

### 3.2 GameplayTag、GameplayAbility、GameplayEffect 配置

- Ability 身份使用 `Ability.Id.*`，输入使用集中声明的 `InputTag.*`，不要在蓝图中调用 `RequestGameplayTag` 字符串。
- 装备提供的属性使用 `Data.Equipment.*` SetByCaller Tag，由 `FDOItemEffectSpecBuilder` 统一写入。
- 装备来源 Tag 放在 `UDOAbilitySet.GrantedTags`；不要用临时 GE 手动模拟 Ability 激活标签。
- 装备 GA 的取消/互斥规则继续使用 `ActivationBlockedTags`、`BlockAbilitiesWithTag` 和 `CancelAbilitiesWithTag`。
- `EquipmentAbilitySet` 中的 Ability 蓝图不能使用抽象 C++ 基类本身；必须选择具体的可实例化蓝图类。

### 3.3 Actor / Component 挂载

- `ADOPlayerState` 持有 `UDOEquipmentComponent` 和 `UDOAbilitySystemComponent`。
- Player Character 不重复创建 EquipmentComponent。
- ASC ActorInfo 初始化完成后，装备属性和装备 Ability 才会创建；读档恢复应在 PlayerState 组件可用、ASC 已初始化后执行。
- `UDOEquipmentInstance` 不挂到 Pawn，不作为复制子对象，不加入 SaveGame。

### 3.4 穿戴、卸下、替换流程

### 穿戴

```text
Client RequestEquipItem
 -> Server 校验 InstanceId / Definition / 槽位 / 等级 / 职业
 -> 若替换，先检查旧装备可放回背包
 -> 创建 EquipmentInstance
 -> 应用属性 GE、授予 EquipmentAbilitySet
 -> 从 Inventory 移除新装备
 -> 旧装备回背包，新 Entry 写入 EquipmentList
 -> 广播 Changed 和 OperationResult
```

### 卸下

服务器先尝试把装备实例放回背包，成功后移除该实例的属性 GE、AbilitySpec、GE 和 Tag，再删除装备 Entry。

### 失败回滚

任一 GAS 应用、库存移动或回背包失败时，恢复旧数组和旧 GAS 来源。回滚不清理职业 AbilitySet。

### 3.5 蓝图创建教程

1. 在 Content Browser 创建具体装备 Ability 蓝图，父类选择项目中可实例化的 `DOGameplayAbility` 子类。
2. 如需被动属性，创建 GameplayEffect 蓝图并配置 Infinite 或 HasDuration，确认所有 SetByCaller 数据在应用前有值。
3. 创建 `DA_AbilitySet_Equipment_*`，逐项填写 GA、GE、GrantedTags。
4. 打开装备 DataAsset，把 AbilitySet 放入 `EquipmentAbilitySet`。
5. 执行 Data Validation，确认 `AbilityClass`、`AbilityId` 和 Trigger 配置完整。

### 3.6 编辑器操作步骤

1. 打开一个测试装备 DataAsset，配置装备槽、属性和 AbilitySet。
2. 打开玩家 PlayerState，确认只存在一个 Inventory、Equipment 和 ASC。
3. 打开 Character Blueprint，确认存在 `EquipmentPresentationComponent`，但不要在 Character 上新增 EquipmentComponent。
4. PIE 前确认 ASC ActorInfo 已初始化；否则装备属性应用会被拒绝。
5. PIE 中穿戴、替换、卸下各一次，观察 Message Router 的 Equipment Changed/OperationResult。

# 4. 测试流程

### 单机/自动化

- `DragonOath.Equipment.Transaction`：穿戴、替换、卸下及失败回滚。
- `DragonOath.Equipment.SaveAndRestore`：存档恢复后重新创建装备 GE。
- `DragonOath.Equipment.SnapshotValidation`：重复槽位、重复 InstanceId、非法槽位和堆叠装备拒绝。
- `DragonOath.Equipment.SourceGrantLifecycle`：验证来源化 Ability/GE 句柄和精确撤销。

### 网络

PIE 使用 Listen Server + 1 Client：只在服务器执行穿戴/卸下；Owner Client 收到完整 EquipmentList，远端客户端只能看到 Phase 4 的公开表现摘要。

### GAS

1. 穿戴装备后检查 ASC 的属性变化和装备 GrantedTags。
2. 另外授予职业 Ability，卸下装备，确认职业 Ability 仍存在。
3. 同时穿戴两件提供同类 GE 的装备，分别卸下其中一件，确认另一件效果仍保留。
4. 读档、重生和重新 Possess 后确认装备来源重新建立。

# 5. 常见错误

| 现象 | 原因 | 处理 |
|---|---|---|
| 装备 GA 无法授予 | `AbilityClass` 指向抽象 C++ 类 | 改为具体蓝图子类。 |
| 卸装后职业技能消失 | 调用了全量 `ClearDOAbilities()` | 只调用 `RemoveDOAbilitySet(EquipmentInstance.GrantedHandles)`。 |
| 属性 GE 有句柄但数值为 0 | SetByCaller 没有写入 Spec | 使用 `FDOItemEffectSpecBuilder`，不要直接套用未填值的 CDO。 |
| 读档后属性重复 | 恢复前没有移除旧 EquipmentInstance | 先清理旧实例，再按快照重建。 |
| 替换失败丢物品 | 回背包空间未在修改数组前检查 | 保持服务器先校验 `CanInsertExistingItem`。 |

## 验证状态

DragonOath 源码主体已完成。新增生命周期测试已改为测试专用具体 Ability，并使用无属性修改的无限 GE 验证句柄回收。命令行自动化测试 `DragonOath.Equipment` 最新为 7/7 通过，包含 `AffixSpec`、`DurabilityRuntimeState`、`AppearanceRegistry`、`SnapshotValidation`、`SourceGrantLifecycle` 和 `Transaction`；仅有既有 GameplayCue 搜索路径警告，无测试错误。当前完整 Editor 构建仍受工程既有 UnLua/VRM4U 问题影响，按要求未修改这些外部插件。

## 6. 本轮补充

- 新增 `DragonOath.Equipment.AffixSpec`，验证基础属性和实例词缀会合并到同一个 SetByCaller 属性值。
- 新增 `DragonOath.Equipment.DurabilityRuntimeState`，验证耐久为零时不施加属性 GE，修复后可重新建立属性来源。
- 新增 `UDOEquipmentLayout` 与 `CompatibleSlotQuery`，默认仍兼容现有九个槽位；只有配置 Layout 时才启用自定义槽位集合。
- 完整 Editor 仍被 VRM4U `VRM4ULoader` Unity 重复定义阻塞，本阶段未修改 VRM4U/UnLua。
- 本轮修复耐久状态更新：同一激活状态只更新运行时快照，耐久失效/恢复时才重建 GAS 来源；重建失败会恢复原耐久和原运行时状态。最新回归日志为 `Saved/Logs/Codex_Phase23_Equipment_Rerun.log`。
