#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"

#include "DOInventoryMessages.generated.h"

class UDOEquipmentComponent;
class UDOInventoryComponent;
class UDOItemQuickBarComponent;

/** 背包 FastArray 聚合刷新消息，只传变化实例 ID，不把消息总线当作操作通道。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOInventoryChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDOInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> ChangedInstanceIds;

	UPROPERTY(BlueprintReadOnly)
	int32 Revision = 0;
};

/** 装备数据变化消息。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOEquipmentChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDOEquipmentComponent> EquipmentComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGameplayTag> ChangedSlotTags;

	UPROPERTY(BlueprintReadOnly)
	int32 Revision = 0;
};

/** 快捷栏数据变化消息。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemQuickBarChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDOItemQuickBarComponent> QuickBarComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 Revision = 0;
};

/** 背包操作失败消息。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOInventoryOperationFailedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDOInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 ClientOperationId = 0;

	UPROPERTY(BlueprintReadOnly)
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::Unknown;
};

/** 装备请求失败消息，用于让客户端清理装备操作的 Pending 状态。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOEquipmentOperationFailedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDOEquipmentComponent> EquipmentComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 ClientOperationId = 0;

	UPROPERTY(BlueprintReadOnly)
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::Unknown;
};

/** 快捷栏请求失败消息，用于让快捷栏 UI 清理 Pending 状态。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemQuickBarOperationFailedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDOItemQuickBarComponent> QuickBarComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 ClientOperationId = 0;

	UPROPERTY(BlueprintReadOnly)
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::Unknown;
};
