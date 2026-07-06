# UE 5.8 GAS 学习与 DragonOath Ability 基类设计

状态：学习笔记 + 当前项目设计依据。

说明：用户提供的 `https://doc.codable.cn/unreal/ue58-gameplay-ability-system/` 当前在本环境无法直接打开，本文先以本机 `D:\UE_5.8` 的 GameplayAbilities 源码为准。后续如果补充文章正文，再把差异追加到本文。

## GAS 核心对象

GAS 不建议把技能、属性、伤害、Buff 都写在 Character 里。推荐把职责拆开：

| 对象 | 负责什么 |
| --- | --- |
| `UAbilitySystemComponent` | 拥有技能、属性、效果、输入状态，是 GAS 的运行核心 |
| `UGameplayAbility` | 一个可激活技能，负责技能流程、任务、预测、消耗、冷却 |
| `FGameplayAbilitySpec` | “某个 ASC 拥有某个 Ability”的运行记录，里面有 Handle、等级、动态标签、实例 |
| `UGameplayEffect` | 修改属性、添加 Buff/Debuff、冷却、消耗 |
| `UAttributeSet` | 保存可复制属性，例如生命、魔法、攻击、防御 |
| `GameplayTag` | 描述状态、技能、输入、事件、Cue，避免硬编码枚举 |
| `GameplayCue` | 客户端表现层，例如命中特效、音效、飘字 |

DragonOath 当前推荐：

```text
PlayerState 拥有 ASC
Character 作为 AvatarActor
PlayerController 在 PostProcessInput 处理能力输入
GameplayAbility 只做技能流程
AttributeSet 只保存和 Clamp 数值
ExecutionCalculation 处理复杂伤害公式
```

## UE 5.8 UGameplayAbility 已经提供的能力

`UGameplayAbility` 自带很多能力，项目基类不要重复包装。

已经有的常用接口：

```text
GetAbilitySystemComponentFromActorInfo()
GetOwningActorFromActorInfo()
GetAvatarActorFromActorInfo()
GetCurrentActorInfo()
GetCurrentActivationInfo()
GetCurrentAbilitySpecHandle()
GetCurrentAbilitySpec()
GetAssetTags()
IsPredictingClient()
IsLocallyControlled()
K2_HasAuthority()
CommitAbility()
K2_CommitAbility()
K2_CommitAbilityCost()
K2_CommitAbilityCooldown()
GetCostGameplayEffect()
GetCooldownGameplayEffect()
ApplyGameplayEffectSpecToOwner()
K2_ApplyGameplayEffectSpecToOwner()
K2_ApplyGameplayEffectSpecToTarget()
K2_EndAbility()
K2_CancelAbility()
```

因此，项目基类不应该重新声明：

```text
IsLocallyControlled()
IsPredictingClient()
CostGameplayEffectClass
CooldownGameplayEffectClass
CommitAbility 的蓝图封装
普通 ApplyGameplayEffectToOwner / Target 封装
```

重复声明这些内容会带来两个问题：

```text
UHT 可能认为你在重复声明父类 UFUNCTION。
UE 5.8 API 更新后，项目基类更容易踩废弃字段或签名变化。
```

## UE 5.8 AbilitySpec 变化

`FGameplayAbilitySpec::DynamicAbilityTags` 在 UE 5.8 中已经标记废弃。

项目代码应该使用：

```cpp
AbilitySpec.GetDynamicSpecSourceTags()
```

不要直接使用：

```cpp
AbilitySpec.DynamicAbilityTags
```

`FGameplayAbilitySpec::ActivationInfo` 也已标记废弃。它只适用于 NonInstanced Ability，而 NonInstanced Ability 本身也不推荐继续扩展。

Instanced Ability 应该从 Ability 实例读取：

```cpp
AbilityInstance->GetCurrentActivationInfo()
```

DragonOath 的项目技能默认应该使用：

```text
InstancedPerActor
LocalPredicted
```

这适合玩家主动技能、输入预测、WaitInputPress/WaitInputRelease、TargetData 等流程。

