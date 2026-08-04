# 07. Inventory System Design

## 1. 文档目的

本文定义 DragonOath 第一版背包、装备、物品快捷栏及其 UI 的完整实现方案。

目标是复刻参考图中的核心体验：角色装备与背包位于同一页面，左侧查看角色、装备槽和属性，右侧按分类浏览物品，并提供分页、快捷栏、出售、仓库、修理和丢弃等操作入口。

复刻范围只包含布局逻辑、信息结构和交互体验。界面边框、图标、角色形象、文字和其他美术资源必须使用 DragonOath 自有资产，不直接复制参考游戏资源。

本文同时作为系统设计与落地记录。第 31 节的 C++ 原生 GE、动态 Spec 和 `ItemSystem` 目录重组已经落地；内容资产迁移与后续玩法扩展仍按第 22 节的阶段顺序推进。

## 2. 参考界面拆解

参考图可以拆成四个主要区域：

```text
┌─────────────────────────────────────────────────────────────────┐
│ 顶部：角色名、页面标题、关闭按钮                                 │
├──────────────────────────────┬──────────────────────────────────┤
│ 左侧角色与装备面板           │ 右侧背包面板                     │
│                              │                                  │
│ - 战力/防护力                │ - 分类选择器                     │
│ - 角色模型预览               │ - 物品格子                       │
│ - 环绕角色的装备槽           │ - 页码与翻页                     │
│ - 生命/魔法/攻防等属性       │ - 物品快捷栏                     │
│ - 元素与进阶属性             │ - 出售/仓库/修理/丢弃操作        │
│ - 货币                        │                                  │
├──────────────────────────────┴──────────────────────────────────┤
│ 右侧导航：角色、背包、消耗品、其他系统入口                       │
└─────────────────────────────────────────────────────────────────┘
```

参考图中分类选择器处于展开状态，分类包含头部、肩部、背部、胸部、手套、裤子、鞋子、饰品和武器。该交互在 DragonOath 中使用 GameplayTag 驱动，不使用装备部位枚举。

## 3. 第一版目标

第一版必须完成以下闭环：

```text
服务器生成物品
  -> 玩家拾取
  -> 物品进入背包并复制给所属客户端
  -> 背包 UI 显示
  -> 玩家筛选、移动、堆叠、拆分或使用物品
  -> 玩家穿戴装备
  -> 装备通过 GAS 修改角色属性
  -> 角色面板和基础角色预览刷新
  -> 背包与装备可以保存和恢复
```

第一版功能范围：

- 固定容量背包，建议初始 40 格。
- 装备、消耗品、材料、任务物品四类物品。
- 自动堆叠、移动、交换、拆分、合并、整理和丢弃。
- 九个基础装备部位。
- 四个物品快捷栏槽位。
- 装备属性通过 GameplayEffect 应用到 ASC。
- 分类过滤、品质排序、分页、提示框和装备对比。
- Listen Server + 1 Client 下完整可用。
- 单机/本地阶段使用 SaveGame 保存。

第一版暂不实现：

- 随机词缀重铸和复杂洗练。
- 装备套装效果。
- 跨角色共享账号仓库。
- 拍卖行和玩家交易。
- 公网后端数据库。
- 批量分解和复杂强化概率系统。

出售、仓库和修理按钮需要在第一版 UI 中存在并具备正确禁用状态；其完整业务流程可以放到第二阶段。

## 4. 总体架构

```text
                          Server Authority
                                 │
                 ┌───────────────┴────────────────┐
                 │                                │
        ADOPlayerState                    掉落/NPC/存档系统
                 │                                │
        ┌────────┼───────────┐                    │
        │        │           │                    │
UDOInventory  UDOEquipment  UDOItemQuickBar <─────┘
 Component      Component       Component
        │        │           │
        │        │           └── Owner-only 快捷栏复制
        │        ├── 私有装备数据 Owner-only
        │        └── 服饰外观系统（后续独立接入）
        └── FFastArray Owner-only 增量复制
                 │
                 ▼
       客户端 OnRep / FastArray 回调
                 │
                 ▼
   GameplayMessageRouter 本地刷新消息
                 │
                 ▼
 UDOInventoryScreen / HUD QuickBar
```

核心原则：

- 背包真实数据只由服务器修改。
- UI 不保存权威物品数量，不直接改数组。
- GameplayMessageRouter 只通知界面刷新，不承载背包修改请求。
- DataAsset 保存静态定义，FastArray 保存运行时实例。
- 装备属性进入 GAS，不在 Widget 中计算最终角色属性。
- 客户端可以本地筛选和分页，但移动、使用、穿戴等操作必须等待服务器确认。

### 4.1 技术选型

| 技术                                | 用途                             | 选择原因                                             |
| ----------------------------------- | -------------------------------- | ---------------------------------------------------- |
| `UPrimaryDataAsset`               | 物品静态定义                     | 支持软引用、资产扫描、Cook 管理和稳定 PrimaryAssetId |
| GameplayTag / GameplayTagQuery      | 类型、品质、装备部位、筛选条件   | 与项目现有 Tag 规范一致，避免扩展时修改枚举          |
| `FFastArraySerializer`            | 背包和装备增量复制               | 只复制变化条目，适合频繁增删和位置变化               |
| Server RPC                          | 移动、穿戴、使用、出售等请求     | 保证服务器权威，客户端不能直接改真实数据             |
| GAS GameplayEffect                  | 装备和消耗品属性效果             | 与现有 ASC、AttributeSet、Buff 和网络复制统一        |
| SetByCaller                         | 每件装备的动态属性数值           | 使用同一个通用 GE 承载不同装备数值                   |
| CommonUI / Setly                    | 页面栈、返回、输入模式和手柄焦点 | 项目已启用，适合背包这类主动打开的菜单页面           |
| 原生 Slate                          | 背包主体渲染、格子虚拟化、拖拽和高密度布局 | 背包是固定结构、高交互密度页面，适合 C++ 统一控制 |
| GameplayMessageRouter               | 复制完成后的本地 UI 刷新通知     | 解耦组件和 Widget，不承担权威请求                    |
| `UTileView` / `UCommonListView` | 虚拟化物品列表                   | 避免为所有格子永久创建 Widget                        |
| SceneCapture2D + RenderTarget       | 基础角色预览（可选）             | 只显示当前 Pawn；不把装备属性和外观绑定             |
| AssetManager + Soft Reference       | 图标和定义加载                   | 避免同步加载和打包漏资源                             |
| SaveGame + 版本字段                 | 第一阶段持久化                   | 后续可平滑迁移到服务器账号数据                       |

## 5. 对象归属与生命周期

### 5.1 PlayerState 持有运行时组件

建议在 `ADOPlayerState` 上创建：

```text
UDOInventoryComponent
UDOEquipmentComponent
UDOItemQuickBarComponent
```

原因：

- PlayerState 生命周期长于 Pawn，死亡、重生和重新 Possess 不会丢失背包。
- 玩家 ASC 同样在 PlayerState 上，装备 GE 可以直接施加到同一个 ASC。
- Owner-only 复制天然适合玩家私有背包。
- 后续接入存档或服务器账号数据时边界清晰。

角色 Pawn 不负责装备外观。装备只影响装备槽、属性和 GAS；后续服饰/时装系统单独负责角色外观与预览。

### 5.2 组件职责

`UDOInventoryComponent`：

- 保存背包容量和背包条目。
- 添加、移除、堆叠、拆分、移动、交换和整理物品。
- 处理拾取、丢弃和消耗。
- 暴露只读查询接口给 UI。
- 负责 Owner-only FastArray 复制。

`UDOEquipmentComponent`：

- 保存装备槽和已装备物品。
- 校验职业、等级、部位和唯一性规则。
- 穿戴与卸下装备。
- 创建、保存和移除装备 GameplayEffectHandle。

`UDOItemQuickBarComponent`：

- 保存四个物品快捷栏绑定。
- 将快捷栏输入转换成“使用某类物品”的服务器请求。
- 监听背包数量变化并刷新 HUD。

## 6. 目录规划

不创建顶层 `Public/Private`，继续使用项目的功能域目录：

```text
Source/DragonOath/
  ItemSystem/
    Core/
      DOItemDefinition.h/.cpp
      DOItemAttributeTypes.h
    Inventory/
      DOInventoryTypes.h
      DOInventoryComponent.h/.cpp
      DOInventoryMessages.h
    Equipment/
      DOEquipmentTypes.h
      DOEquipmentComponent.h/.cpp
    Usage/
      DOItemUseTypes.h/.cpp
      DOItemUseEffects.h/.cpp
    QuickBar/
      DOItemQuickBarComponent.h/.cpp
      DOItemQuickBarViewModel.h/.cpp
    Pickup/
      DOItemPickup.h/.cpp
    AbilitySystem/
      DOItemGameplayEffects.h/.cpp
      DOItemEffectSpecBuilder.h/.cpp
      DOGameplayAbility_UseItem.h/.cpp
    Tests/
      DOInventoryAutomationTests.cpp
  UI/
    Inventory/
      DOInventoryScreen.h/.cpp
      DOInventoryViewModel.h/.cpp
      DOInventorySlotViewModel.h/.cpp
      DOItemTooltipViewModel.h/.cpp
  SaveGame/
    DOSaveGame.h/.cpp
```

