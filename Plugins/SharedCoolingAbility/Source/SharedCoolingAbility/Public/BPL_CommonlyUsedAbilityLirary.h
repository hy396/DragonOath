// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.h"
#include "BPL_CommonlyUsedAbilityLirary.generated.h"

UENUM(BlueprintType)
enum class ETagsQueryCondition : uint8
{
	MatchAny        UMETA(DisplayName = "满足任意标签"),
	MatchAll        UMETA(DisplayName = "满足所有标签"),
};

UCLASS()
class SHAREDCOOLINGABILITY_API UBPL_CommonlyUsedAbilityLirary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/* ----------------------------------- Common Ability Lirary -----------------------------------*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (GameplayAbilitySpecHandle)", BlueprintAutocast), Category = "BPL_CommonlyUsedAbilityLirary")
 	static FString Conv_ActiveGameplayEffectHandleToString(const FActiveGameplayEffectHandle& AGEHandle);

	UFUNCTION(BlueprintPure, meta=(DisplayName = "Equal (GameplayAbilitySpecHandle)", CompactNodeTitle = "==", Keywords = "== equal"), Category="BPL_CommonlyUsedAbilityLirary")
	static bool EqualEqual_GameplayAbilitySpecHandle(const FGameplayAbilitySpecHandle& A, const FGameplayAbilitySpecHandle& B);

	UFUNCTION(BlueprintPure, meta=(DisplayName = "ToString (GameplayAbilitySpecHandle)", BlueprintAutocast), Category="BPL_CommonlyUsedAbilityLirary")
	static FString Conv_GameplayAbilitySpecHandleToString(const FGameplayAbilitySpecHandle& GameplayAbilitySpecHandle);

	UFUNCTION(BlueprintCallable , meta=(DisplayName="Give Ability" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static FGameplayAbilitySpecHandle K2_GiveAbility(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> InAbilityClass , int32 Level , int32 InputID = -1 );

	UFUNCTION(BlueprintCallable , meta=(DisplayName="TryActivateAbilityByHandle" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static bool TryActivateAbilityByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& AbilityToActivate, bool bAllowRemoteActivation = true);

	UFUNCTION(BlueprintCallable , meta=(DisplayName="ClearAbility" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void K2_ClearAbility(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle,bool InRemoveAbilityOnEnd);

	UFUNCTION( BlueprintCallable , meta=(DisplayName="ModifyGameplayEffectRemainingTimeByClass" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void ModifyGameplayEffectRemainingTimeByClass(UAbilitySystemComponent* AbilitySystemComponent, const TSubclassOf<UGameplayEffect> QueryGE, float ModifiedIncrement);
	
	UFUNCTION( BlueprintCallable , meta=(DisplayName="ModifyGameplayEffectRemainingTimeByHandle" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void ModifyGameplayEffectRemainingTimeByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& AGEHandle, float ModifiedIncrement);

	UFUNCTION( BlueprintCallable , meta=(DisplayName="ModifyGameplayEffectRemainingTimeByTags" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void ModifyGameplayEffectRemainingTimeByTags(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Tags, float ModifiedIncrement, ETagsQueryCondition TagsQueryCondition);

	UFUNCTION( BlueprintPure , meta=(DisplayName="GetGameplayEffectDuration" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static float GetGameplayEffectDuration(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle);

	UFUNCTION( BlueprintPure , meta=(DisplayName="GetGameplayEffectStartTimeAndDuration" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void GetGameplayEffectStartTimeAndDuration(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle, float& StartEffectTime, float& Duration);

	UFUNCTION( BlueprintPure , meta=(DisplayName="GetAbilityCooldownTimeRemainingAndDurationByHandle" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void GetAbilityCooldownTimeRemainingAndDurationByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle, float& TimeRemaining, float& CooldownDuration);

	UFUNCTION( BlueprintPure , meta=(DisplayName="GetAbilityCooldownTimeRemainingAndDurationByAbility" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static void GetAbilityCooldownTimeRemainingAndDurationByAbility(UAbilitySystemComponent* AbilitySystemComponent, UGameplayAbility* Ability, float& TimeRemaining, float& CooldownDuration);

	UFUNCTION( BlueprintPure , meta=(DisplayName="GetPrimaryAbilityInstanceFromClass" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static UGameplayAbility* GetPrimaryAbilityInstanceFromClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> InAbilityClass);

	UFUNCTION( BlueprintPure , meta=(DisplayName="GetPrimaryAbilityInstanceFromHandle" ) , Category = "BPL_CommonlyUsedAbilityLirary")
	static UGameplayAbility* GetPrimaryAbilityInstanceFromHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle);

};