## DragonOath 的 Ability 基类职责

`UDOGameplayAbility` 只保留项目真正需要的内容：

```text
1. 项目默认策略
   - InstancedPerActor
   - LocalPredicted
   - 默认自动 Commit，可按技能关闭

2. 输入激活策略
   - OnInputTriggered
   - WhileInputActive
   - OnSpawn

3. Typed Getter
   - GetDOAbilitySystemComponent()
   - GetDOPawnAvatar()
   - GetDOController()

4. 项目扩展点
   - DOCanActivate()
   - DOActivateAbility()
   - DOEndAbility()
   - DOInputPressed()
   - DOInputReleased()

5. 少量项目辅助
   - ApplyEffectSpecToTarget()
   - ApplyEffectSpecToHitResult()
   - BroadcastTargetData()
   - LogAbilityFailure()
```

不再保留：

```text
IsServerSide()
IsPredictingClient()
CommitAbilityChecksOrFail()
CommitAbilityCostOrFail()
CommitAbilityCooldownOrFail()
未接入系统的 ActivationGroup
未接入系统的 CameraMode
空的 Montage 回调
重复 Cost / Cooldown GameplayEffectClass
```

## 激活策略

### OnInputTriggered

按下输入时触发一次。

适合：

```text
普攻
闪避
瞬发技能
普通释放技能
```

### WhileInputActive

按键保持按下时尝试激活。

适合：

```text
格挡
持续瞄准
蓄力开始
引导技能
```

### OnSpawn

技能被授予或 Avatar 切换完成后自动激活。

适合：

```text
被动监听技能
死亡监听技能
常驻属性修正技能
出生时自动挂载的状态技能
```

## 输入链路

DragonOath 采用 Lyra 风格输入链路：

```text
InputAction
-> ULyraInputComponent
-> GameplayTag
-> UDOAbilitySystemComponent::AbilityInputTagPressed / Released
-> 缓存 AbilitySpecHandle
-> ADOPlayerController::PostProcessInput
-> UDOAbilitySystemComponent::ProcessAbilityInput
-> TryActivateAbility
-> UDOGameplayAbility::ActivateAbility
```

这样设计的好处：

```text
输入层不知道具体技能类。
技能只关心自己的 ActivationPolicy。
ASC 统一处理 Pressed / Held / Released 顺序。
```

## UDOGameplayAbility 设计原则

### 不重复父类能力

如果 `UGameplayAbility` 已经有稳定接口，优先直接使用父类。

例如：

```text
蓝图判断本地控制：直接用父类 IsLocallyControlled
蓝图提交消耗冷却：直接用父类 CommitAbility / CommitAbilityCost / CommitAbilityCooldown
C++ 判断预测端：直接用父类 IsPredictingClient
```

### 项目基类只写项目差异

`UDOGameplayAbility` 应该回答 DragonOath 自己的问题：

```text
这个技能如何被输入触发？
这个技能是否自动 Commit？
这个技能怎么拿到 UDOAbilitySystemComponent？
这个技能如何把 TargetData 发给服务器？
这个技能失败时怎么按项目日志输出？
```

### Blueprint 和 C++ 分层

项目基类可以暴露给蓝图：

```text
GetDOAbilitySystemComponent
GetDOPawnAvatar
GetDOController
GetActivationPolicy
ApplyEffectSpecToTarget
ApplyEffectSpecToHitResult
BroadcastTargetData
```

不应该暴露给蓝图：

```text
FGameplayAbilityActorInfo*
FGameplayTagContainer*
FGameplayAbilityActivationInfo
```

这些是 GAS 内部 C++ 流程参数，UHT 不允许或不适合暴露。

## 后续扩展

等基础技能能稳定编译、激活、消耗、冷却、命中后，再加：

```text
ActivationGroup
技能互斥和取消规则
Montage 统一回调
CameraMode
AbilityTagRelationshipMapping
```

不要在第一版基类里提前塞空实现。空实现越多，后续越难判断哪些是真功能，哪些只是复制残留。
