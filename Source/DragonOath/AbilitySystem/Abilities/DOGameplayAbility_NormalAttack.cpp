// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/DOGameplayAbility_NormalAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Core/DOGameplayTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility_NormalAttack)

UDOGameplayAbility_NormalAttack::UDOGameplayAbility_NormalAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 普攻是瞬发技能，按下即触发
	ActivationPolicy = EDOAbilityActivationPolicy::OnInputTriggered;

	// 冲刺攻击窗口期间被阻止，让冲刺攻击优先
	ActivationBlockedTags.AddTag(DragonOathGameplayTags::Status::DashAttackWindow);
}

void UDOGameplayAbility_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 播放普攻动画
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AttackMontage, 1.0f, NAME_None, false);

		MontageTask->OnCompleted.AddDynamic(this, &UDOGameplayAbility_NormalAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UDOGameplayAbility_NormalAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 没有动画直接结束
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UDOGameplayAbility_NormalAttack::OnMontageCompleted()
{
	// 动画正常播放完成，结束技能
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDOGameplayAbility_NormalAttack::OnMontageCancelled()
{
	// 动画被取消（被击、死亡等），结束技能并标记为取消
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}