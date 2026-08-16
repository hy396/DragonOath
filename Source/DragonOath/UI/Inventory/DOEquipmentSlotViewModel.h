#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"

class UTexture2D;

/** 单个装备槽的稳定展示模型；Slate 不再直接查询 EquipmentComponent。 */
struct DRAGONOATH_API FDOEquipmentSlotViewModel
{
	FGameplayTag SlotTag;
	FText DisplayName;
	FText ItemDisplayName;
	FDOItemInstanceRecord Item;
	TSoftObjectPtr<UTexture2D> Icon;
	FGameplayTag Rarity;
	bool bIsEmpty = true;
	bool bIsPending = false;

	FGuid GetInstanceId() const { return Item.InstanceId; }
};
