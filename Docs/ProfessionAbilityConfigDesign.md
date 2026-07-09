# 职业技能配置方案

## 目标

职业创建或切换时，一次性把该职业的所有技能授予给 ASC。

技能是否可用不由"有没有授予"决定，而由技能等级决定：

- 初始基础技能给 `1` 级
- 需要学习的技能给 `0` 级
- 被动技能同样支持 `0` 级和 `1` 级以上
- UI 技能树可以直接读取职业配置和当前等级，显示完整技能列表

这样做的好处是技能树、存档、升级、洗点都围绕"技能等级"工作，不需要频繁 Give / Clear 技能。

## 核心规则

所有职业技能在初始化时都进入 ASC。

`Level = 0` 表示已经存在于职业技能表，但角色尚未学习，不能激活。

`Level >= 1` 表示已经学习，可以按触发方式激活。

`UDOGameplayAbility::CanActivateAbility` 统一拦截 `Level <= 0` 的技能，避免输入、事件、被动自动激活绕过学习状态。**已实现**：

```cpp
bool UDOGameplayAbility::CanActivateAbility(...) const
{
    if (!Super::CanActivateAbility(...))
    {
        return false;
    }

    if (GetAbilityLevel(Handle, ActorInfo) <= 0)
    {
        return false;
    }

    return true;
}
```

此外，`UDOAbilitySystemComponent::AbilityInputTagPressed` 也会在输入匹配阶段提前过滤 `Level <= 0` 的技能，避免不必要的 SpecHandle 缓存和激活尝试。**已实现**。

被动技能从 `0` 升到 `1` 时，需要主动激活或应用对应被动效果。

被动技能从 `1` 降回 `0` 时，需要取消技能并移除它产生的持续效果。

## 职业标识

**确认**：职业使用 GameplayTag，不使用枚举。

GameplayTag 支持层级查询（`HasTag` 匹配父标签），转职、隐藏职业、职业分类都能自然表达。枚举在新增职业时需要改代码并重新编译，GameplayTag 只需改 ini。

**已实现**：`DOGameplayTag.h` 中已增加 `Profession` 命名空间。

示例：

```cpp
DragonOathGameplayTags::Profession::DragonFighter
DragonOathGameplayTags::Profession::Mage
DragonOathGameplayTags::Profession::Archer
```

## 技能配置资产

使用 `UPrimaryDataAsset`，不使用纯 DataTable。

原因是技能配置里会放：

- 技能蓝图类
- 输入 GameplayTag
- 事件 GameplayTag
- 初始等级
- 最大等级
- UI 标识
- 被动/主动触发规则

这些字段用 DataAsset 在编辑器里更自然，也更接近 Lyra 的 AbilitySet 思路。

## 技能授予项

每个技能授予项表示"这个职业拥有这个技能，以及它的初始状态"。

建议结构：

```cpp
USTRUCT(BlueprintType)
struct FDOAbilityGrant
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDOGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InitialLevel = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EDOAbilityGrantTriggerType TriggerType = EDOAbilityGrantTriggerType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag EventTag;
};
```

`AbilityId` 用来做 UI、存档、升级查找，不建议直接用技能类名当唯一标识。

示例：

```cpp
Ability.DragonFighter.NormalAttack
Ability.DragonFighter.Dodge
Ability.DragonFighter.FlameSlash
Ability.DragonFighter.DragonBloodPassive
```

> **变更说明**：移除了原方案中的 `bActivateWhenGranted` 字段。该字段与 `TriggerType::OnGranted` 语义重复，授予后自动激活的行为统一由 `TriggerType` 决定。

## 触发方式与激活策略

### TriggerType vs ActivationPolicy

项目中存在两个相关枚举，职责不同，不要混淆：

- `EDOAbilityGrantTriggerType`：配置在 `FDOAbilityGrant` 中，负责**授予时的绑定方式**（绑输入、绑事件、自动激活、不绑定）。
- `EDOAbilityActivationPolicy`：配置在 `UDOGameplayAbility` 蓝图中，负责**激活后的输入行为策略**（点按 vs 持续按住 vs 授予时自动）。