内容目录：

```text
Content/DragonOath/
  Items/
    Definitions/
    Equipment/
    Consumables/
    Materials/
    Icons/
  Effects/
    Equipment/
    Consumables/
  UI/
    Inventory/
      Screens/
      Widgets/
      Styles/
      Materials/
  CharacterPreview/
```

## 7. GameplayTag 设计

所有 Tag 集中声明在 `DOGameplayTag.h/.cpp`，注释使用中文。

### 7.1 物品分类

```text
Item.Type.Equipment
Item.Type.Consumable
Item.Type.Material
Item.Type.Quest

Item.Category.Weapon
Item.Category.Armor
Item.Category.Accessory
Item.Category.Potion
Item.Category.EnhancementMaterial
```

### 7.2 装备部位

```text
Equipment.Slot.Head
Equipment.Slot.Shoulder
Equipment.Slot.Back
Equipment.Slot.Chest
Equipment.Slot.Hands
Equipment.Slot.Legs
Equipment.Slot.Feet
Equipment.Slot.Accessory
Equipment.Slot.Weapon
```

饰品后续如果需要左右槽，可扩展为：

```text
Equipment.Slot.Accessory.Left
Equipment.Slot.Accessory.Right
```

### 7.3 品质

```text
Item.Rarity.Common
Item.Rarity.Uncommon
Item.Rarity.Rare
Item.Rarity.Epic
Item.Rarity.Legendary
```

第一版与现有属性设计保持一致：白色、蓝色、紫色、橙色。`Uncommon` 可以预留但不必立即产出。

### 7.4 SetByCaller

```text
Data.Equipment.AttackPower
Data.Equipment.DefensePower
Data.Equipment.MaxHealth
Data.Equipment.MaxMana
Data.Equipment.CriticalRating
Data.Equipment.HitRating
Data.Equipment.EvasionRating
Data.Equipment.AttackSpeed
Data.Equipment.MoveSpeed
```

### 7.5 本地刷新消息

```text
Message.UI.Inventory.Changed
Message.UI.Equipment.Changed
Message.UI.ItemQuickBar.Changed
Message.UI.Inventory.OperationFailed
```

## 8. 静态物品定义

### 8.1 UDOItemDefinition

物品静态配置使用 `UPrimaryDataAsset`，资产命名为 `DA_Item_*`。

```cpp
UCLASS(BlueprintType)
class DRAGONOATH_API UDOItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 显示名称和描述只属于静态配置，不复制到网络。
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine=true))
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag ItemType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag Rarity;

    // 分类、用途和规则标签，供 GameplayTagQuery 过滤。
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTagContainer ItemTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxStackSize = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 SortPriority = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 SellPrice = 0;

    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
    TArray<TObjectPtr<UDOItemFragment>> Fragments;
};
```

物品通过 `FPrimaryAssetId` 进行网络和存档标识，不保存磁盘路径字符串，也不依赖蓝图类名。

建议让 `UDOItemDefinition::GetPrimaryAssetId()` 固定返回 `ItemDefinition:<AssetName>`，避免蓝图继承层级改变 PrimaryAssetType：

```cpp
FPrimaryAssetId UDOItemDefinition::GetPrimaryAssetId() const
{
    static const FPrimaryAssetType ItemDefinitionType(TEXT("ItemDefinition"));
    return FPrimaryAssetId(ItemDefinitionType, GetFName());
}
```

需要在 `DefaultGame.ini` 的 AssetManager 设置中注册 `DOItemDefinition` 扫描目录。

配置形式如下，实际类路径以代码落地后的反射路径为准：

```ini
+PrimaryAssetTypesToScan=(PrimaryAssetType="ItemDefinition",AssetBaseClass="/Script/DragonOath.DOItemDefinition",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/Game/DragonOath/Items/Definitions")),SpecificAssets=,Rules=(Priority=0,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))
```

### 8.2 Fragment 设计

使用小型 Fragment 避免一个 ItemDefinition 塞满所有类型字段：

```text
UDOItemFragment_Inventory
  - bCanDiscard
  - bCanSell
  - bUnique
  - bBindOnPickup

UDOItemFragment_Equipment
  - EquipmentSlotTag
  - RequiredLevel
  - RequiredProfessionQuery
  - FDOAttributeModifierValues AttributeModifiers
  - MaxDurability

UDOItemFragment_Consumable
  - EDOConsumableEffectKind EffectKind
  - FDOResourceRestoreValues InstantRestore
  - FDOItemTimedModifierValues TimedModifier
  - UseGameplayAbility
  - UseEventTag
  - FDOItemCooldownConfig Cooldown

UDOItemFragment_World
  - PickupMesh
  - DropNiagara
  - PickupSound
```

第一版只实现 Inventory、Equipment 和 Consumable 三个 Fragment。

装备和简单道具不再要求策划为每个物品创建 GameplayEffect 蓝图。属性通过类型化结构体直接暴露在 ItemDefinition 中，服务器读取数值后，以 C++ 原生 GE 为模板动态创建 `FGameplayEffectSpec`。详细改进方案见第 31 节。

## 9. 运行时物品实例

每个背包堆栈都拥有稳定的 `InstanceId`。即使是可堆叠材料，也将一个堆栈视为一个实例。

```cpp
USTRUCT(BlueprintType)
struct FDOItemInstanceRecord
{
    GENERATED_BODY()

    // 网络请求和存档使用的稳定实例标识。
    UPROPERTY()
    FGuid InstanceId;

    // 指向静态物品定义。
    UPROPERTY()
    FPrimaryAssetId DefinitionId;

    UPROPERTY()
    int32 StackCount = 1;

    // 背包中的稳定位置，-1 表示当前不在普通背包。
    UPROPERTY()
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY()
    int32 UpgradeLevel = 0;

    UPROPERTY()
    int32 CurrentDurability = 0;

    UPROPERTY()
    TArray<FDOItemAffixRoll> Affixes;
};
```

第一版装备可以保留 `Affixes` 字段但不生成随机词缀，避免将来修改存档结构。

选择结构体而不是复制 UObject 的原因：

- FastArray 复制简单稳定。
- SaveGame 和未来后端序列化直接。
- 不需要维护 replicated subobject 注册和销毁。
- UI 可以另建本地 ViewModel，不要求运行时物品本身是 UObject。

## 10. FastArray 背包复制

```cpp
USTRUCT()
struct FDOInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY()
    FDOItemInstanceRecord Item;
};

USTRUCT()
struct FDOInventoryList : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FDOInventoryEntry> Entries;

    UPROPERTY(NotReplicated)
    TObjectPtr<UDOInventoryComponent> OwnerComponent;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
};

template<>
struct TStructOpsTypeTraits<FDOInventoryList> : public TStructOpsTypeTraitsBase2<FDOInventoryList>
{
    enum
    {
        WithNetDeltaSerializer = true
    };
};
```

在 `UDOInventoryComponent` 中使用：

```cpp
DOREPLIFETIME_CONDITION(
    UDOInventoryComponent,
    InventoryList,
    COND_OwnerOnly);
```

修改规则：

- 新增条目后调用 `MarkItemDirty`。
- 修改数量、槽位或强化等级后调用 `MarkItemDirty`。
- 批量整理后对所有位置变化的条目调用 `MarkItemDirty`。
- 删除条目后调用 `MarkArrayDirty`。
- 只在服务器修改 FastArray。

`PostReplicatedAdd`、`PostReplicatedChange` 和 `PostReplicatedRemove` 只收集变更实例 ID，并让组件广播一次本地刷新消息，避免每个条目触发一次完整 UI 重建。

`UDOInventoryComponent` 构造时调用 `SetIsReplicatedByDefault(true)`。在 `ADOPlayerState` 构造函数中通过 `CreateDefaultSubobject` 创建组件，保证服务器和客户端拥有一致的组件生命周期。

## 11. 背包核心算法

### 11.1 添加物品

添加顺序固定：

1. 校验物品定义和数量。
2. 按槽位顺序填充相同 DefinitionId 的未满堆栈。
3. 查找空槽创建新堆栈。
4. 返回成功数量和剩余数量。

```cpp
USTRUCT(BlueprintType)
struct FDOInventoryAddResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 RequestedCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 AddedCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 RemainingCount = 0;

    UPROPERTY(BlueprintReadOnly)
    EDOInventoryFailureReason FailureReason;
};
```

拾取系统在服务器调用 `TryAddItem`。如果背包只能接收一部分，拾取物保留剩余数量；不能因为客户端显示动画就提前销毁掉落物。

### 11.2 移动和交换

客户端只提交：

```text
InstanceId
SourceSlot
TargetSlot
RequestedCount
ClientOperationId
```

服务器重新查询 InstanceId 的真实位置，不信任客户端提供的 SourceSlot。TargetSlot 为空时移动；相同物品且允许堆叠时合并；其他情况交换。

### 11.3 拆分堆栈

拆分要求：

- 数量大于 0 且小于原堆栈数量。
- 目标槽为空。
- 物品允许堆叠。
- 新堆栈生成新的 InstanceId。

### 11.4 整理

整理在服务器执行，排序规则由共享 C++ 比较器定义：

```text
物品类型优先级
  -> 装备部位优先级
  -> 品质降序
  -> 需求等级降序
  -> DefinitionId
  -> InstanceId
```

