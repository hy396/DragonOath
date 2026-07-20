// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interface/SharedCoolingInterface.h"
#include "ASC_SharedCoolingComponent.generated.h"

/**
 * Using the SharedCooling ,it is not mandatory to use ASC_SharedCoolingComponent; you can also use a custom AbilitySystemComponent.
 * 
 * If you want to customize AbilitySystemComponent, please be sure to inherit from ISharedCoolingSystemInterface.
 */
UCLASS()
class SHAREDCOOLINGABILITY_API UASC_SharedCoolingComponent : public UAbilitySystemComponent,public ISharedCoolingInterface
{
	GENERATED_BODY()
public:

};