| TriggerType | 含义 | 对应 ActivationPolicy 配合 |
|---|---|---|
| `Input` | 玩家按键触发 | Ability 蓝图配 `OnInputTriggered` 或 `WhileInputActive` |
| `GameplayEvent` | 事件触发 | Ability 蓝图配 `AbilityTriggers` |
| `OnGranted` | 授予后自动激活 | Ability 蓝图配 `OnSpawn` |
| `None` | 不自动绑定 | 通常用于纯 UI 展示或外部手动激活 |

### TriggerType 枚举

```cpp
UENUM(BlueprintType)
enum class EDOAbilityGrantTriggerType : uint8
{
	None,
	Input,
	GameplayEvent,
	OnGranted
};
```

`Input` 表示玩家按键触发。

配置示例：

```cpp
TriggerType = Input
InputTag = InputTag.Ability.Skill1
```

`GameplayEvent` 表示事件触发。

配置示例：

```cpp
TriggerType = GameplayEvent
EventTag = Event.Death
```

事件触发技能同时使用 GAS 原生 `AbilityTriggers`，由外部调用：

```cpp
ASC->HandleGameplayEvent(EventTag, &EventData);
```

`OnGranted` 表示授予后自动激活，适合监听类被动技能。映射到 `UDOGameplayAbility` 的 `EDOAbilityActivationPolicy::OnSpawn`。

如果技能初始等级是 `0`，即使是 `OnGranted` 也不应该激活。**已实现**：`CanActivateAbility` 会拦截 `Level <= 0` 的技能，`OnGiveAbility` 和 `OnAvatarSet` 中的自动激活也会被该检查阻止。

### EventTag 双写约定

**确认**：`EventTag` 两边都写。

- AbilitySet 中的 `EventTag` 用于 UI 展示和数据校验。
- Ability 蓝图的 `AbilityTriggers` 用于 GAS 运行时事件匹配。

校验规则中应检查两边一致性。

## 职业技能组

每个职业对应一个技能组。

```cpp
UCLASS(BlueprintType)
class UDOAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDOAbilityGrant> GrantedAbilities;
};
```

龙斗士示例：

```text
NormalAttack
  InitialLevel = 1
  TriggerType = Input
  InputTag = InputTag.Ability.Primary

Dodge
  InitialLevel = 1
  TriggerType = Input
  InputTag = InputTag.Ability.Dodge

FlameSlash
  InitialLevel = 0
  TriggerType = Input
  InputTag = InputTag.Ability.Skill1

DragonBloodPassive
  InitialLevel = 0
  TriggerType = OnGranted

DeathAbility
  InitialLevel = 1
  TriggerType = GameplayEvent
  EventTag = Event.Death
```

## 职业到技能组的映射

做一个职业总配置。

```cpp
UCLASS(BlueprintType)
class UDOProfessionAbilityConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, TObjectPtr<UDOAbilitySet>> ProfessionAbilitySets;
};
```

玩家角色或 PlayerState 保存当前职业 Tag。

初始化时根据职业 Tag 找到 `UDOAbilitySet`，一次性授予全部技能。

### 多层 AbilitySet 叠加

实际游戏中可能需要基础通用技能、职业专属技能、装备附加技能等多层叠加。建议 `GiveDOAbilitySet` 支持多次调用并自动按 `AbilityId` 去重。后续如果需要分层配置，可将 `TMap<FGameplayTag, TObjectPtr<UDOAbilitySet>>` 扩展为 `TMap<FGameplayTag, TArray<TObjectPtr<UDOAbilitySet>>>`，或在外部按顺序调用多次 `GiveDOAbilitySet`。

## ASC 封装接口

`UDOAbilitySystemComponent` 需要提供项目层封装，避免到处手写 `FGameplayAbilitySpec`。

建议接口：

```cpp
FGameplayAbilitySpecHandle GiveDOAbility(const FDOAbilityGrant& Grant);

void GiveDOAbilitySet(const UDOAbilitySet* AbilitySet);

bool SetDOAbilityLevel(FGameplayTag AbilityId, int32 NewLevel);

int32 GetDOAbilityLevel(FGameplayTag AbilityId) const;

void CancelAbilityById(FGameplayTag AbilityId);
```

授予时需要把输入 Tag 放到 Spec 的动态 Source Tags。

