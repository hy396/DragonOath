// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/BlueprintAsync/AsyncTask_WaitAnyGameplayEvent.h"
#include "AbilitySystemComponent.h"


UAbilityAsync_WaitAnyGameplayEvent* UAbilityAsync_WaitAnyGameplayEvent::WaitAnyGameplayEventToActor(AActor* TargetActor, FGameplayTagContainer EventTags, bool OnlyTriggerOnce, bool OnlyMatchExact)
{
	UAbilityAsync_WaitAnyGameplayEvent* MyObj = NewObject<UAbilityAsync_WaitAnyGameplayEvent>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->EventTags = EventTags;
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;
	MyObj->OnlyMatchExact = OnlyMatchExact;
	return MyObj;
}

void UAbilityAsync_WaitAnyGameplayEvent::GameplayEventCallback(const FGameplayEventData* Payload)
{
	if (ShouldBroadcastDelegates())
	{
		FGameplayEventData TempPayload = *Payload;
		EventReceived.Broadcast(MoveTemp(TempPayload));
	}
	if (OnlyTriggerOnce)
	{
		EndAction();
	}
}

void UAbilityAsync_WaitAnyGameplayEvent::GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastDelegates())
	{
		FGameplayEventData TempPayload = *Payload;
		TempPayload.EventTag = MatchingTag;
		EventReceived.Broadcast(MoveTemp(TempPayload));
	}
	if (OnlyTriggerOnce)
	{
		EndAction();
	}
}

void UAbilityAsync_WaitAnyGameplayEvent::Activate()
{
	Super::Activate();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (ASC)
	{
		TArray<FGameplayTag> OutGameplayTags;
		EventTags.GetGameplayTagArray(OutGameplayTags);
		for (auto& Tag : OutGameplayTags)
		{
			if (OnlyMatchExact)
			{
				AllDelegateHandles.FindOrAdd(Tag, ASC->GenericGameplayEventCallbacks.FindOrAdd(Tag).AddUObject(this, &UAbilityAsync_WaitAnyGameplayEvent::GameplayEventCallback));
			}
			else
			{
				AllDelegateHandles.FindOrAdd(Tag, ASC->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(Tag), FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UAbilityAsync_WaitAnyGameplayEvent::GameplayEventContainerCallback)));
			}
		}
	}
	else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitAnyGameplayEvent::EndAction()
{
	if (AllDelegateHandles.Num() > 0)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			for (auto& CurrentHandle : AllDelegateHandles)
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

	Super::EndAction();
}
