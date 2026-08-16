#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "DOItemInstanceTypes.generated.h"

/** 物品实例的动态词缀结果。静态词缀规则仍由 ItemDefinition 提供。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemAffixRoll
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (DisplayName = "词缀标签"))
	FGameplayTag AffixTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, meta = (DisplayName = "词缀数值"))
	float Magnitude = 0.0f;
};

/** 一个稳定的运行时物品实例。容器组件拥有并修改该值类型记录。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemInstanceRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid InstanceId;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FPrimaryAssetId DefinitionId;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 StackCount = 1;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 UpgradeLevel = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 CurrentDurability = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<FDOItemAffixRoll> Affixes;

	bool IsValid() const
	{
		return InstanceId.IsValid() && DefinitionId.IsValid() && StackCount > 0;
	}
};
