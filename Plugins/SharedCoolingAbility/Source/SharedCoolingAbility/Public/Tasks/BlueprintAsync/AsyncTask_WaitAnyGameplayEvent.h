// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AsyncTask_WaitAnyGameplayEvent.generated.h"

/**
 * 
 */
 UCLASS()
 class SHAREDCOOLINGABILITY_API UAbilityAsync_WaitAnyGameplayEvent : public UAbilityAsync
 {
	 GENERATED_BODY()
public:
 
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitGameplayEventsDelegate, FGameplayEventData, Payload);

 	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
 	static UAbilityAsync_WaitAnyGameplayEvent* WaitAnyGameplayEventToActor(AActor* TargetActor, FGameplayTagContainer EventTags, bool OnlyTriggerOnce = false, bool OnlyMatchExact = true);

	UPROPERTY(BlueprintAssignable)
	FWaitGameplayEventsDelegate EventReceived;

	virtual void GameplayEventCallback(const FGameplayEventData* Payload);
	virtual void GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload);

 protected:
	 virtual void Activate() override;
	 virtual void EndAction() override;
	 FGameplayTagContainer EventTags;
	 bool OnlyTriggerOnce = false;
	 bool OnlyMatchExact = false;
private:
	 TMap<FGameplayTag, FDelegateHandle> AllDelegateHandles;
 };