```cpp
FGameplayAbilitySpec Spec(Grant.AbilityClass, Grant.InitialLevel);

if (Grant.InputTag.IsValid())
{
	Spec.GetDynamicSpecSourceTags().AddTag(Grant.InputTag);
}

GiveAbility(Spec);
```

### AbilityId 到 SpecHandle 的映射

`AbilityId -> SpecHandle` 的查询关系直接作为 `UDOAbilitySystemComponent` 的成员，不使用独立结构体：

```cpp
// UDOAbilitySystemComponent.h
TMap<FGameplayTag, FGameplayAbilitySpecHandle> AbilityIdToSpecHandle;
```

这样 ASC 自己管理映射生命周期，切职业时可以一键 Clear。

> **变更说明**：原方案中 `FDOGrantedAbilityHandles` 作为独立 USTRUCT 存在，但实际上它只有 ASC 会使用，且没有跨系统共享需求。直接作为 ASC 成员更简洁。

## 玩家授予流程

玩家 ASC 由 `ADOPlayerState` 持有。

`ADOPlayerCharacter` 初始化 ASC 后，只负责把 PlayerState ASC 注册为当前 Avatar。

真正的职业技能授予建议放在 PlayerState 或一个专门的职业组件里。

流程：

```text
玩家登录或创建角色
PlayerState 拿到职业 Tag
PlayerState 找到职业 AbilitySet
服务端 GiveDOAbilitySet
所有技能进入 ASC
0级技能保留但不可激活
1级技能可输入或事件触发
1级被动技能自动激活（OnSpawn 策略 + CanActivateAbility 放行）
```

需要防止重复授予。

PlayerState 可以记录：

```cpp
bool bProfessionAbilitiesGranted = false;
```

或者记录已授予的 `AbilitySet` 和 `SpecHandle`。

## 怪物授予流程

怪物通常不需要 PlayerState。

怪物可以在自己的角色类或蓝图上挂 `UDOAbilitySystemComponent`。

怪物初始化时，根据怪物配置授予自己的技能组。

流程：

```text
怪物 Spawn
怪物自身 ASC 初始化
服务端 GiveDOAbilitySet
根据 AI / StateTree / GameplayEvent 激活技能
```

**确认**：怪物复用 `UDOAbilitySet` 和 `FDOAbilityGrant` 结构，但走单独的怪物模板配置资产（如 `UDOMonsterAbilityConfig`），不经过 `UDOProfessionAbilityConfig`。授予流程完全相同，只是调用方不同。

## 技能升级

升级不重新授予技能。

升级时找到已有 `FGameplayAbilitySpec`，使用 GAS 公开 API 修改等级：

```cpp
bool UDOAbilitySystemComponent::SetDOAbilityLevel(FGameplayTag AbilityId, int32 NewLevel)
{
    // 必须在服务端执行，客户端不能直接修改 Level
    if (!HasAuthority(...))
    {
        return false;
    }

    const FGameplayAbilitySpecHandle* Handle = AbilityIdToSpecHandle.Find(AbilityId);
    if (!Handle)
    {
        return false;
    }

    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(*Handle);
    if (!Spec)
    {
        return false;
    }

    Spec->Level = NewLevel;
    MarkAbilitySpecDirty(*Spec);
    return true;
}
```

> **变更说明**：原方案直接写 `Spec.Level = NewLevel; MarkAbilitySpecDirty(Spec);`，但 `MarkAbilitySpecDirty` 是 `UAbilitySystemComponent` 的 protected 成员。正确做法是在 `UDOAbilitySystemComponent`（继承自 `UAbilitySystemComponent`）的实现中调用，因为子类可以访问 protected 成员。同时增加服务端权限检查。

从 `0` 升到 `1` 时：

- 输入技能开始允许被输入触发（`AbilityInputTagPressed` 的 Level 过滤会放行）
- 事件技能开始允许被事件触发
- 被动技能需要自动激活或应用被动效果

从 `1` 降到 `0` 时：

- 如果技能正在激活，需要取消
- 如果技能产生了持续 GameplayEffect，需要移除
- UI 显示为未学习

### 被动技能 1 -> 0 的清理机制

被动技能降级时的清理需要区分被动类型：

