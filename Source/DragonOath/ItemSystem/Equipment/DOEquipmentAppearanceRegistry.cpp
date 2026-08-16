#include "ItemSystem/Equipment/DOEquipmentAppearanceRegistry.h"

#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOEquipmentAppearanceRegistry)

bool FDOEquipmentAppearanceEntry::Matches(const FName InAppearanceId, const FGameplayTag& InVariantTag) const
{
	return AppearanceId == InAppearanceId && (!InVariantTag.IsValid() || VariantTag == InVariantTag);
}

const FDOEquipmentAppearanceEntry* UDOEquipmentAppearanceRegistry::FindAppearance(const FName InAppearanceId, const FGameplayTag& InVariantTag) const
{
	if (InAppearanceId.IsNone())
	{
		return nullptr;
	}

	// Prefer an exact variant.  An empty variant entry is the deterministic fallback.
	const FDOEquipmentAppearanceEntry* Fallback = nullptr;
	for (const FDOEquipmentAppearanceEntry& Entry : Entries)
	{
		if (Entry.AppearanceId != InAppearanceId)
		{
			continue;
		}

		if (InVariantTag.IsValid() && Entry.VariantTag == InVariantTag)
		{
			return &Entry;
		}
		if (!Entry.VariantTag.IsValid())
		{
			Fallback = &Entry;
		}
	}

	return Fallback;
}

bool UDOEquipmentAppearanceRegistry::ResolveAppearance(const FName InAppearanceId, const FGameplayTag InVariantTag, FDOEquipmentAppearanceEntry& OutAppearance) const
{
	if (const FDOEquipmentAppearanceEntry* Entry = FindAppearance(InAppearanceId, InVariantTag))
	{
		OutAppearance = *Entry;
		return true;
	}

	OutAppearance = FDOEquipmentAppearanceEntry();
	return false;
}

EDataValidationResult UDOEquipmentAppearanceRegistry::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const FDOEquipmentAppearanceEntry& Entry = Entries[Index];
		const FString Prefix = FString::Printf(TEXT("Entries[%d]: "), Index);
		if (Entry.AppearanceId.IsNone())
		{
			Context.AddError(FText::Format(FText::FromString(TEXT("{0}AppearanceId must be set.")), FText::FromString(Prefix)));
			Result = EDataValidationResult::Invalid;
		}

		for (int32 OtherIndex = 0; OtherIndex < Index; ++OtherIndex)
		{
			const FDOEquipmentAppearanceEntry& Other = Entries[OtherIndex];
			if (Other.AppearanceId == Entry.AppearanceId && Other.VariantTag == Entry.VariantTag)
			{
				Context.AddError(FText::Format(
					FText::FromString(TEXT("{0}duplicates Entries[{1}] for the same AppearanceId and VariantTag.")),
					FText::FromString(Prefix),
					FText::AsNumber(OtherIndex)));
				Result = EDataValidationResult::Invalid;
				break;
			}
		}
	}

	return Result;
}
