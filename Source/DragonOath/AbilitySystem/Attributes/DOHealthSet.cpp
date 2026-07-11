// DragonOath health AttributeSet.

#include "DOHealthSet.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDOHealthSet::UDOHealthSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitDamage(0.0f);
	InitHealthRegen(0.0f);
}

void UDOHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 【网络知识点】
	// DOREPLIFETIME_CONDITION_NOTIFY 是GAS属性同步的标准写法
	// COND_None = 同步给所有客户端
	// REPNOTIFY_Always = 即使值没变也触发OnRep（GAS要求这样做）
	//
	// 为什么用 REPNOTIFY_Always？
	// 因为GAS内部依赖OnRep来更新ASC的属性缓存。
	// 如果只在值变化时通知，某些边界情况（如Clamp后值不变）会导致客户端ASC不同步。
	DOREPLIFETIME_CONDITION_NOTIFY(UDOHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
	// 生命回复只影响 Owner 的 Periodic GE 计算，仅同步给 Owner 即可。
	DOREPLIFETIME_CONDITION_NOTIFY(UDOHealthSet, HealthRegen, COND_OwnerOnly, REPNOTIFY_Always);
	// 注意：Damage是Meta属性，不需要同步
}

// ================================================================
// OnRep 回调 —— 通知ASC属性已变化
// ================================================================

void UDOHealthSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	// 这一行是关键！它告诉ASC："Health属性刚从网络同步过来了"
	// ASC会据此触发绑定的 AttributeValueChangeDelegate
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOHealthSet, Health, OldHealth);
}

void UDOHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOHealthSet, MaxHealth, OldMaxHealth);
}

void UDOHealthSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOHealthSet, HealthRegen, OldHealthRegen);
}

void UDOHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}
// ================================================================
// PreAttributeChange —— 客户端 + 服务器都会调用
// ================================================================

void UDOHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 只做Clamp，不做游戏逻辑
	// 【常见错误】在这里处理死亡逻辑 —— 错！因为客户端也会调用，会导致重复执行
	ClampAttribute(Attribute, NewValue);
}
void UDOHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetHealth() > NewValue)
		{
			UDOAbilitySystemComponent* DOASC = GetUDOAbilitySystemComponent();
			check(DOASC);

			DOASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

bool UDOHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

// ================================================================
// PostGameplayEffectExecute —— 只在服务器执行！
// ================================================================

void UDOHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 【核心网络知识点】
	// 这个函数只在服务器上执行！
	// 所以可以安全地在这里做权威性的游戏逻辑判断

	// 从Data中提取有用的上下文信息
	// Data.EffectSpec —— 触发这次变化的GE规格
	// Data.EvaluatedData —— 实际计算结果
	// Data.Target —— 目标ASC

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Damage是Meta属性，读取后转换为Health变化
		const float LocalDamage = GetDamage();
		SetDamage(0.0f); // 立即清零，Meta属性只是中转

		if (LocalDamage > 0.0f)
		{
			const float NewHealth = GetHealth() - LocalDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			// ==================== 吸血（真实伤害驱动，仅服务端）====================
			// 本函数只在服务器执行，天然避免 LocalPredicted 客户端双重回血。
			// 吸血量 = LifeStealRate * LocalDamage（LocalDamage 已含暴击/格挡/减免/倍率），数据正确。
			// 仅当伤害 GE 的 Source Tags 含 Damage.CanLifeSteal 时生效。
			{
				const FGameplayTagContainer* SourceTags = Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
				if (SourceTags && SourceTags->HasTag(DragonOathGameplayTags::Damage::CanLifeSteal))
				{
					if (AActor* SourceActor = Data.EffectSpec.GetEffectContext().GetInstigator())
					{
						if (const IAbilitySystemInterface* SourceASI = Cast<IAbilitySystemInterface>(SourceActor))
						{
							if (UAbilitySystemComponent* SourceASC = SourceASI->GetAbilitySystemComponent())
							{
								if (const UDOCombatSet* SourceCombat = SourceASC->GetSet<UDOCombatSet>())
								{
									const float HealAmount = SourceCombat->GetLifeStealRate() * LocalDamage;
									if (HealAmount > 0.0f)
									{
										// 复用 Healing Meta：由本类的 Healing 分支转换为 Source 的 Health 回复。
										SourceASC->ApplyModToAttribute(GetHealingAttribute(), EGameplayModOp::Additive, HealAmount);
									}
								}
							}
						}
					}
				}
			}

			// ==================== 伤害特效 / 伤害数字显示落点（TODO）====================
			// 此处已同时拿到最终伤害 LocalDamage 与 EffectContext，是播放受击特效与伤害飘字最自然的位置。
			// 实际表现（GameplayCue.GameplayCue.Damage.* 类 Tag 或 Widget 飘字）将在后续阶段实现，可传入的数据：
			//   - 最终伤害 LocalDamage                          -> 伤害数字
			//   - EffectContext->IsCriticalHit()               -> 暴击闪光 / 红字 / 放大
			//   - EffectContext->IsBlockedHit()                -> 格挡火花 / 偏斜（Phase 4 补齐格挡判定后生效）
			//   - EffectContext->GetHitBoneName()              -> 部位受击点（爆头特效）
			//   - EffectContext->GetDamageDirection()          -> 受击击退 / 朝向
			//   - EffectContext->GetDamageElementTag()         -> 元素受击色（火/冰/雷）
			//   - EffectContext->GetDamageMultiplier()         -> 倍率提示（爆头 2x）
			//   - Data.Target.GetAvatarActor()                -> 特效挂点（目标）
			//   - Data.EffectSpec.GetEffectContext().GetOriginalInstigator() -> 来源定位（如有需要）
			// TODO: 在客户端（LocalPredicted 预测或权威）依据上述数据播放受击特效与伤害飘字。

			// 死亡判定 —— 服务器权威
			if (GetHealth() <= 0.0f)
			{
				// 通过GameplayEvent或Tag来通知死亡，而不是直接调用Die()
				// 这样可以让GA_Death来处理死亡流程，更加解耦
				FGameplayEventData EventData;
				EventData.Instigator = Data.EffectSpec.GetEffectContext().GetOriginalInstigator();
				EventData.Target = Data.Target.GetAvatarActor();

				// 发送死亡事件，让监听此事件的GA来处理
				Data.Target.HandleGameplayEvent(
					DragonOathGameplayTags::Event::Death,
					&EventData
				);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// Healing是Meta属性，读取后转换为Health回复（吸血等来源）
		const float LocalHeal = GetHealing();
		SetHealing(0.0f); // 立即清零，Meta属性只是中转

		if (LocalHeal > 0.0f)
		{
			const float NewHealth = GetHealth() + LocalHeal;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}
}

void UDOHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// 生命值只允许落在 0 到 MaxHealth 之间。
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// 最大生命值至少保留 1，避免除零和空血上限。
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetHealthRegenAttribute())
	{
		// 回复速率不允许为负
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
