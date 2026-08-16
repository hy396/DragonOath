#pragma once

#include "CoreMinimal.h"

#include "ItemSystem/Core/DOItemInstanceTypes.h"
#include "ItemSystem/Core/DOItemOperationTypes.h"

/** 不依赖 UObject/World 的库存变更辅助算法。 */
struct DRAGONOATH_API FDOInventoryMutation
{
	static void NormalizeChangedIds(FDOInventoryMutationChangeSet& ChangeSet);

	static bool ValidateUniqueSlots(
		const TArray<FDOItemInstanceRecord>& Items,
		int32 Capacity,
		FDOInventoryMutationChangeSet* OutChangeSet = nullptr);
};
