// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "UObject/Interface.h"

#include "GameplayAbilitySpec.h"

#include "SharedCoolingInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class USharedCoolingInterface : public UInterface
{
	GENERATED_BODY()
};

class SHAREDCOOLINGABILITY_API ISharedCoolingInterface
{
	GENERATED_BODY()


public:
	void RegisterSharedCoolingAbilities(const FGameplayTag& InTag, const FGameplayAbilitySpecHandle& GASpecHandle);
 	void CancelSharedCoolingAbilities(const FGameplayTag& InTag, const FGameplayAbilitySpecHandle& GASpecHandle);
	void NotifyAllSharedAbilityRefreshCoolTime(FGameplayTagContainer SharedCoolingGameplayEffectTag, FGameplayAbilitySpecHandle InstigatorHandle = FGameplayAbilitySpecHandle());

private:
	TMap<FGameplayTag, TArray<FGameplayAbilitySpecHandle>> SharedCoolingAbilities;
};