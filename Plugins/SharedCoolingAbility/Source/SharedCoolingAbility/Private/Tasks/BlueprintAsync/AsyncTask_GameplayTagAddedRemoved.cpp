// Copyright 2020 Dan Kestranek.


#include "Tasks/BlueprintAsync/AsyncTask_GameplayTagAddedRemoved.h"


UAsyncTask_GameplayTagAddedRemoved* UAsyncTask_GameplayTagAddedRemoved::WaitAnyGameplayTagAddedOrRemoved(UAbilitySystemComponent * AbilitySystemComponent, FGameplayTagContainer InTags)
{
	UAsyncTask_GameplayTagAddedRemoved* ListenForGameplayTagAddedRemoved = NewObject<UAsyncTask_GameplayTagAddedRemoved>();
	ListenForGameplayTagAddedRemoved->ASC = AbilitySystemComponent;
	ListenForGameplayTagAddedRemoved->Tags = InTags;

	if (!IsValid(AbilitySystemComponent) || InTags.IsEmpty())
	{
		ListenForGameplayTagAddedRemoved->EndTask();
		return nullptr;
	}

	for (auto TagIt = InTags.CreateConstIterator(); TagIt; ++TagIt)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(*TagIt, EGameplayTagEventType::NewOrRemoved).AddUObject(ListenForGameplayTagAddedRemoved, &UAsyncTask_GameplayTagAddedRemoved::TagChanged);
	}

	return ListenForGameplayTagAddedRemoved;
}

void UAsyncTask_GameplayTagAddedRemoved::EndTask()
{
	if (IsValid(ASC))
	{
		for (auto TagIt = Tags.CreateConstIterator(); TagIt; ++TagIt)
		{
			ASC->RegisterGameplayTagEvent(*TagIt, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5 
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UAsyncTask_GameplayTagAddedRemoved::TagChanged(const FGameplayTag Tag, int32 NewCount)
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
