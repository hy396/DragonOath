#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "DOEquipmentLayout.generated.h"

/** 角色装备布局的静态配置。为空时由装备组件使用项目默认槽位集合。 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOEquipmentLayout : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Equipment|Layout", meta = (Categories = "Equipment.Slot", DisplayName = "装备槽位标签"))
	TArray<FGameplayTag> SlotTags;

	bool ContainsSlot(const FGameplayTag& SlotTag) const
	{
		return SlotTag.IsValid() && SlotTags.Contains(SlotTag);
	}

	int32 GetSlotCount() const
	{
		return SlotTags.Num();
	}
};
