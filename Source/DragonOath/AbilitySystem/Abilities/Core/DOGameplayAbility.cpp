// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Core/DOGameplayAbility.h"

#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility)

UDOGameplayAbility::UDOGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// UE5.8 已不推荐 NonInstanced 作为默认策略。玩家技能默认每个 ASC 一份实例，状态更清晰。
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 玩家操作类技能优先本地预测，服务端仍会做最终裁决。
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

float UDOGameplayAbility::GetAttackSpeed() const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UDOCombatSet* CombatSet = ASC->GetSet<UDOCombatSet>())
		{
			return CombatSet->GetAttackSpeed();
		}
	}
	return 1.0f; // 保底
}

bool UDOGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Level <= 0 表示技能尚未学习，统一拦截所有激活路径（输入、事件、被动自动激活）。
	if (GetAbilityLevel(Handle, ActorInfo) <= 0)
	{
		return false;
	}

	return true;
}

void UDOGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// OnSpawn 策略在授予时尝试激活。Level <= 0 的技能会被 CanActivateAbility 拦截。
	if (ActivationPolicy == EDOAbilityActivationPolicy::OnSpawn && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
}

void UDOGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// Avatar 切换后（如玩家 Possess 新 Pawn），已授予的 OnSpawn 技能需要重新尝试激活。
	// GAS 的 InitAbilityActorInfo 会调用此函数，不需要 ASC 手动遍历。
	if (ActivationPolicy == EDOAbilityActivationPolicy::OnSpawn && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
}
