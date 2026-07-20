// Copyright 2020 Dan Kestranek.


#include "Tasks/AbilityTasks/AbilityTask_GameplayTagAddedRemoved.h"


UAbilityTask_GameplayTagAddedRemoved* UAbilityTask_GameplayTagAddedRemoved::AWaitAnyGameplayTagAddedOrRemoved(UGameplayAbility* OwningAbility, FGameplayTagContainer InTags)
{
	UAbilityTask_GameplayTagAddedRemoved* ListenForGameplayTagAddedRemoved = NewObject<UAbilityTask_GameplayTagAddedRemoved>();
	ListenForGameplayTagAddedRemoved->Tags = InTags;
	return ListenForGameplayTagAddedRemoved;
}

void UAbilityTask_GameplayTagAddedRemoved::Activate()
{
	if (UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent())
	{
		for (auto TagIt = Tags.CreateConstIterator(); TagIt; ++TagIt)
		{
			GetTargetAbilitySystemComponent()->RegisterGameplayTagEvent(*TagIt, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
			ASC->RegisterGameplayTagEvent(*TagIt, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAbilityTask_GameplayTagAddedRemoved::TagChanged);
		}
	}
}

void UAbilityTask_GameplayTagAddedRemoved::OnDestroy(bool AbilityEnding)
{
	if (IsValid(GetTargetAbilitySystemComponent()) && !Tags.IsEmpty())
	{
		for (auto TagIt = Tags.CreateConstIterator(); TagIt; ++TagIt)
		{
			GetTargetAbilitySystemComponent()->RegisterGameplayTagEvent(*TagIt, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}
	Super::OnDestroy(AbilityEnding);
}

UAbilitySystemComponent* UAbilityTask_GameplayTagAddedRemoved::GetTargetAbilitySystemComponent()
{
#if ENGINE_MAJOR_VERSION == 5 
	return AbilitySystemComponent.Get();
#else
	return AbilitySystemComponent;
#endif
}


void UAbilityTask_GameplayTagAddedRemoved::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		OnTagAdded.Broadcast(Tag);
	}
	else
	{
		OnTagRemoved.Broadcast(Tag);
	}
}