客户端分类过滤不会改变服务器槽位；点击“整理”才会真正修改 SlotIndex。

### 11.5 删除和丢弃

任务物品、锁定物品和正在交易的物品禁止丢弃。高品质装备或数量大于一的堆栈需要 Modal 确认。

服务器确认后才删除条目。需要生成世界掉落时，由服务器生成 Pickup Actor，并设置拾取归属和保护时间。

## 12. 服务器 RPC 与校验

建议使用职责明确的 RPC，不使用一个包含任意命令字符串的万能 RPC：

```text
Server_RequestMoveItem
Server_RequestSplitStack
Server_RequestSortInventory
Server_RequestUseItem
Server_RequestEquipItem
Server_RequestUnequipItem
Server_RequestDiscardItem
Server_RequestAssignQuickBar
```

所有请求至少校验：

- 调用者是否拥有该 PlayerState。
- InstanceId 是否存在。
- 数量、槽位和 Tag 是否合法。
- 物品当前是否锁定或正在其他事务中。
- 玩家等级、职业和角色状态是否满足。
- NPC 商店、仓库或修理上下文是否仍然有效。
- 玩家与交互 Actor 的距离是否合法。
- 货币和背包空间是否足够。

客户端不得提交价格、属性值、强化结果或物品定义内容。客户端只提交“想操作哪个实例”和“目标位置”。

每次操作携带递增 `ClientOperationId`。服务器通过 Client RPC 返回成功或失败原因，UI 用该 ID 清除对应槽位的 Pending 状态。

## 13. 装备系统

### 13.1 装备条目

装备列表不与普通背包共用 SlotIndex：

```cpp
USTRUCT()
struct FDOEquippedItemEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY()
    FGameplayTag SlotTag;

    UPROPERTY()
    FDOItemInstanceRecord Item;
};
```

穿戴流程：

```text
客户端请求穿戴 InstanceId
  -> 服务器读取 ItemDefinition
  -> 校验 Equipment Fragment、职业、等级和部位
  -> 预检查旧装备是否能放回背包
  -> 从背包移除新装备
  -> 旧装备放回背包
  -> 新装备写入 EquipmentList
  -> 应用装备 GE
  -> 标记 FastArray
  -> 客户端收到复制并刷新 UI
```

跨背包和装备的操作必须先完成全部预检查，再修改数组，避免中途失败产生复制物品或丢失物品。

### 13.2 装备属性进入 GAS

创建 C++ 原生通用无限时长 GE：

```text
UDOEquipmentAttributeEffect
DurationPolicy = Infinite
```

该类为所有第一版装备属性配置 SetByCaller Modifier。装备 DataAsset 不指定 GE 类，只在 `FDOAttributeModifierValues` 中直接填写数值；未填写的属性按 0 处理。

服务器穿戴装备时：

1. 读取装备基础数值。
2. 根据装备等级和强化曲线计算最终数值。
3. 创建 `FGameplayEffectSpecHandle`。
4. 写入 `Data.Equipment.*` SetByCaller。
5. 应用到 PlayerState 上的 ASC。
6. 按 Equipment.Slot Tag 保存 `FActiveGameplayEffectHandle`。

这里的“动态”指动态创建 `FGameplayEffectSpec` 并写入 SetByCaller，不是在运行时创建临时 `UGameplayEffect` 对象。联机复制需要客户端能够识别稳定的原生 GE 类，不能让每件装备生成只存在于服务器内存中的 GE 定义。

卸下装备时通过保存的 Handle 精确移除 GE。

不要把每件装备的属性直接写入 AttributeSet BaseValue，也不要让 UI 自行加总最终 AttackPower。最终面板值始终从 ASC 的 AttributeSet 读取，这样职业基础值、装备、Buff 和 Debuff 可以自然叠加。

### 13.3 强化计算

强化倍率使用 DataAsset 或 CurveTable 配置：

```text
FinalAttribute = BaseAttribute * (1 + UpgradeMultiplier[UpgradeLevel])
```

强化上限遵循现有设计：

```text
1-15 级装备：最高 +6
16 级以上装备：最高 +15
```

强化值由服务器计算并保存，客户端不得提交最终数值。

### 13.4 MaxHealth 与 MaxMana 策略

第一版采用“穿戴增加上限但不免费回复”的规则：

- 穿戴增加 MaxHealth 时，当前 Health 不自动增加。
- 卸下导致 MaxHealth 降低时，现有 AttributeSet 负责 Clamp 当前 Health。
- MaxMana 同理。

如果后续希望保持生命百分比，应在装备事务中统一实现，不能由 Widget 临时修正。

### 13.5 装备与外观边界

本阶段不把装备和角色外观绑定。装备组件只处理装备槽、职业/等级校验、属性 GameplayEffect 和装备事务；不复制外观摘要，也不加载 Mesh 或武器 Actor。

后续如果加入服饰/时装，单独创建外观组件和外观数据结构。服饰系统可以拥有自己的复制策略、外观变体和预览 Actor，但不能反向把服饰字段塞进 `UDOItemFragment_Equipment`，也不能让装备属性事务依赖外观加载结果。

## 14. 消耗品与快捷栏

快捷栏固定四格，与参考图一致。

快捷栏保存 `DefinitionId`，不保存某个堆栈的 InstanceId。这样背包整理、堆栈合并或拆分后绑定不会失效。使用时服务器查找最靠前的有效堆栈。

消耗品分两种：

- 简单消耗品：在 DataAsset 中直接填写回复量、限时属性和持续时间，由 C++ 原生 GE 动态构建 Spec，例如回复药水和攻击 Buff 药水。
- 复杂消耗品：触发 GameplayAbility，例如需要动画、目标选择或持续施法的道具。

简单消耗品不创建 GE 蓝图。`EffectKind` 决定使用 `UDOItemInstantRestoreEffect` 或 `UDOItemTimedAttributeEffect`；复杂流程才配置 `UseGameplayAbility` / `UseEventTag`。

使用流程：

```text
快捷键 1-4
  -> 本地 QuickBarComponent 查 DefinitionId
  -> Server_RequestUseItemByDefinition
  -> 服务器查找有效堆栈
  -> 校验死亡、沉默、公共冷却等状态
  -> 应用 GE 或激活 Ability
  -> 成功后扣除数量
  -> 复制背包变化
```

必须在效果成功应用后再扣除物品。失败激活不能吞掉消耗品。

## 15. UI 页面结构

### 15.1 页面类型

创建一个组合页面，而不是两个互相跳转的独立页面。页面生命周期使用 CommonUI，页面内容使用原生 Slate：

```text
UDOInventoryScreen -> UCommonActivatableWidget
  RebuildWidget() -> SDOInventoryEquipmentPanel
InputConfig = Menu
UI Layer = UI.Layer.Menu
```

组合页面更接近参考图，也能在选择背包装备时立即显示左侧装备对比。

第一版不使用 Widget Blueprint 搭建背包主体。`UDOInventoryScreen` 是 CommonUI 的原生壳，`SDOInventoryEquipmentPanel` 及其子控件全部使用 C++ Slate 创建。这样仍然可以被 `PrimaryGameLayout` 推入 `UI.Layer.Menu`，同时避免把核心布局和交互散落在蓝图中。

联机游戏中打开背包不暂停服务器世界。CommonUI 切换到 Menu 输入模式，并在本地屏蔽角色技能输入。关闭页面时恢复输入。

### 15.2 Slate 组件树

```text
UDOInventoryScreen
  SDOInventoryTopBar
  SDOCharacterEquipmentPanel
    SDOCombatPowerSummary
    SDOCharacterPreview
    SDOEquipmentSlot x 9
    SDOAttributeSummary
    SDOCurrencyBar
  SDOInventoryPanel
    SDOItemCategorySelector
    STileView<FDOInventorySlotViewModel>
    SDOInventoryPagination
    SDOItemQuickBar
    SDOInventoryActionBar
  SDOItemTooltip
  SDOItemCompareTooltip
  SMenuAnchor / SDOItemContextMenu
```

数量确认、丢弃确认和错误提示仍然放到 `UI.Layer.Modal`。如果这些弹窗也使用 Slate，则通过 CommonUI 的原生 `UCommonActivatableWidget` 壳推入 Modal Layer，不要直接创建独立操作系统窗口。

### 15.3 布局建议

设计基准为 1920x1080：

```text
主面板最大尺寸：1560 x 880
左侧宽度：约 46%
右侧宽度：约 54%
中间分隔：16-24 px
物品格：72-88 px，使用固定 AspectRatio 1:1
格子间距：8 px
快捷栏：4 个固定槽位
```

低分辨率下不缩成无法点击的小字：

- 保持格子最小 64 px。
- 属性详情可以切换成可滚动区域。
- 角色预览允许缩小，但装备槽保持固定交互尺寸。
- 使用项目 DPI Curve 适配，不按 viewport width 动态计算字体。

视觉上保留“左右书页”的识别，但使用 DragonOath 自有主题：深色半透明背景、暖金边框、明亮内容底色、品质色边框和青绿色正向数值。不要把整个界面做成单一黄色或单一紫色。

边框使用 9-Slice Brush 或 UI Material，避免位图拉伸变形。

### 15.4 原生 Slate 实现细节

