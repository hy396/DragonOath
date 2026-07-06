#include "AbilitySystem/Abilities/DOGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "DOLogChannels.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility)

UDOGameplayAbility::UDOGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// UE5.8 已经不推荐把 NonInstanced 当默认策略。玩家技能默认每个 ASC 一份实例，状态更清晰。
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 玩家操作类技能优先本地预测，服务端仍会做最终裁决。
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UDOGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return DOCanActivate(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UDOGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (bAutoCommitOnActivate)
	{
		FGameplayTagContainer RelevantTags;
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo, &RelevantTags))
		{
			LogAbilityFailure(TEXT("CommitAbility failed during activation."), ActorInfo);
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	// 先走父类入口，让 GAS 内部状态进入激活态；项目逻辑再放到 DOActivateAbility。
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	DOActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDOGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 项目清理先执行，随后交还 GAS 清理激活状态和复制状态。
	DOEndAbility(bWasCancelled);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDOGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UDOGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	DOInputPressed(Handle, ActorInfo, ActivationInfo);
}

void UDOGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	DOInputReleased(Handle, ActorInfo, ActivationInfo);
}

void UDOGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActivationPolicy == EDOAbilityActivationPolicy::OnSpawn && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
}

void UDOGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}

UDOAbilitySystemComponent* UDOGameplayAbility::GetDOAbilitySystemComponent() const
{
	return Cast<UDOAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

APawn* UDOGameplayAbility::GetDOPawnAvatar() const
{
	return Cast<APawn>(GetAvatarActorFromActorInfo());
}

AController* UDOGameplayAbility::GetDOController() const
{
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (APlayerController* PlayerController = ActorInfo->PlayerController.Get())
		{
			return PlayerController;
		}
	}

	if (const APawn* Pawn = GetDOPawnAvatar())
	{
		return Pawn->GetController();
	}

	return nullptr;
}

void UDOGameplayAbility::ApplyEffectSpecToTarget(FGameplayEffectSpecHandle& SpecHandle, UAbilitySystemComponent* TargetASC)
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !TargetASC || !SpecHandle.IsValid())
	{
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void UDOGameplayAbility::ApplyEffectSpecToHitResult(FGameplayEffectSpecHandle& SpecHandle, const FHitResult& HitResult, UAbilitySystemComponent* TargetASC)
{
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->GetContext().AddHitResult(HitResult, true);

	UAbilitySystemComponent* ResolvedTargetASC = TargetASC;
	if (!ResolvedTargetASC && HitResult.GetActor())
	{
		ResolvedTargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor());
	}

	ApplyEffectSpecToTarget(SpecHandle, ResolvedTargetASC);
}

void UDOGameplayAbility::BroadcastTargetData(const FGameplayAbilityTargetDataHandle& TargetData, bool bShouldReplicate)
{
	UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent();
	if (!DOASC)
	{
		return;
	}

	const FGameplayTag ApplicationTag;
	const FPredictionKey PredictionKey = CurrentActivationInfo.GetActivationPredictionKey();

	// 预测技能在客户端先产生目标数据时，需要同步给服务端验证和执行。
	if (bShouldReplicate && !K2_HasAuthority())
	{
		DOASC->ServerSetReplicatedTargetData(CurrentSpecHandle, PredictionKey, TargetData, ApplicationTag, DOASC->ScopedPredictionKey);
	}

	// 本地也广播一次，等待目标数据的 AbilityTask 可以立即继续。
	DOASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, PredictionKey).Broadcast(TargetData, ApplicationTag);
}

void UDOGameplayAbility::OnPawnAvatarSet()
{
	if (ActivationPolicy == EDOAbilityActivationPolicy::OnSpawn)
	{
		if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
		{
			DOASC->TryActivateAbilityByClass(GetClass());
		}
	}
}

bool UDOGameplayAbility::DOCanActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return true;
}

void UDOGameplayAbility::DOActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
}

void UDOGameplayAbility::DOEndAbility(bool bWasCancelled)
{
}

void UDOGameplayAbility::DOInputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
}

void UDOGameplayAbility::DOInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
}

void UDOGameplayAbility::LogAbilityFailure(const FString& Reason, const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!bLogAbilityFailures)
	{
		return;
	}

	UE_LOG(LogDragonOathAbilitySystem, Verbose, TEXT("Ability %s failed for %s: %s"),
		*GetNameSafe(GetClass()),
		*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr),
		*Reason);
}
