// Copyright Epic Games, Inc. All Rights Reserved.

#include "DOAbilitySystemGlobals.h"

#include "AbilitySystem/Pipeline/DOGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOAbilitySystemGlobals)

struct FGameplayEffectContext;

UDOAbilitySystemGlobals::UDOAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UDOAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FDOGameplayEffectContext();
}

