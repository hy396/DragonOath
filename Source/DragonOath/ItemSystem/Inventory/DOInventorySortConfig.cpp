#include "ItemSystem/Inventory/DOInventorySortConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOInventorySortConfig)

int32 UDOInventorySortConfig::GetItemTypeWeight(const FGameplayTag& Tag, const int32 Fallback) const
{
	const int32* Weight = ItemTypeWeights.Find(Tag);
	return Weight ? *Weight : Fallback;
}

int32 UDOInventorySortConfig::GetEquipmentSlotWeight(const FGameplayTag& Tag, const int32 Fallback) const
{
	const int32* Weight = EquipmentSlotWeights.Find(Tag);
	return Weight ? *Weight : Fallback;
}

int32 UDOInventorySortConfig::GetRarityWeight(const FGameplayTag& Tag, const int32 Fallback) const
{
	const int32* Weight = RarityWeights.Find(Tag);
	return Weight ? *Weight : Fallback;
}
