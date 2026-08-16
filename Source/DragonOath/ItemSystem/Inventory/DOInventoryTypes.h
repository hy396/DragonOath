#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "ItemSystem/Core/DOItemInstanceTypes.h"

#include "DOInventoryTypes.generated.h"

class UDOInventoryComponent;
struct FDOInventoryList;

/** 背包请求失败原因，客户端只显示结果，不自行推断服务器状态。 */
UENUM(BlueprintType)
enum class EDOInventoryFailureReason : uint8
{
	None,
	InvalidDefinition,
	InvalidCount,
	CapacityFull,
	ItemNotFound,
	InvalidSlot,
	NotOwner,
	NotAllowed,
	InvalidTarget,
	NotEnoughQuantity,
	CannotStack,
	Locked,
	RequirementFailed,
	Unknown
};

/** FastArray 中的单条背包记录。 */
USTRUCT()
struct DRAGONOATH_API FDOInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FDOItemInstanceRecord Item;

	void PostReplicatedAdd(const FDOInventoryList& /*Serializer*/) {}
	void PostReplicatedChange(const FDOInventoryList& /*Serializer*/) {}
	void PreReplicatedRemove(const FDOInventoryList& /*Serializer*/) {}
};

/** PlayerState 背包的 Owner-only 增量复制容器。 */
USTRUCT()
struct DRAGONOATH_API FDOInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDOInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UDOInventoryComponent> OwnerComponent = nullptr;

	/** 接收端在一次 NetDeltaSerialize 内聚合的 ID，删除前必须先缓存。 */
	TArray<FGuid> PendingChangedInstanceIds;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FDOInventoryEntry, FDOInventoryList>(Entries, DeltaParams, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);
};

template<>
struct TStructOpsTypeTraits<FDOInventoryList> : public TStructOpsTypeTraitsBase2<FDOInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

/** 添加物品的结果，允许拾取系统保留未放入背包的剩余数量。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOInventoryAddResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RequestedCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 AddedCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RemainingCount = 0;

	UPROPERTY(BlueprintReadOnly)
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;

	bool IsSuccess() const { return AddedCount > 0 && FailureReason == EDOInventoryFailureReason::None; }
};

/** 一次服务器操作的结果，用于清理 UI Pending 状态和显示失败提示。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOInventoryOperationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ClientOperationId = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;

	UPROPERTY(BlueprintReadOnly)
	int32 AuthoritativeRevision = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 Outcome = 0;
};
