// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/DOGameplayAbility_Dash.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility_Dash)

UDOGameplayAbility_Dash::UDOGameplayAbility_Dash(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EDOAbilityActivationPolicy::OnInputTriggered;

	// 技能激活时自动添加，结束时自动移除
	ActivationOwnedTags.AddTag(DragonOathGameplayTags::Status::Dashing);

	// if (bInvincibleDuringDash)
	// {
	// 	// ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Combat.Invincible")));
	// 	// ActivationOwnedTags.AddTag(DragonOathGameplayTags::State::Combat::Invincible);
	// }

	// 冲刺中不能再冲刺，由 GAS 默认 CanActivateAbility 自动检查
	ActivationBlockedTags.AddTag(DragonOathGameplayTags::Status::Dashing);
}

bool UDOGameplayAbility_Dash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 父类已处理：Level <= 0 拦截、ActivationBlockedTags 检查、Cost 检查等
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UDOGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 播放冲刺蒙太奇。不等待蒙太奇结束，动画只负责表现，位移由 RootMotion Task 驱动。
	if (DashMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, DashMontage, 1.0f, NAME_None, false);
		MontageTask->OnCancelled.AddDynamic(this, &UDOGameplayAbility_Dash::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}

	const FVector DashDirection = GetDashDirection();

	UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask =
		UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this,
			NAME_None,
			DashDirection,
			DashStrength,
			DashDuration,
			false,
			StrengthOverTimeCurve,
			ERootMotionFinishVelocityMode::ClampVelocity,
			FVector::ZeroVector,
			GetMaxSpeed(),
			false
		);

	RootMotionTask->OnFinish.AddDynamic(this, &UDOGameplayAbility_Dash::OnDashFinished);
	RootMotionTask->ReadyForActivation();
}

void UDOGameplayAbility_Dash::OnDashFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDOGameplayAbility_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 只有正常结束才施加冲刺攻击窗口
	if (!bWasCancelled && DashAttackWindowEffectClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DashAttackWindowEffectClass, GetAbilityLevel(), Context);
			if (Spec.IsValid())
			{
				Spec.Data.Get()->SetDuration(DashAttackWindowDuration, true);
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	// ActivationOwnedTags 会在 Super::EndAbility 中自动移除
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UDOGameplayAbility_Dash::GetDashDirection() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar)
	{
		return Avatar->GetActorForwardVector();
	}
	return FVector::ForwardVector;
}

float UDOGameplayAbility_Dash::GetMaxSpeed() const
{
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		const UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
		if (MoveComp)
		{
			return MoveComp->GetMaxSpeed();
		}
	}
	return 500.0f;
}