推荐的 C++ 层级如下：

```text
UDOInventoryScreen : UCommonActivatableWidget
  - 持有 UDOInventoryViewModel
  - 重写 RebuildWidget()
  - 返回 SNew(SDOInventoryEquipmentPanel)

SDOInventoryEquipmentPanel : SCompoundWidget
  - UDOInventoryViewModel 的弱引用
  - SListView / STileView
  - 选中、分页、拖拽和快捷键处理

FDOInventorySlotViewModel
  - InstanceId、DefinitionId、StackCount、Icon、状态
  - 不持有 Slate Widget 指针
```

Slate 主要使用以下机制：

```text
SCompoundWidget              组合面板和自定义槽位
STileView                    虚拟化物品格子
FReply::BeginDragDrop        拖拽开始
FInventoryDragDropOperation  携带 InstanceId 和目标信息
SMenuAnchor                  上下文菜单和分类下拉菜单
MakeToolTip                  Tooltip
OnKeyDown / OnNavigation     键盘和手柄焦点
FSlateStyleSet               统一颜色、字体、边框和图标样式
FSlateBoxBrush               九宫格边框
```

`SWidget` 不是 `UObject`，不能直接依赖 UPROPERTY 反射和蓝图生命周期。必须遵守以下边界：

- Slate Widget 只保存 `TWeakObjectPtr` 或 `TSharedPtr` 的显示对象，不保存权威背包数据副本。
- Slate Widget 不直接调用服务器 RPC；由 `UDOInventoryViewModel` 或 `UDOInventoryScreen` 调用 InventoryComponent 的请求接口。
- ViewModel 收到 `Message.UI.Inventory.Changed` 后，先更新快照，再调用 `RequestListRefresh` 或更新受影响的 Entry。
- Widget 释放时注销 `FGameplayMessageListenerHandle`，不能让 Slate 回调捕获已销毁的 Widget。
- 图标和 Mesh 仍然使用 AssetManager 异步加载；资源加载完成后只触发对应 Slate 控件重绘。
- 不在 `Tick` 中轮询背包、ASC 或资源加载状态。

Slate 的样式集中注册到 `FDOInventoryStyle`。颜色、字体、品质边框、选中边框和禁用状态都从 StyleSet 读取，不能散落在各个 `SNew` 链中。这样后续换主题或制作手柄版式时不需要重写业务控件。

拖拽流程：

```text
SDOItemSlot::OnDragDetected
  -> FInventoryDragDropOperation{InstanceId, SourceSlot, Count}
  -> SDOEquipmentSlot::OnDrop / SDOItemSlot::OnDrop
  -> UDOInventoryViewModel::RequestMoveOrEquip
  -> Server RPC
  -> Replication
  -> Inventory.Changed / Equipment.Changed
```

这种方式保留纯 Slate 的输入控制，同时不让 Slate 绕过服务器权威规则。

## 16. 角色预览

角色预览使用本地 `SceneCapture2D + RenderTarget`：

```text
ADOCharacterPreviewActor（不复制）
  USceneCaptureComponent2D
  角色 SkeletalMesh 组件
  （后续可选）服饰外观组件
  Preview Animation Instance
```

实现要点：

- PreviewActor 只在本地客户端生成。
- SceneCapture 使用 ShowOnlyList，只渲染预览角色和预览灯光。
- 背景透明，通过 UI Material 合成到面板中。
- 不直接捕获玩家当前 Pawn，避免世界光照、摄像机距离和战斗动画影响预览。
- 从当前角色复制 Mesh、AnimClass 和基础材质；服饰系统接入后再读取独立的外观快照。
- 默认循环播放待机动画。
- 鼠标拖动或右摇杆可以旋转模型。
- 页面关闭时销毁 PreviewActor 或归还对象池。
- 预览不参与装备属性事务；服饰变化时由独立服饰系统刷新。

第一版如果预览管线影响进度，可以先使用静态角色立绘占位，但正式验收必须替换为实时角色模型。(先占位，暂时不展示角色)

## 17. 列表、分页与 ViewModel

物品格使用 `UTileView` 或 `UCommonListView` 的虚拟化能力，不手工创建数百个 Widget。

每个显示槽使用本地 UObject ViewModel：

```text
UDOInventorySlotViewModel
  SlotIndex
  InstanceId
  DefinitionId
  DisplayName
  Icon Soft Reference
  StackCount
  Rarity
  bIsEmpty
  bIsEquipped
  bIsUsable
  bIsPending
```

ViewModel 只是复制数据的显示快照，不允许修改 InventoryComponent。

参考图使用页码，因此第一版保留分页：

```text
PageSize = GridColumns * GridRows
PageCount = CeilToInt(FilteredItems / PageSize)
```

页面切换和分类筛选完全在客户端进行，不发送 RPC。

Definition 和 Icon 使用软引用异步加载。格子先显示占位图，加载完成后刷新对应 Entry；禁止在 Tick 或 `NativeOnListItemObjectSet` 中同步加载大资源。

## 18. 分类、选中与提示框

分类选择器使用 GameplayTagQuery：

```text
全部
武器
头部
肩部
背部
胸部
手套
裤子
鞋子
饰品
消耗品
材料
任务物品
```

选择物品时保存 InstanceId，不保存 Entry 数组指针。背包复制刷新后重新按 InstanceId 查找；物品已不存在时清空选择。

Tooltip 显示：

- 名称、品质、类型和部位。
- 需求等级和职业。
- 基础属性、强化等级和耐久。
- 使用效果或装备说明。
- 堆叠数量和出售价格。
- 是否绑定、锁定、不可交易或任务物品。

装备物品额外显示对比 Tooltip：

- 当前部位已装备物品。
- 候选装备与当前装备的属性差值。
- 增加值使用绿色，减少值使用红色。
- 对比只计算装备静态贡献，不临时修改 ASC。

最终角色属性仍从 ASC 读取。

## 19. 输入与交互

### 19.1 键鼠

```text
I / B              打开或关闭背包
左键               选择物品
双击               使用或穿戴
右键               打开上下文菜单
拖拽               移动、交换、装备或绑定快捷栏
Shift + 拖拽       拆分堆栈
Esc                关闭弹窗或返回
1-4                战斗中使用物品快捷栏
```

### 19.2 手柄

```text
左摇杆/方向键      移动焦点
A                  使用/穿戴/确认
X                  打开操作菜单
Y                  显示装备对比
B                  返回
LB/RB               切换主模块或分类
右摇杆              旋转角色预览
```

`BP_GetDesiredFocusTarget` 返回当前分页第一个可见物品；没有物品时返回分类按钮，保证手柄导航不丢焦点。

### 19.3 拖拽数据

`UDragDropOperation` 只携带：

```text
SourceContainerTag
InstanceId
RequestedCount
SourceSlotIndex
```

禁止携带 ItemDefinition 指针作为权威依据。Drop 目标只负责转换成服务器请求。

## 20. 操作栏设计

底部按钮与参考图一致，但根据上下文启用：

```text
出售：存在有效商店会话且选中物品可出售
仓库：存在有效仓库会话
修理：存在有效修理 NPC，装备耐久未满且货币足够
丢弃：物品允许丢弃
```

未满足条件时显示禁用状态和简短 Tooltip，不弹出无意义错误。

商店、仓库和修理必须由服务器生成交互会话 Token。客户端仅凭打开 Widget 不能获得操作权限。

## 21. UI 刷新与消息总线

推荐流程：

```text
服务器修改 Inventory FastArray
  -> Owner 客户端收到 Delta Replication
  -> FDOInventoryList PostReplicatedAdd/Change/Remove
  -> UDOInventoryComponent 聚合本帧变化
  -> Broadcast Message.UI.Inventory.Changed
  -> InventoryScreen 重新查询受影响槽位
  -> Slot ViewModel 更新
```

消息 Payload 建议使用专用结构：

```cpp
USTRUCT(BlueprintType)
struct FDOInventoryChangedMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UDOInventoryComponent> InventoryComponent;

    UPROPERTY(BlueprintReadOnly)
    TArray<FGuid> ChangedInstanceIds;

    UPROPERTY(BlueprintReadOnly)
    int32 Revision = 0;
};
```

不要通过 GameplayMessageRouter 请求移动、使用或穿戴物品。请求必须直接调用组件 RPC。

Widget 激活时注册 Listener，停用或销毁时注销。页面重新激活后先主动读取完整快照，不能假设页面关闭期间仍接收了所有本地消息。

## 22. 实现阶段

### Phase 0：数据和 Tag 基础

- 新增 Item、Equipment、Data.Equipment 和 Message.UI Tag。
- 创建 `UDOItemDefinition` 与三个基础 Fragment。
- 创建 `FDOAttributeModifierValues`、`FDOResourceRestoreValues` 和道具效果配置结构体。
- 配置 AssetManager 扫描物品定义。
- 创建 8-12 个测试物品资产和占位图标。
- 创建装备、即时回复、限时属性和冷却四类 C++ 原生 GE 模板。

验收：编辑器可以创建物品 DataAsset，DataAsset 校验能发现缺失 Tag、图标和非法堆叠数。

### Phase 1：服务器背包核心

- 创建 Inventory Types 和 FastArray。
- 创建 PlayerState 上的 InventoryComponent。
- 实现添加、移除、堆叠、移动、交换、拆分和整理。
- 实现 Owner-only 复制和变更消息。
- 创建调试命令添加测试物品。

