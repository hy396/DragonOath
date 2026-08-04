#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "ItemSystem/Core/DOItemDefinition.h"

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

/** 第一版保留的动态词缀结构，暂不生成随机词缀。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemAffixRoll
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AffixTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Magnitude = 0.0f;
};

/** 一个背包堆栈就是一个稳定的运行时实例。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemInstanceRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid InstanceId;

	UPROPERTY(BlueprintReadOnly)
	FPrimaryAssetId DefinitionId;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	int32 UpgradeLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentDurability = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FDOItemAffixRoll> Affixes;

	bool IsValid() const
	{
		return InstanceId.IsValid() && DefinitionId.IsValid() && StackCount > 0;
	}
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

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FDOInventoryEntry, FDOInventoryList>(Entries, DeltaParams, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
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
};
