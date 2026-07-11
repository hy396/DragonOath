# 伤害结算数据写回 EffectContext 与显示落点方案

> 阶段：第二阶段（进阶属性 + AttributeSet 拆分）补充
> 状态：已审批，执行中

## 1. 背景与问题

`UDOExecutionCalculation_Damage` 已能在服务端计算命中、暴击、格挡并输出最终伤害到 `Damage` Meta 属性。但存在两个待补齐点：

1. **计算结果未传出**：暴击/格挡等判定结果只用于本地算伤害，没有写回 `FDOGameplayEffectContext`。客户端 GameplayCue 无法知道该次伤害是否暴击/格挡，因而无法播放差异化受击特效。
2. **吸血基于估算值（缺陷）**：当前吸血在 `UDOAbilitySystemComponent::ApplyDamageToTarget` 用 `LifeStealRate * (SkillBaseDamage + AttackPower)` 估算回血。该值**未使用 ExecCalc 算出的真实伤害**——未扣防御、未算暴击翻倍、未算格挡减半、未乘部位倍率。即"打出的实际伤害"与"吸回来的血量"脱节。

本方案补齐"计算结果写回自定义 EffectContext → 随 GE 网络同步到客户端 → AttributeSet 扣血处既驱动真实吸血、又作为伤害特效/伤害数字的显示落点"的完整链路。

## 2. 与 Aura 的差异分析

| 维度 | Aura 做法 | DragonOath 决定 |
|---|---|---|
| 写回入口 | `ExecCalc_Damage::Execute` 内 `Spec.GetContext()` 后调用 `UAuraAbilitySystemLibrary::SetIsCriticalHit(...)` | 同，但**不新建 Library 类**，直接 `static_cast<FDOGameplayEffectContext*>` 后调 Set 接口 |
| Library 封装 | `UAuraAbilitySystemLibrary` 提供 `SetIsCriticalHit/SetIsBlockedHit` 等 static 薄封装 | 当前无蓝图侧批量操作 Context 需求，YAGNI，不建类（未来如需蓝图暴露再抽） |
| 自定义字段 | DeBuff / DeathImpulse / Knockback / Radial 等 Aura 专属玩法 | 不导入（DragonOath 暂无对应系统）；保留暴击/格挡/部位/方向/元素/倍率 |
| 吸血 | Aura 在 ExecCalc 内通过 `SetIsSuccessfulDeBuff` 等 + 后续 GE 应用 | 见第 4 节，改为在 `PostGameplayEffectExecute` 用真实伤害计算 |

结论：Aura 的 Library 仅是 `static_cast + Set` 的薄封装，对 DragonOath 当前阶段属过度设计。直接写回语义更清晰、调用链更短。

## 3. 写回 EffectContext（方案 A + X）

### 3.1 写回入口：ExecCalc 内部

伤害计算的随机判定（命中/暴击/格挡/倍率）只在 ExecCalc 服务端一次性完成，结果写回 Context 后随 GE **自动网络复制**到客户端，避免客户端重复计算导致预测不一致。

`FDOGameplayEffectContext` 已具备字段与 `NetSerialize`（位掩码只同步非零字段），写回无需改动序列化。写回代码片段（执行阶段）：

```cpp
FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
if (FDOGameplayEffectContext* DOCtx = static_cast<FDOGameplayEffectContext*>(EffectContextHandle.Get()))
{
    DOCtx->SetIsCriticalHit(bCriticalHit);
    DOCtx->SetIsBlockedHit(bBlocked);
    // 若 ExecCalc 后续扩展部位/方向/元素/倍率，一并写回：
    // DOCtx->SetHitBoneName(...); DOCtx->SetDamageDirection(...);
    // DOCtx->SetDamageElementTag(...); DOCtx->SetDamageMultiplier(...);
}
```

防御性断言（避免 Context 未注册时崩溃）：

```cpp
checkf(EffectContextHandle.Get() &&
       EffectContextHandle.Get()->GetScriptStruct()->IsChildOf(FDOGameplayEffectContext::StaticStruct()),
       TEXT("EffectContext 不是 FDOGameplayEffectContext，请确认 UDOAbilitySystemGlobals 已注册"));
```

插入位置：在暴击/格挡判定结果得出之后（与 `OutExecutionOutput.AddOutputModifier` 独立，前后皆可），建议在判定块末尾集中写回。

### 3.2 显示落点：PostGameplayEffectExecute（方案 X）

`UDOHealthSet::PostGameplayEffectExecute` 的 `Damage` 分支处同时拿到：
- 最终 `LocalDamage`（已结算扣血，含暴击/格挡/减免/倍率）
- `Data.EffectSpec.GetEffectContext()`（含暴击/格挡/部位/方向/元素/倍率全部字段）

是播放受击特效与伤害飘字最自然的位置。当前仅以 TODO 标注，说明可传入数据（见第 5 节 TODO 数据清单），不实现特效表现本身。

## 4. 吸血修正（用户提议，数据正确）

### 4.1 现状问题

`ApplyDamageToTarget` 估算吸血：

```cpp
const float HealAmount = LifeStealRate * (SkillBaseDamage + AttackPower); // 预减免估算
```