验收：Listen Server 给客户端添加物品时，只有该客户端收到完整背包数据。

### Phase 2：装备和 GAS

- 创建 EquipmentComponent 和装备 FastArray。
- 实现九个装备部位。
- 实现穿戴、卸下和交换事务。
- 通过 `FDOItemEffectSpecBuilder` 动态构建 Spec，并用 SetByCaller 应用装备属性。
- 保存并移除 ActiveGameplayEffectHandle。
- 装备外观摘要复制暂不纳入本阶段；装备系统只负责装备槽、属性和 GAS。

验收：客户端穿戴装备后，服务器权威属性变化，Owner UI 更新；角色外观由后续独立的服饰系统负责。

### Phase 3：组合背包 UI

- 创建 `UDOInventoryScreen`，由其承载原生 Slate 背包主体。
- 完成左右双栏、装备槽、属性面板、物品格和分页。
- 实现分类选择器、选中状态和 Tooltip。
- 实现拖拽、拆分弹窗和丢弃确认。
- 接入 CommonUI Menu Layer、返回和手柄焦点。

验收：键鼠和手柄都能完成浏览、筛选、穿戴和整理。

### Phase 4：快捷栏

- 保留本地基础角色预览捕获器和 RenderTarget。
- UI 使用 RenderTarget 占位；服饰预览由后续独立系统接入。
- 角色预览保留为基础角色占位；服饰外观同步延期到独立服饰系统，本阶段不把装备与角色外观绑定。
- 创建 QuickBarComponent 和 HUD 快捷栏。
- 接入简单消耗品动态 Spec 与复杂消耗品 Ability/Event 使用流程。

验收：基础角色预览可用；快捷键成功使用药水并由服务器扣除数量。

### Phase 5：出售、仓库和修理

- 创建 Vendor/Storage/Repair 交互会话。
- 实现服务端距离、权限、价格和货币校验。
- 实现仓库与背包之间的事务移动。
- 实现耐久和修理价格。

验收：离开 NPC 后旧会话立即失效，客户端不能伪造价格或远程操作。

### Phase 6：存档与自动化测试

- 创建 Inventory/Equipment/QuickBar SaveData。
- 加入 SaveVersion 和迁移入口。
- 完成加载后重新应用装备 GE。
- 添加核心算法自动化测试。
- 完成 Listen Server + 1 Client 测试清单。

## 23. 存档设计

```cpp
USTRUCT()
struct FDOInventorySaveData
{
    GENERATED_BODY()

    UPROPERTY()
    int32 SaveVersion = 1;

    UPROPERTY()
    int32 Capacity = 40;

    UPROPERTY()
    TArray<FDOItemInstanceRecord> InventoryItems;

    UPROPERTY()
    TArray<FDOEquippedItemEntry> EquippedItems;

    UPROPERTY()
    TArray<FPrimaryAssetId> QuickBarDefinitions;
};
```

加载顺序：

1. 服务器读取 SaveGame。
2. 校验 DefinitionId 是否仍存在。
3. 执行版本迁移。
4. 恢复背包和装备 FastArray。
5. 重新创建装备 GE Handle。
6. 恢复快捷栏。
7. 标记数组复制。

不要保存 `FActiveGameplayEffectHandle`、UObject 指针、Widget 状态或软加载句柄。

正式联网后，SaveData 可以映射到服务器账号数据；`FPrimaryAssetId + FGuid + 数值字段` 的设计不需要推翻。

## 24. 战力和属性面板

参考图的“战力”和“守护力”属于汇总展示值。DragonOath 中建议创建纯 C++ 计算器和配置资产：

```text
UDOCombatRatingConfig
UDOCombatRatingLibrary
```

计算器读取 ASC 已复制的最终属性，按配置权重计算：

```text
CombatPower = AttackPower、CriticalRating、HitRating、AttackSpeed 等加权结果
GuardPower  = DefensePower、MaxHealth、EvasionRating、MoveSpeed 等加权结果
```

权重不写死在 Widget。该数值只用于展示和推荐，不参与伤害结算。

属性面板第一版显示：

```text
AttackPower
DefensePower
MaxHealth
MaxMana
CriticalRating
HitRating
EvasionRating
AttackSpeed
MoveSpeed
LifeStealRate
```

元素属性等 `DOElementSet` 完成后再加入对应分组。

## 25. 性能要求

- 背包复制必须使用 FastArray，不复制整个 TArray 快照。
- 背包数据使用 `COND_OwnerOnly`。
- 物品格使用虚拟化 TileView/ListView。
- Widget 不使用 Tick 轮询背包或 ASC。
- 属性变化通过 ASC Delegate，背包变化通过本地消息。
- 图标和复杂 Tooltip 资源使用软引用异步加载；预览 Mesh 属于后续服饰系统。
- ItemDefinition 解析结果可由本地缓存复用。
- 一次批量整理只广播一次聚合刷新消息。
- SceneCapture 在页面关闭时停止 CaptureEveryFrame。
- 静态预览状态可以按需 `CaptureScene()`，不必永久每帧捕获。

## 26. 自动化与联机测试

### 26.1 核心算法

- 空背包添加可堆叠物品。
- 优先填充半满堆栈。
- 背包满时返回剩余数量。
- 拆分生成新 InstanceId。
- 合并不超过 MaxStackSize。
- 不同 DefinitionId 不能合并。
- 整理结果稳定且无重复条目。
- 删除最后一个数量时正确移除条目。

### 26.2 装备事务

- 穿戴到空槽。
- 替换已有装备。
- 背包已满时替换装备失败且数据不变。
- 职业或等级不满足时失败。
- 卸下装备后 GE 被精确移除。
- 重生后装备 GE 不重复叠加。
- Save/Load 后背包、装备和属性恢复；外观恢复属于后续服饰系统。

### 26.3 网络

- Listen Server + 1 Client 拾取和穿戴。
- Client 不能直接修改 FastArray。
- Client A 看不到 Client B 的完整背包。
- Client A/Client B 的装备外观同步不属于本阶段验收。
- 连续快速 RPC 不产生复制或负数数量。
- 高延迟下 Pending 状态最终正确清除。
- 玩家断线和重连后数据一致。

### 26.4 UI

- 1920x1080、2560x1440、1366x768 下无文字重叠。
- 键鼠和手柄都能完成核心操作。
- 分类为空时焦点不丢失。
- 页面关闭再打开后选择和分页状态符合设计。
- 异步图标加载失败时显示占位图。
- Tooltip 不超出安全区域。

## 27. 主要风险与处理

### 风险一：UI 先行导致业务写进蓝图

处理：先完成 C++ InventoryComponent 和 EquipmentComponent，再制作正式 Widget。Widget 只调用查询和请求接口。

### 风险二：装备和背包跨组件事务丢物品

处理：所有前置条件一次性校验完成后再修改列表；为关键操作编写失败不变性测试。

### 风险三：装备 GE 重复叠加

处理：按装备槽保存 Handle；ASC 重新初始化时先清理旧 Handle，再从 EquipmentList 重建。

### 风险四：复制内容泄露或带宽浪费

处理：完整背包和装备实例 Owner-only；本阶段不复制装备外观摘要，后续服饰系统单独定义外观复制。

### 风险五：PrimaryAsset 尚未配置

处理：Phase 0 同时修改 AssetManager 扫描规则，并添加 Cook 验证，避免编辑器可用但打包缺资产。

### 风险六：实时角色预览影响帧率（延期项）

处理：ShowOnlyList、低分辨率 RenderTarget、页面关闭停止捕获、静止时按需捕获。

## 28. 第一版验收标准

以下条件全部满足，背包原型才算完成：

- 页面结构与参考图一致：左侧角色装备，右侧分类背包和操作区。
- 物品定义完全数据驱动。
- 背包在 PlayerState 上持久存在。
- 拾取、移动、交换、堆叠、拆分、整理和丢弃可用。
- 九个装备部位可穿戴和卸下。
- 装备通过 GAS 正确修改角色属性。
- 角色面板读取 ASC 最终属性。
- 角色预览与装备外观绑定暂不验收；后续服饰系统单独定义预览方案。
- 四格物品快捷栏可使用消耗品。
- 分类、分页、Tooltip 和装备对比可用。
- CommonUI 返回、焦点和输入切换正确。
- Listen Server + 1 Client 下服务器权威和 Owner-only 复制正确。
- SaveGame 可以保存并恢复背包、装备和快捷栏。
- 核心算法和装备事务自动化测试通过。

## 29. 推荐开发顺序结论

不要先在 Blueprint 中搭满整套可点击界面。正确顺序是：

```text
ItemDefinition/GameplayTag
  -> Inventory FastArray
  -> Equipment + GAS
  -> 组合背包 UI
  -> 快捷栏
  -> 商店/仓库/修理
  -> SaveGame 和自动化测试
```

这样第一版 UI 接入的是真实、可复制、可存档的数据，不需要在后续联机化时推翻背包结构。

## 30. Lua 使用边界

### 30.1 当前项目状态

当前仓库中存在 `Plugins/UnLua/UnLua.uplugin`，版本为 2.3.6，但 `DragonOath.uproject` 没有启用 `UnLua`。`UnLuaExtensions` 下的 LuaProtobuf、LuaRapidjson 和 LuaSocket 也没有在项目插件列表中启用。

