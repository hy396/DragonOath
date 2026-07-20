// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_GameplayTagAddedRemoved.generated.h"

UCLASS()
class SHAREDCOOLINGABILITY_API UAbilityTask_GameplayTagAddedRemoved : public UAbilityTask
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagAddedRemoved, FGameplayTag, Tag);

	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagRemoved;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GameplayTagAddedRemoved* AWaitAnyGameplayTagAddedOrRemoved(UGameplayAbility* OwningAbility, FGameplayTagContainer InTags);

	virtual void Activate() override;
	void OnDestroy(bool AbilityEnding) override;

	UAbilitySystemComponent* GetTargetAbilitySystemComponent();

protected:
	FGameplayTagContainer Tags;
	virtual void TagChanged(const FGameplayTag Tag, int32 NewCount);
};
