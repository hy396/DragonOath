// DragonOath health AttributeSet.

#include "DOHealthSet.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDOHealthSet::UDOHealthSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitDamage(0.0f);
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
}