因此，当前背包方案不依赖 Lua，也不应该为了制作背包而临时启用 Lua Runtime。

### 30.2 纯 Slate 下的 Lua 适配方式

UnLua 主要绑定 `UCLASS`、`UFUNCTION`、`UPROPERTY` 和 `USTRUCT`。原生 `SWidget` 不是 UObject，不能把整个 Slate Widget 树直接当成 Lua 对象来维护。

如果未来确实需要 Lua，推荐使用以下边界：

```text
原生 Slate Widget
  -> C++ UDOInventoryScreen / UDOInventoryViewModel
  -> UFUNCTION 形式的稳定接口
  -> 可选的 Lua Controller
```

Lua 只调用 `UDOInventoryViewModel` 暴露的高层接口，例如：

```text
SetCategory(Tag)
SetSortMode(Mode)
SelectItem(InstanceId)
RequestUseSelectedItem()
RequestEquipSelectedItem()
GetTooltipData(InstanceId)
```

Lua 不直接操作 `SWidget` 的子控件，也不持有 `TSharedPtr<SWidget>`，避免 Lua GC 和 Slate 引用计数互相影响。

### 30.3 适合使用 Lua 的地方

如果将来启用并验证 UnLua，Lua 可以用于低风险、变化频繁的编排层：

- 背包分类的显示顺序和特殊活动分类。
- Tooltip 的活动文案、提示规则和运营标签。
- 商店、仓库、修理界面的显示流程。
- 活动物品的展示规则和红点提示。
- 新手引导、背包操作教学和界面高亮顺序。
- 非权威的筛选、排序和推荐装备展示。
- 临时活动中“选中物品后展示什么内容”的界面流程。
- 测试工具，例如批量生成测试物品、切换分类和验证 Tooltip。

这些逻辑的共同特点是：失败不会改变真实物品，且可以由 C++ 接口重新查询和校验。

### 30.4 不适合使用 Lua 的地方

以下内容必须保留在 C++ / GAS / 服务器路径：

- `FFastArraySerializer` 的真实库存数据。
- 物品数量、InstanceId、堆叠和事务提交。
- Server RPC、权限校验和交互距离校验。
- 装备穿戴、卸下和防重复事务。
- 装备 GE、AttributeSet 和最终属性计算。
- 消耗品成功判定和扣除数量。
- SaveGame 序列化和版本迁移。
- 高频 Slate Paint、Tick、列表虚拟化和资源生命周期。
- 任何可以产生复制物品、越权装备或货币异常的规则。

### 30.5 推荐结论

第一版采用：

```text
CommonUI       页面生命周期、层级、输入模式和返回
原生 Slate     背包主体布局、格子、拖拽、Tooltip、分页和视觉样式
C++ ViewModel  显示快照、消息监听和操作请求编排
C++/GAS        背包规则、装备属性、服务器权威和网络复制
Lua            暂不启用；未来只作为可选的低风险 UI/活动编排层
```

这样既能获得 Slate 对密集型背包布局的控制力，又不会让背包系统依赖尚未验证的 Lua/UE 5.8 适配链。

## 31. 改进方向：C++ GE 模板与数据驱动动态 Spec

> 实施状态：本节是下一轮 C++ 改造的目标方案，当前代码尚未完成该迁移。现有装备 Fragment 仍使用
> `BaseAttributeMagnitudes`，消耗品 Fragment 仍保留 `UseGameplayEffect`。在第 31.14 节迁移完成前，
> 不要删除旧字段或批量改写现有资产；迁移期间由 C++ 同时兼容新旧字段。

### 31.1 改进目标

当前装备已经使用 C++ 原生 `UDOEquipmentAttributeEffect`，但装备属性仍通过 Tag Map 填写，消耗品仍可直接指定 GE，物品配置容易退化成“每件装备或药水创建一个 GE 蓝图”。后续改造统一采用以下模式：

```text
ItemDefinition / Fragment 直接填写数值
  -> 类型化 C++ 结构体
  -> FDOItemEffectSpecBuilder
  -> 选择稳定的 C++ 原生 GameplayEffect 类
  -> 运行时创建 FGameplayEffectSpec
  -> 写入 SetByCaller 数值、持续时间和动态 Tag
  -> 服务器应用到 ASC
```

目标：

- 普通装备、回复药水和限时属性药水不创建 GE 蓝图。
- 装备和道具 DataAsset 直接显示可填写的攻击、防御、生命、法力等字段。
- 新增受支持属性时只修改公共结构体、原生 GE 映射和 Builder 映射，所有装备/道具资产自动出现新字段。
- GE 的时长策略、堆叠规则、复制和 AttributeSet 目标由 C++ 固定，策划只填写数值。
- 复杂道具继续使用 GameplayAbility，不把动画、选目标或多阶段逻辑塞进通用 GE。

### 31.2 “动态创建”的正确含义

本方案动态创建的是 `FGameplayEffectSpec`，不是运行时 `NewObject<UGameplayEffect>` 后临时拼 Modifier。

```text
正确：稳定 C++ GE Class/CDO + 动态 Spec + SetByCaller
错误：每次使用物品时创建瞬态 UGameplayEffect 定义并动态增加 Modifier
```

原因：

- GE 定义需要在服务器和客户端都有稳定类路径，才能可靠复制 ActiveGameplayEffect。
- 瞬态 GE 对象只存在于服务器内存，客户端无法可靠解析其定义。
- 运行时修改 GE CDO 会污染同类所有实例，也不满足线程和生命周期要求。
- `FGameplayEffectSpec` 本来就是 GAS 用来承载“本次应用的动态数值、来源、等级、Tag 和持续时间”的对象。

### 31.3 建议目录

继续按功能域组织，不创建顶层 Public/Private：

```text
Source/DragonOath/
  ItemSystem/
    Core/
      DOItemAttributeTypes.h
      DOItemDefinition.h/.cpp
    Inventory/
      DOInventoryTypes.h
      DOInventoryComponent.h/.cpp
      DOInventoryMessages.h
    Equipment/
      DOEquipmentTypes.h
      DOEquipmentComponent.h/.cpp
    Usage/
      DOItemUseTypes.h/.cpp
      DOItemUseEffects.h/.cpp
    QuickBar/
      DOItemQuickBarComponent.h/.cpp
      DOItemQuickBarViewModel.h/.cpp
    Pickup/
      DOItemPickup.h/.cpp
    AbilitySystem/
      DOItemGameplayEffects.h/.cpp
      DOItemEffectSpecBuilder.h/.cpp
      DOGameplayAbility_UseItem.h/.cpp
    Tests/
      DOInventoryAutomationTests.cpp
```

需要创建或调整的核心类型如下：

| 类型 | 职责 |
|---|---|
| `FDOAttributeModifierValues` | 装备和限时 Buff 共用的类型化属性数值 |
| `FDOResourceRestoreValues` | 生命、法力、体力等即时回复数值 |
| `FDOItemTimedModifierValues` | 限时属性、持续时间和授予 Tag |
| `FDOItemCooldownConfig` | 道具冷却 Tag 与冷却时长 |
| `EDOConsumableEffectKind` | 选择即时回复、限时属性、Ability 或 Event 流程 |
| `UDOEquipmentAttributeEffect` | 所有装备共用的无限时长原生 GE |
| `UDOItemInstantRestoreEffect` | 所有简单回复道具共用的瞬时原生 GE |
| `UDOItemTimedAttributeEffect` | 所有限时属性道具共用的持续型原生 GE |
| `UDOItemCooldownEffect` | 所有道具冷却共用的持续型原生 GE |
| `FDOItemEffectSpecBuilder` | 把 DataAsset 数值转换为服务器可应用的 Spec |
| `UDOGameplayAbility_UseItem` | 复杂道具 Ability 的 C++ 基类，保存使用上下文并提交物品消耗 |

这些类型均由 C++ 提供。普通装备和简单道具资产不新增 GE 蓝图，也不在 DataAsset 中暴露任意 `GameplayEffectClass`。

### 31.4 SetByCaller Tag

第一轮改造可继续复用现有 `Data.Equipment.*`，避免立刻迁移已有装备资产：

```text
Data.Equipment.AttackPower
Data.Equipment.DefensePower
Data.Equipment.MaxHealth
Data.Equipment.MaxMana
Data.Equipment.CriticalRating
Data.Equipment.HitRating
Data.Equipment.EvasionRating
Data.Equipment.AttackSpeed
Data.Equipment.MoveSpeed
Data.Equipment.LifeStealRate
```

道具即时回复新增：

```text
Data.ItemUse.Healing
Data.ItemUse.ManaRestore
Data.ItemUse.StaminaRestore
Data.ItemUse.Duration
Data.ItemUse.CooldownDuration
```

所有 Tag 继续集中声明在 `DOGameplayTag.h/.cpp`，并使用中文注释。不要在 Builder、Fragment 或 GE 构造函数中手写 Tag 字符串。

### 31.5 公共属性结构体

创建 `ItemSystem/Core/DOItemAttributeTypes.h`。装备和限时 Buff 共用同一套属性字段，避免两边重复维护 Map。

