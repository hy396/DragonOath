#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"

/** 单个背包格子的稳定展示模型；只包含 UI 所需的只读快照。 */
struct DRAGONOATH_API FDOInventorySlotViewModel
{
	FDOItemInstanceRecord Item;
	FText DisplayName;
	FText Description;
	TSoftObjectPtr<UTexture2D> Icon;
	FGameplayTag ItemType;
	FGameplayTag Rarity;
	FGameplayTag EquipmentSlotTag;
	bool bIsEmpty = true;
	bool bIsEquipped = false;
	bool bIsUsable = false;
	bool bCanDiscard = true;
	bool bIsPending = false;

	FGuid GetInstanceId() const { return Item.InstanceId; }
	int32 GetSlotIndex() const { return Item.SlotIndex; }
};

