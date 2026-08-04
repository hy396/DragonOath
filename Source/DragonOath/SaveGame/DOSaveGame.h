#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "ItemSystem/Equipment/DOEquipmentTypes.h"
#include "ItemSystem/Inventory/DOInventoryTypes.h"

#include "DOSaveGame.generated.h"

class ADOPlayerState;

/** 存档中的单个装备槽记录，不携带 FastArray 的运行时序列化状态。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOEquipmentSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FDOItemInstanceRecord Item;
};

/** DragonOath 第一版玩家背包存档。真实网络运行时数据仍由 PlayerState 组件持有。 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentSaveVersion = 1;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = CurrentSaveVersion;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	FGameplayTag ProfessionTag;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Inventory")
	int32 InventoryCapacity = 40;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Inventory")
	TArray<FDOItemInstanceRecord> InventoryItems;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Equipment")
	TArray<FDOEquipmentSaveEntry> EquippedItems;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "ItemQuickBar")
	TArray<FPrimaryAssetId> QuickBarDefinitions;

	/** 从 PlayerState 创建一份独立的存档对象。 */
	static UDOSaveGame* CaptureFromPlayerState(ADOPlayerState* PlayerState);

	/** 将存档恢复到服务器上的 PlayerState，失败时不接受不完整快照。 */
	bool RestoreToPlayerState(ADOPlayerState* PlayerState);

	/** 预留给未来版本迁移；当前版本只做版本合法性检查。 */
	bool MigrateToCurrentVersion();
};
