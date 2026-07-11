// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "DOProfessionAbilityConfig.generated.h"

class UDOAbilitySet;

/**
 * 职业总配置。
 *
 * 把职业 GameplayTag 映射到对应的 UDOAbilitySet。
 * 玩家初始化时根据职业 Tag 找到 AbilitySet，一次性授予全部技能。
 *
 * 资产路径建议：Content/GameData/DA_ProfessionAbilityConfig
 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOProfessionAbilityConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Key", ForceInlineRow, Categories = "Profession"))
	TMap<FGameplayTag, TObjectPtr<UDOAbilitySet>> ProfessionAbilitySets;
};