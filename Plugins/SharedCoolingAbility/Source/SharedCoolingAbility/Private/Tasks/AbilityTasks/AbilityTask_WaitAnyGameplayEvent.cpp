// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tasks/AbilityTasks/AbilityTask_WaitAnyGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

// ----------------------------------------------------------------


UAbilityTask_WaitAnyGameplayEvent* UAbilityTask_WaitAnyGameplayEvent::WaitAnyGameplayEvent(UGameplayAbility* OwningAbility, FGameplayTagContainer Tags, AActor* OptionalExternalTarget, bool OnlyTriggerOnce, bool OnlyMatchExact)
{
	UAbilityTask_WaitAnyGameplayEvent* MyObj = NewAbilityTask<UAbilityTask_WaitAnyGameplayEvent>(OwningAbility);
	MyObj->EventTags = Tags;
	MyObj->SetExternalTarget(OptionalExternalTarget);
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;
	MyObj->OnlyMatchExact = OnlyMatchExact;

	return MyObj;
}

void UAbilityTask_WaitAnyGameplayEvent::Activate()
{
	UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent();
	if (ASC)
	{
		TArray<FGameplayTag> OutGameplayTags;
		EventTags.GetGameplayTagArray(OutGameplayTags);
		for (auto& Tag : OutGameplayTags)
		{
			if (OnlyMatchExact)
			{
				AllHandles.FindOrAdd(Tag,ASC->GenericGameplayEventCallbacks.FindOrAdd(Tag).AddUObject(this, &UAbilityTask_WaitAnyGameplayEvent::GameplayEventCallback));
			}
			else
			{
				AllHandles.FindOrAdd(Tag,ASC->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(Tag), FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UAbilityTask_WaitAnyGameplayEvent::GameplayEventContainerCallback)));
			}
		}
	}

	Super::Activate();
}

void UAbilityTask_WaitAnyGameplayEvent::GameplayEventCallback(const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempPayload = *Payload;
		EventReceived.Broadcast(MoveTemp(TempPayload));
	}
	if (OnlyTriggerOnce)
	{
		EndTask();
	}
}

void UAbilityTask_WaitAnyGameplayEvent::GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempPayload = *Payload;
		TempPayload.EventTag = MatchingTag;
		EventReceived.Broadcast(MoveTemp(TempPayload));
	}
	if (OnlyTriggerOnce)
	{
		EndTask();
	}
}

void UAbilityTask_WaitAnyGameplayEvent::SetExternalTarget(AActor* Actor)
{
	if (Actor)
	{
		UseExternalTarget = true;
		OptionalExternalTarget = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	}
}

UAbilitySystemComponent* UAbilityTask_WaitAnyGameplayEvent::GetTargetAbilitySystemComponent()
{
	if (UseExternalTarget)
	{
		return OptionalExternalTarget;
	}

#if ENGINE_MAJOR_VERSION == 5 
	return AbilitySystemComponent.Get();
#else
	return AbilitySystemComponent;
#endif
}

void UAbilityTask_WaitAnyGameplayEvent::OnDestroy(bool AbilityEnding)
{
	if (AllHandles.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent())
		{
			for (auto& CurrentHandle : AllHandles)
			{
				if (CurrentHandle.Value.IsValid())
				{
					if (OnlyMatchExact)
					{
						ASC->GenericGameplayEventCallbacks.FindOrAdd(CurrentHandle.Key).Remove(CurrentHandle.Value);
					}
					else
					{
						ASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(CurrentHandle.Key), CurrentHandle.Value);
					}

				}
			}
		}
	}

	Super::OnDestroy(AbilityEnding);
}
