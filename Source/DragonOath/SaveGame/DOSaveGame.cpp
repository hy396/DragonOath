#include "SaveGame/DOSaveGame.h"

#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "Player/DOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOSaveGame)

UDOSaveGame* UDOSaveGame::CaptureFromPlayerState(ADOPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return nullptr;
	}

	UDOSaveGame* SaveGame = NewObject<UDOSaveGame>(GetTransientPackage());
	SaveGame->SaveVersion = CurrentSaveVersion;
	SaveGame->ProfessionTag = PlayerState->GetProfessionTag();

	if (const UDOInventoryComponent* Inventory = PlayerState->GetInventoryComponent())
	{
		SaveGame->InventoryCapacity = Inventory->GetCapacity();
		Inventory->GetInventorySnapshot(SaveGame->InventoryItems);
	}

	if (const UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent())
	{
		TArray<FDOEquippedItemEntry> EquippedEntries;
		Equipment->GetEquippedSnapshot(EquippedEntries);
		SaveGame->EquippedItems.Reserve(EquippedEntries.Num());
		for (const FDOEquippedItemEntry& Entry : EquippedEntries)
		{
			FDOEquipmentSaveEntry& SaveEntry = SaveGame->EquippedItems.AddDefaulted_GetRef();
			SaveEntry.SlotTag = Entry.SlotTag;
			SaveEntry.Item = Entry.Item;
		}
	}

	if (const UDOItemQuickBarComponent* QuickBar = PlayerState->GetItemQuickBarComponent())
	{
		QuickBar->GetQuickBarSnapshot(SaveGame->QuickBarDefinitions);
	}

	return SaveGame;
}

bool UDOSaveGame::RestoreToPlayerState(ADOPlayerState* PlayerState)
{
	if (!PlayerState || !PlayerState->HasAuthority() || !MigrateToCurrentVersion())
	{
		return false;
	}

	UDOInventoryComponent* Inventory = PlayerState->GetInventoryComponent();
	UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
	UDOItemQuickBarComponent* QuickBar = PlayerState->GetItemQuickBarComponent();
	if (!Inventory || !Equipment || !QuickBar || QuickBarDefinitions.Num() != UDOItemQuickBarComponent::QuickBarSlotCount)
	{
		return false;
	}

	TArray<FDOEquippedItemEntry> NewEquippedEntries;
	NewEquippedEntries.Reserve(EquippedItems.Num());
	for (const FDOEquipmentSaveEntry& SaveEntry : EquippedItems)
	{
		FDOEquippedItemEntry& Entry = NewEquippedEntries.AddDefaulted_GetRef();
		Entry.SlotTag = SaveEntry.SlotTag;
		Entry.Item = SaveEntry.Item;
		Entry.Item.SlotIndex = INDEX_NONE;
	}

	TSet<FGuid> InventoryInstanceIds;
	for (const FDOItemInstanceRecord& Item : InventoryItems)
	{
		InventoryInstanceIds.Add(Item.InstanceId);
	}
	if (NewEquippedEntries.ContainsByPredicate([&InventoryInstanceIds](const FDOEquippedItemEntry& Entry)
	{
		return InventoryInstanceIds.Contains(Entry.Item.InstanceId);
	}))
	{
		return false;
	}

	// 唯一物品规则必须跨普通背包和装备栏一起校验，不能通过读档同时恢复两份。
	TSet<FPrimaryAssetId> UniqueDefinitionIds;
	auto CheckUniqueDefinition = [PlayerState, &UniqueDefinitionIds](const FDOItemInstanceRecord& Item) -> bool
	{
		const UDOItemDefinition* Definition = UDOItemDefinitionSubsystem::ResolveItemDefinition(PlayerState, Item.DefinitionId);
		const UDOItemFragment_Inventory* InventoryFragment = Definition
			? Definition->FindFragment<UDOItemFragment_Inventory>()
			: nullptr;
		if (InventoryFragment && InventoryFragment->bUnique)
		{
			if (UniqueDefinitionIds.Contains(Item.DefinitionId))
			{
				return false;
			}
			UniqueDefinitionIds.Add(Item.DefinitionId);
		}
		return true;
	};
	for (const FDOItemInstanceRecord& Item : InventoryItems)
	{
		if (!CheckUniqueDefinition(Item))
		{
			return false;
		}
	}
	for (const FDOEquippedItemEntry& Entry : NewEquippedEntries)
	{
		if (!CheckUniqueDefinition(Entry.Item))
		{
			return false;
		}
	}

	const FGameplayTag PreviousProfessionTag = PlayerState->GetProfessionTag();
	const bool bProfessionChanged = ProfessionTag.IsValid() && PreviousProfessionTag != ProfessionTag;
	if (bProfessionChanged)
	{
		PlayerState->SetProfession(ProfessionTag);
	}

	if (!Inventory->ValidateInventorySnapshot(InventoryItems, InventoryCapacity) || !Equipment->ValidateEquippedSnapshot(NewEquippedEntries))
	{
		if (bProfessionChanged)
		{
			PlayerState->SetProfession(PreviousProfessionTag);
		}
		return false;
	}

	// 保存当前状态用于恢复失败时回滚，避免半套存档写入运行时对象。
	TArray<FDOItemInstanceRecord> PreviousInventoryItems;
	Inventory->GetInventorySnapshot(PreviousInventoryItems);
	const int32 PreviousCapacity = Inventory->GetCapacity();
	TArray<FDOEquippedItemEntry> PreviousEquippedEntries;
	Equipment->GetEquippedSnapshot(PreviousEquippedEntries);
	TArray<FPrimaryAssetId> PreviousQuickBarDefinitions;
	QuickBar->GetQuickBarSnapshot(PreviousQuickBarDefinitions);

	if (!Inventory->RestoreInventorySnapshot(InventoryItems, InventoryCapacity) || !Equipment->RestoreEquippedSnapshot(NewEquippedEntries) || !QuickBar->RestoreQuickBarSnapshot(QuickBarDefinitions))
	{
		Inventory->RestoreInventorySnapshot(PreviousInventoryItems, PreviousCapacity);
		Equipment->RestoreEquippedSnapshot(PreviousEquippedEntries);
		QuickBar->RestoreQuickBarSnapshot(PreviousQuickBarDefinitions);
		if (bProfessionChanged)
		{
			PlayerState->SetProfession(PreviousProfessionTag);
		}
		return false;
	}

	return true;
}

bool UDOSaveGame::MigrateToCurrentVersion()
{
	if (SaveVersion <= 0 || SaveVersion > CurrentSaveVersion)
	{
		return false;
	}

	// 未来新增字段时在这里按旧版本逐级补默认值，最后统一提升到当前版本。
	SaveVersion = CurrentSaveVersion;
	return true;
}
