// DragonOath health AttributeSet.

#include "DOHealthSet.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
// 本文件是 DragonOath「数值结算」权威点：仅做 Damage→Health / Healing→Health 转换与吸血。
// 死亡判定 / 状态机 / Tag 应用 / FDOVerbMessage 广播 全部迁出到
// Source/DragonOath/Components/DOHealthComponent.cpp（UDOHealthComponent）。
// Lyra 等价实现见 Source/LyraGame/AbilitySystem/Attributes/LyraHealthSet.cpp
//   + Source/LyraGame/Character/LyraHealthComponent.cpp。

UDOHealthSet::UDOHealthSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitDamage(0.0f);
	// TODO:2026/8/1 初始化 Healing Meta，确保吸血等治疗结算从确定的零值开始。@Claude
	InitHealing(0.0f);
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
		// TODO:2026/8/1 直接属性变更也派发生命/上限事件；Damage/Healing Meta 的完整上下文事件在结算阶段单独派发。@Claude
		// 最大生命变化时先保证当前生命不超过新上限；Health 的变化会由下面的独立回调广播。
		if (GetHealth() > NewValue)
		{
			UDOAbilitySystemComponent* DOASC = GetUDOAbilitySystemComponent();
			check(DOASC);

			DOASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}

		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
		}
	}
	else if (Attribute == GetHealthAttribute() && !bSuppressHealthChangedBroadcast && !FMath::IsNearlyEqual(OldValue, NewValue))
	{
		// 普通数值变化（治疗、回复、复制与直接属性修改）只通知血量变化。
		// Damage Meta 会在 PostGameplayEffectExecute 中额外发出 OnDamageApplied，不在此处猜测来源。
		OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
	}

	if (bOutOfHealth && GetHealth() > 0.0f)
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
		// TODO:2026/8/1 Damage/Healing Meta 落地改为只广播一次完整上下文，并单独区分最终伤害事件。@Claude
		// Damage是Meta属性，读取后转换为Health变化
		const float LocalDamage = GetDamage();
		SetDamage(0.0f); // 立即清零，Meta属性只是中转

		if (LocalDamage > 0.0f)
		{
			const float OldHealthValue = GetHealth();
			const float NewHealthValue = FMath::Clamp(OldHealthValue - LocalDamage, 0.0f, GetMaxHealth());
			const float AppliedDamage = OldHealthValue - NewHealthValue;

			bSuppressHealthChangedBroadcast = true;
			SetHealth(NewHealthValue);
			bSuppressHealthChangedBroadcast = false;

			if (AppliedDamage <= 0.0f)
			{
				return;
			}

			// Damage Meta 的最终落点需要携带完整 Spec；普通属性变更只在 PostAttributeChange 广播。
			OnHealthChanged.Broadcast(
				Data.EffectSpec.GetEffectContext().GetOriginalInstigator(),
				Data.EffectSpec.GetEffectContext().GetEffectCauser(),
				&Data.EffectSpec,
				-AppliedDamage,
				OldHealthValue,
				NewHealthValue
			);
			OnDamageApplied.Broadcast(
				Data.EffectSpec.GetEffectContext().GetOriginalInstigator(),
				Data.EffectSpec.GetEffectContext().GetEffectCauser(),
				&Data.EffectSpec,
				AppliedDamage,
				OldHealthValue,
				NewHealthValue
			);

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

			// ==================== 死亡流程触发（权威）====================
			// HealthSet 的职责到「把 Damage Meta 落到 Health + 吸血」为止。
			// 死亡判定 / 事件广播 / Tag 应用全部交给 UDOHealthComponent。
			// 这里只需在血量触底时触发 OnOutOfHealth 委托（带 bOutOfHealth 幂等保护），
			// UDOHealthComponent 在 InitializeWithAbilitySystem 时挂这个委托，回调里会：
			//   1) 触发 Event::Death 给 GAS 内部（GA_Death 等）
			//   2) 权威广播 FDOVerbMessage「击杀事件」(Message.Combat.Elimination.Fired)
			//   3) 推进 EDODeathState 状态机
			//   4) 应用 Status_Death_Dying Tag 到 ASC
			if (GetHealth() <= 0.0f && !bOutOfHealth)
			{
				bOutOfHealth = true;

					// 六参 FDOAttributeEvent：Instigator, Causer, Spec, Magnitude, OldValue, NewValue。
					// 使用实际扣血后的数值，避免伤害超过剩余生命时把过量伤害作为 OldValue。
					OnOutOfHealth.Broadcast(
						Data.EffectSpec.GetEffectContext().GetOriginalInstigator(),
						Data.EffectSpec.GetEffectContext().GetEffectCauser(),
						&Data.EffectSpec,
						AppliedDamage,
						OldHealthValue,
						NewHealthValue
					);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// TODO:2026/8/1 Healing Meta 落地后仅触发生命变化，禁止误报 Message.Combat.DamageApplied。@Claude
		// Healing 是 Meta 属性，读取后转换为 Health 回复（吸血等来源）。
		const float LocalHeal = GetHealing();
		SetHealing(0.0f);

		if (LocalHeal > 0.0f)
		{
			const float OldHealthValue = GetHealth();
			const float NewHealthValue = FMath::Clamp(OldHealthValue + LocalHeal, 0.0f, GetMaxHealth());
			const float AppliedHealing = NewHealthValue - OldHealthValue;

			bSuppressHealthChangedBroadcast = true;
			SetHealth(NewHealthValue);
			bSuppressHealthChangedBroadcast = false;

			if (AppliedHealing > 0.0f)
			{
				OnHealthChanged.Broadcast(
					Data.EffectSpec.GetEffectContext().GetOriginalInstigator(),
					Data.EffectSpec.GetEffectContext().GetEffectCauser(),
					&Data.EffectSpec,
					AppliedHealing,
					OldHealthValue,
					NewHealthValue
				);
			}
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
