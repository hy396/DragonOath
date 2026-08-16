#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"

#include "DOItemTooltipViewModel.generated.h"

struct FDOInventorySlotViewModel;
class UDOInventoryViewModel;

/** 集中生成物品 Tooltip 和装备属性比较文本，Slate 只负责绑定返回值。 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOItemTooltipViewModel : public UObject
{
	GENERATED_BODY()

public:
	static FText BuildForInventorySlot(const FDOInventorySlotViewModel& Slot, const UDOInventoryViewModel* InventoryViewModel);
	static FText BuildForEquipmentSlot(const FGameplayTag& SlotTag, const FText& DisplayName, const UDOInventoryViewModel* InventoryViewModel);
};
