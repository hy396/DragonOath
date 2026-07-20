// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_SharedCooling.generated.h"

/**
 * The GameplayEffect class is used for the shared cooldown of UGA_SharedCoolingBase.
 */
UCLASS()
class SHAREDCOOLINGABILITY_API UGE_SharedCooling : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_SharedCooling();
};
