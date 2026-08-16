#pragma once

#include "AbilitySystem/Abilities/Core/DOGameplayAbility.h"

#include "DOTestGameplayAbility.generated.h"

/**
 * 测试用的最小具体 Ability，避免生命周期测试依赖抽象的业务技能类。
 */
UCLASS()
class DRAGONOATH_API UDOTestGameplayAbility : public UDOGameplayAbility
{
	GENERATED_BODY()

public:
	UDOTestGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
