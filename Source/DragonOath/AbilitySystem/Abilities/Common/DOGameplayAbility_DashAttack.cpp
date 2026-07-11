// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Common/DOGameplayAbility_DashAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Core/DOGameplayTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility_DashAttack)

UDOGameplayAbility_DashAttack::UDOGameplayAbility_DashAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 冲刺攻击是瞬发技能，按下即触发
	ActivationPolicy = EDOAbilityActivationPolicy::OnInputTriggered;

	// 只有冲刺攻击窗口期间才能激活，由 GAS 自动检查
	ActivationRequiredTags.AddTag(DragonOathGameplayTags::Status::DashAttackWindow);
}

void UDOGameplayAbility_DashAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 消耗冲刺攻击窗口（一次性，避免重复触发）
	ASC->RemoveLooseGameplayTag(DragonOathGameplayTags::Status::DashAttackWindow);

	// 播放冲刺攻击动画
	if (DashAttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, DashAttackMontage, 1.0f, NAME_None, false);

		MontageTask->OnCompleted.AddDynamic(this, &UDOGameplayAbility_DashAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UDOGameplayAbility_DashAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 没有动画直接结束
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UDOGameplayAbility_DashAttack::OnMontageCompleted()
{
	// 动画正常播放完成，结束技能
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDOGameplayAbility_DashAttack::OnMontageCancelled()
{
	// 动画被取消（被击、死亡等），结束技能并标记为取消
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}