```cpp
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOAttributeModifierValues
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float AttackPower = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float DefensePower = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float MaxHealth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float MaxMana = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float CriticalRating = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float HitRating = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float EvasionRating = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float AttackSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float MoveSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0"))
    float LifeStealRate = 0.0f;

    bool IsNearlyZero() const;
};
```

使用类型化字段而不是 `TMap<FGameplayTag, FScalableFloat>` 的原因：

- DataAsset 面板直接显示属性名称，填写速度更快。
- 不会选错不属于装备/道具的 GameplayTag。
- Tooltip、战力计算和数据校验可以直接访问字段。
- 新增公共属性后，所有相关资产自动显示该字段。

当前物品定义没有独立 `ItemLevel`，旧实现也始终按 Level 1 读取 `FScalableFloat`，因此新结构体直接使用 `float`，让编辑器面板只显示可填写数值。强化倍率继续由装备实例的 `UpgradeLevel` 和统一强化曲线计算。未来如果加入物品等级成长，应在 Builder 中读取统一 CurveTable/DataAsset，不要把每个字段重新改成由单个物品维护的曲线。

即时回复单独使用资源结构体，避免把“回复生命”和“增加生命上限”混在一起：

```cpp
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOResourceRestoreValues
{
    GENERATED_BODY()

    // 写入 HealthSet.Healing Meta Attribute，不直接修改 Health。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "回复", meta = (ClampMin = "0.0"))
    float Healing = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "回复", meta = (ClampMin = "0.0"))
    float ManaRestore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "回复", meta = (ClampMin = "0.0"))
    float StaminaRestore = 0.0f;

    bool IsNearlyZero() const;
};
```

结构体只保存和校验数据，不直接依赖 ASC 或修改 Spec。字段到 SetByCaller Tag 的映射统一放在
`FDOItemEffectSpecBuilder` 中，避免 DataAsset 数据类型承担 GAS 应用职责。

### 31.6 消耗品配置结构体

第一版一个简单消耗品只执行一种直接效果。需要同时回复、加 Buff、播放动画或选择目标时，改用 GameplayAbility，避免多个 GE 中途失败后难以回滚。

```cpp
UENUM(BlueprintType)
enum class EDOConsumableEffectKind : uint8
{
    None UMETA(DisplayName = "未配置"),
    InstantRestore UMETA(DisplayName = "即时回复"),
    TimedAttributeModifier UMETA(DisplayName = "限时属性"),
    GameplayAbility UMETA(DisplayName = "Gameplay Ability"),
    GameplayEvent UMETA(DisplayName = "Gameplay Event")
};

USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemTimedModifierValues
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "限时效果", meta = (ClampMin = "0.0"))
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "限时效果", meta = (ShowOnlyInnerProperties))
    FDOAttributeModifierValues Modifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "限时效果")
    FGameplayTagContainer GrantedTags;
};

USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemCooldownConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "冷却")
    FGameplayTag CooldownTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "冷却", meta = (ClampMin = "0.0"))
    float DurationSeconds = 0.0f;

    bool IsEnabled() const;
};
```

`UDOItemFragment_Consumable` 改为：

```cpp
UCLASS(EditInlineNew, DefaultToInstanced)
class DRAGONOATH_API UDOItemFragment_Consumable : public UDOItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "道具效果")
    EDOConsumableEffectKind EffectKind = EDOConsumableEffectKind::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "道具效果",
        meta = (ShowOnlyInnerProperties, EditCondition = "EffectKind == EDOConsumableEffectKind::InstantRestore", EditConditionHides))
    FDOResourceRestoreValues InstantRestore;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "道具效果",
        meta = (ShowOnlyInnerProperties, EditCondition = "EffectKind == EDOConsumableEffectKind::TimedAttributeModifier", EditConditionHides))
    FDOItemTimedModifierValues TimedModifier;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "道具效果",
        meta = (EditCondition = "EffectKind == EDOConsumableEffectKind::GameplayAbility", EditConditionHides))
    TSubclassOf<UGameplayAbility> UseGameplayAbility;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "道具效果",
        meta = (EditCondition = "EffectKind == EDOConsumableEffectKind::GameplayEvent", EditConditionHides))
    FGameplayTag UseEventTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "冷却", meta = (ShowOnlyInnerProperties))
    FDOItemCooldownConfig Cooldown;
};
```

`EditConditionHides` 根据 `EffectKind` 隐藏无关字段，减少策划误填。原来的 `UseGameplayEffect` 标记为 Deprecated，完成资产迁移后删除。

装备 Fragment 改为直接内嵌属性：

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "装备属性", meta = (ShowOnlyInnerProperties))
FDOAttributeModifierValues AttributeModifiers;
```

### 31.7 C++ 原生 GameplayEffect 模板

创建 `ItemSystem/AbilitySystem/DOItemGameplayEffects.h/.cpp`，只需要四个稳定类：

```text
UDOEquipmentAttributeEffect
  DurationPolicy = Infinite
  所有 FDOAttributeModifierValues 字段对应一个 SetByCaller Modifier

UDOItemInstantRestoreEffect
  DurationPolicy = Instant
  Healing / ManaRestore / StaminaRestore 对应 SetByCaller Modifier

UDOItemTimedAttributeEffect
  DurationPolicy = HasDuration
  所有 FDOAttributeModifierValues 字段对应一个 SetByCaller Modifier

UDOItemCooldownEffect
  DurationPolicy = HasDuration
  无属性 Modifier，Spec 动态添加 CooldownTag
```

GE 构造函数负责固定 Attribute 和 SetByCaller Tag 的映射。例如：

```cpp
UDOItemInstantRestoreEffect::UDOItemInstantRestoreEffect()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;
    AddSetByCallerModifier(UDOHealthSet::GetHealingAttribute(), DragonOathGameplayTags::Data::ItemUse::Healing);
    AddSetByCallerModifier(UDOResourceSet::GetManaAttribute(), DragonOathGameplayTags::Data::ItemUse::ManaRestore);
    AddSetByCallerModifier(UDOResourceSet::GetStaminaAttribute(), DragonOathGameplayTags::Data::ItemUse::StaminaRestore);
}
```

`AddSetByCallerModifier` 做成 `.cpp` 内部辅助函数，不暴露给蓝图。

### 31.8 Spec Builder

创建普通 C++ Builder，不需要 `UBlueprintFunctionLibrary`，因为权威调用只发生在服务器组件内部：

```cpp
class DRAGONOATH_API FDOItemEffectSpecBuilder final
{
public:
    static bool BuildEquipmentSpec(
        UDOAbilitySystemComponent& ASC,
        UObject& SourceObject,
        const FDOAttributeModifierValues& Values,
        float UpgradeScale,
        FGameplayEffectSpecHandle& OutSpec);

    static bool BuildInstantRestoreSpec(
        UDOAbilitySystemComponent& ASC,
        UObject& SourceObject,
        const FDOResourceRestoreValues& Values,
        FGameplayEffectSpecHandle& OutSpec);

    static bool BuildTimedModifierSpec(
        UDOAbilitySystemComponent& ASC,
        UObject& SourceObject,
        const FDOItemTimedModifierValues& Values,
        FGameplayEffectSpecHandle& OutSpec);

    static bool BuildCooldownSpec(
        UDOAbilitySystemComponent& ASC,
        UObject& SourceObject,
        const FDOItemCooldownConfig& Cooldown,
        FGameplayEffectSpecHandle& OutSpec);
};
```

Builder 的共同步骤：

1. 校验 ASC 和 `AbilityActorInfo`。
2. 用原生 GE Class 调用 `MakeOutgoingSpec`。
3. 读取 Fragment 中直接填写的 `float`；装备属性额外乘以服务器计算的 `UpgradeScale`。
4. 通过 Builder 内部的属性映射函数写入全部 SetByCaller；通用 GE 已声明的字段即使未配置也显式写入 0，避免 GAS 输出缺少 SetByCaller 的错误日志。
5. 限时效果调用 `Spec.SetDuration()` 写入持续时间。
6. 冷却 Spec 添加动态 GrantedTag，并写入冷却时长。
7. 返回 Spec，不在 Builder 内直接修改背包或扣除数量。

Builder 只负责“数据 -> Spec”。装备事务、物品扣除、回滚和 Handle 保存仍由对应组件负责。

Builder 内部建议再拆出以下私有方法，集中维护属性映射和公共校验：

```cpp
static void WriteAttributeMagnitudes(
    FGameplayEffectSpec& Spec,
    const FDOAttributeModifierValues& Values,
    float Scale);

static void WriteRestoreMagnitudes(
    FGameplayEffectSpec& Spec,
    const FDOResourceRestoreValues& Values);

static bool InitializeSpec(
    UDOAbilitySystemComponent& ASC,
    UObject& SourceObject,
    TSubclassOf<UGameplayEffect> EffectClass,
    FGameplayEffectSpecHandle& OutSpec);
```

这三个方法不暴露给蓝图。新增属性时只需在公共结构体、原生 GE Modifier 和
`WriteAttributeMagnitudes` 中增加对应项，不需要修改每个装备或道具资产的 GE。

### 31.9 装备应用方法

`UDOEquipmentComponent` 保留以下职责：

```cpp
bool BuildEquipmentEffectSpec(
    const FDOItemInstanceRecord& Item,
    const UDOItemFragment_Equipment& Fragment,
    FGameplayEffectSpecHandle& OutSpec) const;

