#include "ItemSystem/Inventory/DOInventoryMutation.h"

void FDOInventoryMutation::NormalizeChangedIds(FDOInventoryMutationChangeSet& ChangeSet)
{
	ChangeSet.ChangedInstanceIds.Sort([](const FGuid& A, const FGuid& B)
	{
		return A.ToString(EGuidFormats::Digits) < B.ToString(EGuidFormats::Digits);
	});
	ChangeSet.RemovedInstanceIds.Sort([](const FGuid& A, const FGuid& B)
	{
		return A.ToString(EGuidFormats::Digits) < B.ToString(EGuidFormats::Digits);
	});
}

bool FDOInventoryMutation::ValidateUniqueSlots(
	const TArray<FDOItemInstanceRecord>& Items,
	const int32 Capacity,
	FDOInventoryMutationChangeSet* OutChangeSet)
{
	if (Capacity <= 0 || Items.Num() > Capacity)
	{
		return false;
	}

	TSet<FGuid> InstanceIds;
	TSet<int32> SlotIndices;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		if (!Item.IsValid()
			|| Item.SlotIndex < 0
			|| Item.SlotIndex >= Capacity
			|| InstanceIds.Contains(Item.InstanceId)
			|| SlotIndices.Contains(Item.SlotIndex))
		{
			return false;
		}

		InstanceIds.Add(Item.InstanceId);
		SlotIndices.Add(Item.SlotIndex);
		if (OutChangeSet)
		{
			OutChangeSet->AddChanged(Item.InstanceId);
		}
	}

	if (OutChangeSet)
	{
		NormalizeChangedIds(*OutChangeSet);
	}
	return true;
}
