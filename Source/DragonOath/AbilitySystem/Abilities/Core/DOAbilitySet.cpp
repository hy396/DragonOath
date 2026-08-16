// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Core/DOAbilitySet.h"

#include "AbilitySystem/Abilities/Core/DOGameplayAbility.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOAbilitySet)

#define LOCTEXT_NAMESPACE "DOAbilitySet"

EDataValidationResult UDOAbilitySet::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (int32 Index = 0; Index < GrantedAbilities.Num(); ++Index)
	{
		const FDOAbilityGrant& Grant = GrantedAbilities[Index];
		const FString Prefix = FString::Printf(TEXT("GrantedAbilities[%d]: "), Index);

		if (Grant.AbilityClass == nullptr)
		{
			Context.AddError(FText::Format(LOCTEXT("MissingAbilityClass", "{0}AbilityClass 未设置。"), FText::FromString(Prefix)));
			Result = EDataValidationResult::Invalid;
		}

		if (!Grant.AbilityId.IsValid())
		{
			Context.AddError(FText::Format(LOCTEXT("MissingAbilityId", "{0}AbilityId 未设置，无法用于 UI/存档/升级查找。"), FText::FromString(Prefix)));
			Result = EDataValidationResult::Invalid;
		}

		switch (Grant.TriggerType)
		{
		case EDOAbilityGrantTriggerType::Input:
			if (!Grant.InputTag.IsValid())
			{
				Context.AddError(FText::Format(LOCTEXT("MissingInputTag", "{0}TriggerType = Input 但 InputTag 未设置。"), FText::FromString(Prefix)));
				Result = EDataValidationResult::Invalid;
			}
			break;

		case EDOAbilityGrantTriggerType::GameplayEvent:
			if (!Grant.EventTag.IsValid())
			{
				Context.AddError(FText::Format(LOCTEXT("MissingEventTag", "{0}TriggerType = GameplayEvent 但 EventTag 未设置。"), FText::FromString(Prefix)));
				Result = EDataValidationResult::Invalid;
			}
			break;

		default:
			break;
		}
	}

	for (int32 Index = 0; Index < GrantedGameplayEffects.Num(); ++Index)
	{
		if (!GrantedGameplayEffects[Index].GameplayEffectClass)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("GrantedGameplayEffects[%d] 缺少 GameplayEffectClass。"), Index)));
			Result = EDataValidationResult::Invalid;
		}
		if (!FMath::IsFinite(GrantedGameplayEffects[Index].Level) || GrantedGameplayEffects[Index].Level <= 0.0f)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("GrantedGameplayEffects[%d] Level 必须大于 0。"), Index)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
