#pragma once

#include "GameplayEffect.h"

#include "DOItemUseEffects.generated.h"

/** 小型生命药水的即时 GameplayEffect。 */
UCLASS()
class DRAGONOATH_API UDOItemHealthPotionEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDOItemHealthPotionEffect();
};

/** 小型法力药水的即时 GameplayEffect。 */
UCLASS()
class DRAGONOATH_API UDOItemManaPotionEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDOItemManaPotionEffect();
};
