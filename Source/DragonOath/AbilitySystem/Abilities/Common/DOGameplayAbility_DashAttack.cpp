// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Common/DOGameplayAbility_DashAttack.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
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

	// TODO:2026/8/1 改为移除授予窗口标签的 Active GE，修复 RemoveLooseGameplayTag 无法消费 GE Granted Tag 的问题。@Claude
	// 窗口标签由 DashAttackWindowEffectClass 的 Active GE 授予，不能按 Loose Tag 移除。
	FGameplayTagContainer WindowTags;
	WindowTags.AddTag(DragonOathGameplayTags::Status::DashAttackWindow);
	ASC->RemoveActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(WindowTags));

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