- **GA 型被动**：在 `EndAbility` / `DOEndAbility` 中主动移除该技能激活时应用的 GE。Ability 内部需要记录 `FActiveGameplayEffectHandle`，结束时通过 `RemoveActiveGameplayEffect` 移除。调用 `CancelAbilityById` 触发此路径。
- **GE 型被动**（不走 GA，直接应用 GE）：需要在 ASC 层维护一个 `AbilityId -> TArray<FActiveGameplayEffectHandle>` 的映射，降级时批量移除。

## 被动技能

**确认**：简单属性型和复杂监听型两种做法并存。

简单属性型被动直接应用 GameplayEffect。

例如：

```text
最大生命 +100
攻击力 +10%
防御力 +20
```

复杂监听型被动用 GameplayAbility。

例如：

```text
受到攻击时反击
血量低于 30% 时触发护盾
暴击后刷新冷却
死亡时触发复活
```

复杂被动使用 `OnGranted` 触发类型（映射到 `OnSpawn` 激活策略）或事件触发。

等级为 `0` 的被动不激活（`CanActivateAbility` 拦截）。

等级大于 `0` 的被动在授予后（`OnGiveAbility`）或 Avatar 设置时（`OnAvatarSet`）自动激活。**已实现**。

## UI 和存档

UI 不应该只从 ASC 已激活技能里找技能。

UI 应该读取职业 `UDOAbilitySet`，展示完整技能树。

然后再从 ASC 查询每个 `AbilityId` 的当前等级。

存档建议保存：

```text
ProfessionTag
AbilityId -> Level
```

不要保存 `FGameplayAbilitySpecHandle`。

`SpecHandle` 是运行时句柄，不适合存档。

加载存档时：

```text
先按职业授予完整 AbilitySet
再根据存档覆盖每个技能等级
最后激活已学习的被动技能
```

## 数据校验

后续可以给 `UDOAbilitySet` 做编辑器校验。

校验规则：

- `AbilityClass` 不能为空
- `AbilityClass` 必须继承 `UDOGameplayAbility`
- `AbilityId` 不能为空
- 同一个 AbilitySet 里 `AbilityId` 不能重复
- `InitialLevel` 不能小于 `0`
- `MaxLevel` 不能小于 `1`
- `InitialLevel` 不能大于 `MaxLevel`
- `TriggerType = Input` 时必须填写 `InputTag`
- `TriggerType = GameplayEvent` 时必须填写 `EventTag`，且 Ability 蓝图的 `AbilityTriggers` 中应包含相同 EventTag
- `TriggerType = None` 时不应该填写输入或事件 Tag

## 技能树与 FDOAbilityGrant 的边界

`FDOAbilityGrant` 只负责"职业有哪些技能以及初始状态"。

技能树的前置条件、消耗、角色等级限制放在 `SkillTreeComponent` 层，不放进 `FDOAbilityGrant`。

两者是正交的：`FDOAbilityGrant` 是静态配置（职业定义），`SkillTreeComponent` 是动态状态（玩家进度）。

## 推荐实施顺序

先实现最小闭环。

- 新建 `DOAbilitySet`
- 新建 `FDOAbilityGrant`
- 新建 `EDOAbilityGrantTriggerType`
- 给 `UDOAbilitySystemComponent` 增加 `GiveDOAbilitySet`
- ~~在 `UDOGameplayAbility::CanActivateAbility` 拦截 `0` 级技能~~ **已完成**
- 做一个龙斗士 `AbilitySet` 资产
- 玩家初始化时服务端授予这个 `AbilitySet`

然后补技能升级。

- 增加 `AbilityId -> SpecHandle` 映射（作为 ASC 成员）
- 增加 `SetDOAbilityLevel`（含服务端权限检查）
- 增加 `GetDOAbilityLevel`
- 处理被动技能 `0 -> 1` 自动激活
- 处理被动技能 `1 -> 0` 取消和清理

最后补职业和存档。

- ~~增加 `Profession` GameplayTag~~ **已完成**
- 增加 `UDOProfessionAbilityConfig`
- PlayerState 保存当前职业
- 存档保存 `AbilityId -> Level`
- UI 从 AbilitySet 读完整技能树
