#pragma once

#include "CoreMinimal.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"

#include "DOItemOperationTypes.generated.h"

/** 物品域操作所属的容器。 */
UENUM(BlueprintType)
enum class EDOItemOperationDomain : uint8
{
	Inventory,
	Equipment,
	QuickBar
};

/** 操作回执状态，成功、失败和无变化都必须结束客户端 Pending。 */
UENUM(BlueprintType)
enum class EDOItemOperationOutcome : uint8
{
	Success,
	Failure,
	NoOp,
	Cancelled,
	Timeout
};

/** 所有物品域请求共用的回执字段。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemOperationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EDOItemOperationDomain Domain = EDOItemOperationDomain::Inventory;

	UPROPERTY(BlueprintReadOnly)
	EDOItemOperationOutcome Outcome = EDOItemOperationOutcome::Failure;

	UPROPERTY(BlueprintReadOnly)
	int32 ClientOperationId = 0;

	UPROPERTY(BlueprintReadOnly)
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;

	UPROPERTY(BlueprintReadOnly)
	int32 AuthoritativeRevision = 0;

	bool IsTerminalSuccess() const
	{
		return Outcome == EDOItemOperationOutcome::Success || Outcome == EDOItemOperationOutcome::NoOp;
	}
};

/** 纯库存算法输出的变更集，不依赖 World、RPC 或 Component。 */
USTRUCT()
struct DRAGONOATH_API FDOInventoryMutationChangeSet
{
	GENERATED_BODY()

	TArray<FGuid> ChangedInstanceIds;
	TArray<FGuid> RemovedInstanceIds;

	void Reset()
	{
		ChangedInstanceIds.Reset();
		RemovedInstanceIds.Reset();
	}

	void AddChanged(const FGuid& InstanceId)
	{
		if (InstanceId.IsValid())
		{
			ChangedInstanceIds.AddUnique(InstanceId);
		}
	}

	void AddRemoved(const FGuid& InstanceId)
	{
		if (InstanceId.IsValid())
		{
			RemovedInstanceIds.AddUnique(InstanceId);
		}
	}
};
