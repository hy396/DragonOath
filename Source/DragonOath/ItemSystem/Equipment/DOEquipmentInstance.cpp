#include "ItemSystem/Equipment/DOEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOEquipmentInstance)

void UDOEquipmentInstance::Initialize(const FDOItemInstanceRecord& InItem, const FGameplayTag& InSlotTag)
{
	Item = InItem;
	SlotTag = InSlotTag;
	Item.SlotIndex = INDEX_NONE;
	AttributeEffectHandle = FActiveGameplayEffectHandle();
	GrantedHandles.Reset();
}

void UDOEquipmentInstance::UpdateItemRuntimeState(const FDOItemInstanceRecord& InItem)
{
	Item = InItem;
	Item.SlotIndex = INDEX_NONE;
}
