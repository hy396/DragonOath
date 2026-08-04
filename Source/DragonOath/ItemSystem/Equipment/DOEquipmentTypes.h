#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"

#include "DOEquipmentTypes.generated.h"

class UDOEquipmentComponent;
struct FDOEquipmentList;

/** Owner-only 的完整装备实例。 */
USTRUCT()
struct DRAGONOATH_API FDOEquippedItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag SlotTag;

	UPROPERTY()
	FDOItemInstanceRecord Item;

	void PostReplicatedAdd(const FDOEquipmentList& /*Serializer*/) {}
	void PostReplicatedChange(const FDOEquipmentList& /*Serializer*/) {}
	void PreReplicatedRemove(const FDOEquipmentList& /*Serializer*/) {}
};

/** 完整装备列表的 FastArray 容器。 */
USTRUCT()
struct DRAGONOATH_API FDOEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDOEquippedItemEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UDOEquipmentComponent> OwnerComponent = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FDOEquippedItemEntry, FDOEquipmentList>(Entries, DeltaParams, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
};

template<>
struct TStructOpsTypeTraits<FDOEquipmentList> : public TStructOpsTypeTraitsBase2<FDOEquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};