与实际打出伤害脱节。

### 4.2 新方案：移到 PostGameplayEffectExecute（服务端权威）

- `PostGameplayEffectExecute` **只在服务器执行**（GAS 机制 + 代码注释保证），天然避免 `LocalPredicted` 客户端双重回血，删除 `HasAuthority()` 判断。
- 此处已拿到真实 `LocalDamage`，吸血量 = `LifeStealRate * LocalDamage`，完全正确（含暴击/格挡/减免/倍率）。
- 通过 `Data.EffectSpec.GetEffectContext()` 取 Source ASC 与 Instigator，查 `Damage.CanLifeSteal` Tag 决定是否吸血。
- 吸血本质是对 Source 的 Healing，复用 `DOHealthSet` 已有 `Healing` Meta 与处理逻辑（给 Source ASC 的 `Healing` Meta 加值即可）。

实现片段（执行阶段）：

```cpp
// 在 Damage 分支、LocalDamage > 0 时：
const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
UAbilitySystemComponent* SourceASC = EffectContext.GetSourceAbilitySystemComponent();
if (SourceASC && EffectContext.GetSourceObject() /* 或 Source Tags */ 含 CanLifeSteal)
{
    if (const UDOCombatSet* SourceCombat = SourceASC->GetSet<UDOCombatSet>())
    {
        const float HealAmount = SourceCombat->GetLifeStealRate() * LocalDamage;
        if (HealAmount > 0.0f)
        {
            SourceASC->ApplyModToAttribute(UDOHealthSet::GetHealingAttribute(),
                EGameplayModOp::Additive, HealAmount);
        }
    }
}
```

> 注：`CanLifeSteal` 配置在伤害 GE 的 Source Tags 上。判断是否吸血以 Source ASC 捕获的 Source Tags 或 `EffectContext` 携带的 Tag 为准（执行阶段确认具体取 Tag 的 API）。

### 4.3 清理

- 删除 `DOAbilitySystemComponent.cpp` 中 407–423 行估算吸血块。
- 更新 `DOAbilitySystemComponent.h` `ApplyDamageToTarget` 注释（移除"估算回血"描述）。
- 更新 `DOExecutionCalculation_Damage.h` 旧注释"吸血不在此处处理……在服务端为来源回血"。

## 5. TODO 数据清单（伤害特效/飘字显示落点）

在 `DOHealthSet::PostGameplayEffectExecute` 的 Damage 分支处，实际特效（GameplayCue `GameplayCue.Damage.*` 类 Tag 或 Widget 飘字）可通过 `EffectContext` 拿到以下数据：

| 数据 | 来源 | 用途 |
|---|---|---|
| 最终伤害 `LocalDamage` | `GetDamage()` | 伤害数字 |
| 是否暴击 `bIsCriticalHit` | `EffectContext->IsCriticalHit()` | 暴击闪光/红字/放大 |
| 是否格挡 `bIsBlockedHit` | `EffectContext->IsBlockedHit()` | 格挡火花/偏斜 |
| 命中骨骼 `HitBoneName` | `EffectContext->GetHitBoneName()` | 部位受击点（爆头特效） |
| 伤害方向 `DamageDirection` | `EffectContext->GetDamageDirection()` | 受击击退/朝向 |
| 元素 Tag `DamageElementTag` | `EffectContext->GetDamageElementTag()` | 元素受击色（火/冰/雷） |
| 部位倍率 `DamageMultiplier` | `EffectContext->GetDamageMultiplier()` | 倍率提示（爆头 2x） |
| 目标 Actor | `Data.Target.GetAvatarActor()` | 特效挂点 |
| 来源 Actor | `EffectContext.GetOriginalInstigator()` | 来源定位（如有需要） |

## 6. 联机验证注意点

- PIE Listen Server + 1 Client 起步。
- 确认暴击/格挡写回后，`NetSerialize` 位掩码正确同步（客户端 GameplayCue 能读到 `IsCriticalHit`）。
- 吸血仅在服务端 `PostGameplayEffectExecute` 触发，客户端不应重复回血（观察 Source HP 不异常跳变）。
- 吸血量应随暴击/格挡/减免变化（对比旧估算值，验证数据正确性）。
- `LocalPredicted` 下客户端预测伤害与权威一致（预测回滚正常，无双重扣血/回血）。

## 7. 修改文件清单

| 文件 | 改动 |
|---|---|
| `Docs/方案/伤害结算写回EffectContext与显示落点.md` | 本方案文档（新增） |
| `Pipeline/DOExecutionCalculation_Damage.cpp` | 判定后写回 `FDOGameplayEffectContext` 字段 |
| `Pipeline/DOExecutionCalculation_Damage.h` | 更新"吸血不在此处"旧注释 |
| `Attributes/DOHealthSet.cpp` | `PostGameplayEffectExecute` 加真实伤害吸血 + 显示落点 TODO |
| `Core/DOAbilitySystemComponent.cpp` | 删除估算吸血块（407–423） |
| `Core/DOAbilitySystemComponent.h` | 更新 `ApplyDamageToTarget` 注释 |
