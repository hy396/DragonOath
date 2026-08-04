// Copyright DragonOath. All Rights Reserved.

#include "AbilitySystem/BlueprintLibrary/DOAbilitySystemBlueprintLibrary.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOAbilitySystemBlueprintLibrary)

FString UDOAbilitySystemBlueprintLibrary::Conv_ActiveGameplayEffectHandleToString(const FActiveGameplayEffectHandle& Handle)
{
	return Handle.ToString();
}

bool UDOAbilitySystemBlueprintLibrary::EqualEqual_GameplayAbilitySpecHandle(const FGameplayAbilitySpecHandle& A, const FGameplayAbilitySpecHandle& B)
{
	return A == B;
}

FString UDOAbilitySystemBlueprintLibrary::Conv_GameplayAbilitySpecHandleToString(const FGameplayAbilitySpecHandle& Handle)
{
	return Handle.ToString();
}

FGameplayAbilitySpecHandle UDOAbilitySystemBlueprintLibrary::GiveAbility(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass, const int32 Level, const int32 InputID)
{
	if (!AbilitySystemComponent || !AbilityClass || !AbilitySystemComponent->IsOwnerActorAuthoritative() || Level <= 0)
	{
		return FGameplayAbilitySpecHandle();
	}

	return AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, Level, InputID));
}

bool UDOAbilitySystemBlueprintLibrary::TryActivateAbilityByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& AbilityToActivate, const bool bAllowRemoteActivation)
{
	if (!AbilitySystemComponent || !AbilityToActivate.IsValid())
	{
		return false;
	}

	return AbilitySystemComponent->TryActivateAbility(AbilityToActivate, bAllowRemoteActivation);
}

void UDOAbilitySystemBlueprintLibrary::ClearAbility(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle, const bool bRemoveAbilityOnEnd)
{
	if (!AbilitySystemComponent || !Handle.IsValid() || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}

	if (bRemoveAbilityOnEnd)
	{
		AbilitySystemComponent->SetRemoveAbilityOnEnd(Handle);
	}
	else
	{
		AbilitySystemComponent->ClearAbility(Handle);
	}
}

int32 UDOAbilitySystemBlueprintLibrary::ModifyGameplayEffectRemainingTimeByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayEffect> EffectClass, const float ModifiedIncrement)
{
	if (!AbilitySystemComponent || !EffectClass || !FMath::IsFinite(ModifiedIncrement) || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return 0;
	}

	FGameplayEffectQuery Query;
	Query.EffectDefinition = EffectClass;

	int32 ModifiedCount = 0;
	for (const FActiveGameplayEffectHandle& Handle : AbilitySystemComponent->GetActiveEffects(Query))
	{
		if (ModifyGameplayEffectRemainingTimeByHandle(AbilitySystemComponent, Handle, ModifiedIncrement))
		{
			++ModifiedCount;
		}
	}

	return ModifiedCount;
}

bool UDOAbilitySystemBlueprintLibrary::ModifyGameplayEffectRemainingTimeByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle, const float ModifiedIncrement)
{
	if (!AbilitySystemComponent || !Handle.IsValid() || !FMath::IsFinite(ModifiedIncrement) || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return false;
	}

	if (!AbilitySystemComponent->GetActiveGameplayEffect(Handle))
	{
		return false;
	}

	// GAS 会在开始时间调整后检查持续时间；减少量超过剩余时间时，效果会正常结束并移除。
	AbilitySystemComponent->ModifyActiveEffectStartTime(Handle, ModifiedIncrement);
	return true;
}

int32 UDOAbilitySystemBlueprintLibrary::ModifyGameplayEffectRemainingTimeByTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& Tags, const float ModifiedIncrement, const EDOTagsQueryCondition QueryCondition)
{
	if (!AbilitySystemComponent || Tags.Num() == 0 || !FMath::IsFinite(ModifiedIncrement) || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return 0;
	}

	const FGameplayEffectQuery Query = QueryCondition == EDOTagsQueryCondition::MatchAll
		? FGameplayEffectQuery::MakeQuery_MatchAllEffectTags(Tags)
		: FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(Tags);

	int32 ModifiedCount = 0;
	for (const FActiveGameplayEffectHandle& Handle : AbilitySystemComponent->GetActiveEffects(Query))
	{
		if (ModifyGameplayEffectRemainingTimeByHandle(AbilitySystemComponent, Handle, ModifiedIncrement))
		{
			++ModifiedCount;
		}
	}

	return ModifiedCount;
}

float UDOAbilitySystemBlueprintLibrary::GetGameplayEffectDuration(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle)
{
	if (!AbilitySystemComponent || !Handle.IsValid())
	{
		return 0.0f;
	}

	return AbilitySystemComponent->GetGameplayEffectDuration(Handle);
}

void UDOAbilitySystemBlueprintLibrary::GetGameplayEffectStartTimeAndDuration(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle, float& StartEffectTime, float& Duration)
{
	StartEffectTime = 0.0f;
	Duration = 0.0f;

	if (!AbilitySystemComponent || !Handle.IsValid())
	{
		return;
	}

	AbilitySystemComponent->GetGameplayEffectStartTimeAndDuration(Handle, StartEffectTime, Duration);
}

void UDOAbilitySystemBlueprintLibrary::GetAbilityCooldownTimeRemainingAndDurationByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle, float& TimeRemaining, float& CooldownDuration)
{
	TimeRemaining = 0.0f;
	CooldownDuration = 0.0f;

	if (!AbilitySystemComponent || !Handle.IsValid())
	{
		return;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	UGameplayAbility* Ability = AbilitySpec ? AbilitySpec->GetPrimaryInstance() : nullptr;
	if (!Ability)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo || ActorInfo->AbilitySystemComponent.Get() != AbilitySystemComponent)
	{
		return;
	}

	Ability->GetCooldownTimeRemainingAndDuration(Handle, ActorInfo, TimeRemaining, CooldownDuration);
}

void UDOAbilitySystemBlueprintLibrary::GetAbilityCooldownTimeRemainingAndDurationByAbility(UAbilitySystemComponent* AbilitySystemComponent, UGameplayAbility* Ability, float& TimeRemaining, float& CooldownDuration)
{
	TimeRemaining = 0.0f;
	CooldownDuration = 0.0f;

	if (!AbilitySystemComponent || !Ability)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo || ActorInfo->AbilitySystemComponent.Get() != AbilitySystemComponent)
	{
		return;
	}

	Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(), ActorInfo, TimeRemaining, CooldownDuration);
}

UGameplayAbility* UDOAbilitySystemBlueprintLibrary::GetPrimaryAbilityInstanceFromClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilitySystemComponent || !AbilityClass)
	{
		return nullptr;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
	return AbilitySpec ? AbilitySpec->GetPrimaryInstance() : nullptr;
}

UGameplayAbility* UDOAbilitySystemBlueprintLibrary::GetPrimaryAbilityInstanceFromHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle)
{
	if (!AbilitySystemComponent || !Handle.IsValid())
	{
		return nullptr;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	return AbilitySpec ? AbilitySpec->GetPrimaryInstance() : nullptr;
}
