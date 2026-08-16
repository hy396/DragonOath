#include "ItemSystem/Equipment/DOEquipmentLayout.h"

#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOEquipmentLayout)

EDataValidationResult UDOEquipmentLayout::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	TSet<FGameplayTag> UniqueSlots;
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		if (!SlotTag.IsValid() || UniqueSlots.Contains(SlotTag))
		{
			Context.AddError(FText::FromString(TEXT("Equipment Layout 包含无效或重复槽位标签。")));
			Result = EDataValidationResult::Invalid;
			break;
		}
		UniqueSlots.Add(SlotTag);
	}
	return Result;
}
