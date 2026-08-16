#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "DOInventorySortConfig.generated.h"

/** 可选的库存排序权重资产，缺省时使用 Definition 自带 SortPriority。 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOInventorySortConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Inventory|Sort", meta = (DisplayName = "物品类型权重"))
	TMap<FGameplayTag, int32> ItemTypeWeights;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Inventory|Sort", meta = (DisplayName = "装备槽位权重"))
	TMap<FGameplayTag, int32> EquipmentSlotWeights;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Inventory|Sort", meta = (DisplayName = "稀有度权重"))
	TMap<FGameplayTag, int32> RarityWeights;

	int32 GetItemTypeWeight(const FGameplayTag& Tag, const int32 Fallback) const;
	int32 GetEquipmentSlotWeight(const FGameplayTag& Tag, const int32 Fallback) const;
	int32 GetRarityWeight(const FGameplayTag& Tag, const int32 Fallback) const;
};