bool ApplyEquipmentEffect(
    const FDOItemInstanceRecord& Item,
    const UDOItemFragment_Equipment& Fragment,
    FActiveGameplayEffectHandle& OutHandle);

void RemoveEquipmentEffect(const FGameplayTag& SlotTag);
```

应用流程：

```text
读取 Fragment.AttributeModifiers
  -> 计算 UpgradeScale
  -> FDOItemEffectSpecBuilder::BuildEquipmentSpec
  -> ASC.ApplyGameplayEffectSpecToSelf
  -> 按装备槽保存 Handle
```

每件装备仍然拥有独立 ActiveGameplayEffectHandle。即使所有装备共用一个 C++ GE 类，卸下时也能精确移除对应实例。

### 31.10 消耗品服务器事务

`UDOInventoryComponent` 增加或拆分以下内部方法：

```cpp
bool CanUseConsumable(
    const FDOItemInstanceRecord& Item,
    const UDOItemFragment_Consumable& Fragment,
    EDOInventoryFailureReason& OutFailureReason) const;

bool ApplyDirectConsumableEffect(
    const FDOItemInstanceRecord& Item,
    const UDOItemFragment_Consumable& Fragment,
    FActiveGameplayEffectHandle& OutPersistentEffectHandle,
    EDOInventoryFailureReason& OutFailureReason);

bool ApplyConsumableCooldown(
    const UDOItemFragment_Consumable& Fragment,
    FActiveGameplayEffectHandle& OutCooldownHandle,
    EDOInventoryFailureReason& OutFailureReason);

bool BeginComplexConsumableUse(
    const FDOItemInstanceRecord& Item,
    const UDOItemFragment_Consumable& Fragment,
    EDOInventoryFailureReason& OutFailureReason);

bool CommitConsumableUse(
    const FGuid& InstanceId,
    const FPrimaryAssetId& ExpectedDefinitionId,
    EDOInventoryFailureReason& OutFailureReason);
```

`ApplyDirectConsumableEffect` 只接受 `InstantRestore` 和 `TimedAttributeModifier`。推荐同步事务顺序：

```text
预校验物品数量、死亡状态、EffectKind 和冷却
  -> 确认本次扣除必然可提交
  -> 应用冷却 GE，并保存临时 Handle
  -> 应用即时回复或限时属性 GE
  -> 主效果失败：移除刚应用的冷却和持久效果
  -> 主效果成功：扣除一个物品
  -> 广播背包变化
```

即时 GE 执行后不能回滚，所以“可扣除数量”必须在应用效果前一次性校验完成；预校验与扣除提交之间不能插入异步等待。

`GameplayAbility` 和 `GameplayEvent` 不进入上述同步事务。`BeginComplexConsumableUse` 只负责服务器校验并启动复杂流程，不立即扣除物品。Ability 或事件处理器完成动画、目标选择等所有可失败步骤后，在施加不可逆效果前调用服务器内部的 `CommitConsumableUse`；提交时重新校验 `InstanceId`、`DefinitionId`、数量和冷却，成功后才能继续应用最终效果。

`CommitConsumableUse` 不能开放成允许客户端直接调用并传入效果数值的 RPC。蓝图 Ability 如需提交，应继承项目提供的 C++ 道具 Ability 基类，由基类保存服务器生成的使用上下文并调用该内部方法。

复杂道具第一次落地时创建 `UDOGameplayAbility_UseItem`：

```cpp
UCLASS(Abstract)
class DRAGONOATH_API UDOGameplayAbility_UseItem : public UDOGameplayAbility
{
    GENERATED_BODY()

protected:
    // 只提交服务器创建的物品使用上下文，不接受蓝图传入属性值或冷却值。
    UFUNCTION(BlueprintCallable, Category = "DO|物品")
    bool CommitItemUse();

private:
    FGuid SourceItemInstanceId;
    FPrimaryAssetId SourceItemDefinitionId;
};
```

`BeginComplexConsumableUse` 在服务器激活 Ability 前写入这两个字段。蓝图子类只能在最终执行点调用 `CommitItemUse`，不能自行构造物品 ID、扣除数量或应用任意 GE。

### 31.11 编辑器操作方式

普通装备：

1. 创建 `DOItemDefinition` DataAsset，例如 `DA_Item_Equipment_Sword_01`。
2. 添加 `UDOItemFragment_Inventory` 和 `UDOItemFragment_Equipment`。
3. 填写装备部位、需求等级、职业 Query 和耐久。
4. 在 `AttributeModifiers` 中直接填写攻击、防御、生命等数值。
5. 不创建、不指定任何 GE 蓝图。

回复药水：

1. 添加 `UDOItemFragment_Consumable`。
2. `EffectKind = InstantRestore`。
3. 在 `InstantRestore` 中填写 `Healing`、`ManaRestore` 或 `StaminaRestore`。
4. 需要公共冷却时填写 `CooldownTag` 和 `DurationSeconds`。
5. 不创建 GE 蓝图。

限时 Buff 道具：

1. `EffectKind = TimedAttributeModifier`。
2. 填写 `TimedModifier.DurationSeconds`。
3. 在 `TimedModifier.Modifiers` 中填写攻击、防御、移速等增量。
4. 可选填写激活期间需要授予的状态 Tag。
5. 不创建 GE 蓝图。

复杂道具：

1. 有动画、目标选择、多段效果或异步流程时选择 `GameplayAbility`。
2. 填写 `UseGameplayAbility`，Ability 负责复杂流程。
3. Ability 仍调用背包组件的服务器接口提交消耗，不能在蓝图中直接修改数量。

### 31.12 新增属性时如何扩展

例如增加 `ElementalPower`：

1. 在对应 AttributeSet 增加 `ElementalPower` 属性及复制代码。
2. 在 `DOGameplayTag.h/.cpp` 增加 `Data.Equipment.ElementalPower`，注释用中文。
3. 在 `FDOAttributeModifierValues` 增加 `float ElementalPower = 0.0f`。
4. 在 `UDOEquipmentAttributeEffect` 和 `UDOItemTimedAttributeEffect` 构造函数增加 SetByCaller Modifier。
5. 在 `FDOItemEffectSpecBuilder::WriteAttributeMagnitudes` 增加字段到 Tag 的映射。
6. 在 Tooltip、装备对比和战力配置中按需要增加显示/权重。
7. 添加自动化测试，验证装备、卸下、Buff 到期和存档恢复。

完成后所有装备和限时道具 DataAsset 会自动出现 `ElementalPower` 字段，不需要批量创建或修改 GE 蓝图。

### 31.13 Data Validation

`UDOItemDefinition::IsDataValid` 增加以下规则：

- `EffectKind` 不能为 `None`；新建消耗品必须明确选择一种使用方式。
- 装备 `MaxStackSize` 必须为 1，且 `AttributeModifiers` 至少有一个非零属性或明确允许纯功能装备。
- `InstantRestore` 至少填写一个回复字段，不能同时填写 TimedModifier/Ability/Event。
- `TimedAttributeModifier` 的持续时间必须大于 0，并至少包含一个属性或 GrantedTag。
- `GameplayAbility` 必须配置 Ability Class，直接效果字段必须为空。
- `GameplayEvent` 必须配置有效 Event Tag，直接效果字段必须为空。
- 冷却 Tag 和冷却时长必须同时有效，不能只填一个。
- 属性值、持续时间、耐久和需求等级不能为非法负数。
- 不允许物品 DataAsset 指向任意 GameplayEffect 蓝图来绕过统一模板。

### 31.14 迁移步骤

按以下顺序改造，避免一次性破坏现有测试资产：

1. 新增属性结构体、道具效果枚举和冷却结构体。
2. 新增四个 C++ 原生 GE 类和 `FDOItemEffectSpecBuilder`；复杂道具首次落地时再创建 `UDOGameplayAbility_UseItem`。
3. 装备组件优先读取新的 `AttributeModifiers`；新字段为空时临时兼容旧 `BaseAttributeMagnitudes`。
4. 消耗品组件优先读取新的 `EffectKind` 配置；旧 `UseGameplayEffect` 标记 Deprecated。
5. 批量迁移现有 ItemDefinition 测试资产。
6. 更新 Tooltip、装备对比和 Data Validation。
7. 删除旧 Map 和普通道具的 `UseGameplayEffect` 兼容分支。
8. 运行背包、装备、消耗品、存档和 Listen Server 测试。

### 31.15 验收标准

- 新建普通装备时只创建一个 ItemDefinition，并能直接填写全部支持属性。
- 新建回复药水或限时 Buff 时不创建 GE 蓝图。
- 所有装备共用 `UDOEquipmentAttributeEffect`，但每件装备拥有独立 Handle 和数值。
- 所有简单消耗品由 C++ Builder 动态创建 Spec，效果成功后才扣除数量。
- 新增属性后，装备和限时道具资产自动出现对应字段。
- 客户端不能提交最终属性值、持续时间或冷却时间。
- SaveGame 只保存物品实例和 DefinitionId，不保存 GameplayEffectSpec/Handle；读档后从 ItemDefinition 重新构建装备 GE。
- 联机复制中不出现瞬态 GE 定义、客户端无法解析 GE 或 ActiveGameplayEffect 丢失问题。
