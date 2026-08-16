#include "ItemSystem/Inventory/DOInventoryComponent.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/AbilitySystem/DOItemEffectSpecBuilder.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Inventory/DOInventorySortConfig.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "ItemSystem/Usage/DOItemUseTypes.h"
#include "Net/UnrealNetwork.h"
#include "Player/DOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOInventoryComponent)

namespace
{
	bool IsServerComponent(const UActorComponent* Component)
	{
		const AActor* Owner = Component ? Component->GetOwner() : nullptr;
		return Owner && Owner->HasAuthority();
	}

	int32 RaritySortValue(const FGameplayTag& Rarity)
	{
		if (Rarity == DragonOathGameplayTags::Item::Rarity::Legendary) return 5;
		if (Rarity == DragonOathGameplayTags::Item::Rarity::Epic) return 4;
		if (Rarity == DragonOathGameplayTags::Item::Rarity::Rare) return 3;
		if (Rarity == DragonOathGameplayTags::Item::Rarity::Uncommon) return 2;
		return 1;
	}

	int32 ItemTypeSortValue(const FGameplayTag& ItemType)
	{
		if (ItemType == DragonOathGameplayTags::Item::Type::Equipment) return 0;
		if (ItemType == DragonOathGameplayTags::Item::Type::Consumable) return 1;
		if (ItemType == DragonOathGameplayTags::Item::Type::Material) return 2;
		if (ItemType == DragonOathGameplayTags::Item::Type::Quest) return 3;
		return 4;
	}

	int32 EquipmentSlotSortValue(const FGameplayTag& SlotTag)
	{
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Head) return 0;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Shoulder) return 1;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Back) return 2;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Chest) return 3;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Hands) return 4;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Legs) return 5;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Feet) return 6;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Accessory) return 7;
		if (SlotTag == DragonOathGameplayTags::Equipment::Slot::Weapon) return 8;
		return 9;
	}

	bool IsDefinitionEquipped(const ADOPlayerState* PlayerState, const FPrimaryAssetId& DefinitionId)
	{
		if (!PlayerState || !DefinitionId.IsValid())
		{
			return false;
		}

		const UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
		if (!Equipment)
		{
			return false;
		}

		TArray<FDOEquippedItemEntry> EquippedItems;
		Equipment->GetEquippedSnapshot(EquippedItems);
		return EquippedItems.ContainsByPredicate([&DefinitionId](const FDOEquippedItemEntry& Entry)
		{
			return Entry.Item.DefinitionId == DefinitionId;
		});
	}

	FGameplayTag GetConsumableCooldownTag(const UDOItemFragment_Consumable& Fragment)
	{
		return Fragment.Cooldown.CooldownTag.IsValid() ? Fragment.Cooldown.CooldownTag : Fragment.SharedCooldownTag;
	}
}

void FDOInventoryList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : AddedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedInstanceIds.AddUnique(Entries[Index].Item.InstanceId);
		}
	}
}

void FDOInventoryList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : ChangedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedInstanceIds.AddUnique(Entries[Index].Item.InstanceId);
		}
	}
}

void FDOInventoryList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : RemovedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedInstanceIds.AddUnique(Entries[Index].Item.InstanceId);
		}
	}
}

void FDOInventoryList::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& /*Parameters*/)
{
	if (OwnerComponent && PendingChangedInstanceIds.Num() > 0)
	{
		OwnerComponent->HandleFastArrayChanged(PendingChangedInstanceIds);
	}
	PendingChangedInstanceIds.Reset();
}

UDOInventoryComponent::UDOInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	InventoryList.OwnerComponent = this;
}

void UDOInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventoryList.OwnerComponent = this;
}

void UDOInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UDOInventoryComponent, Capacity, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDOInventoryComponent, Revision, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDOInventoryComponent, InventoryList, COND_OwnerOnly);
}

void UDOInventoryComponent::GetInventorySnapshot(TArray<FDOItemInstanceRecord>& OutItems) const
{
	OutItems.Reset(InventoryList.Entries.Num());
	for (const FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		OutItems.Add(Entry.Item);
	}
	OutItems.Sort([](const FDOItemInstanceRecord& A, const FDOItemInstanceRecord& B)
	{
		return A.SlotIndex < B.SlotIndex;
	});
}

bool UDOInventoryComponent::ValidateInventorySnapshot(const TArray<FDOItemInstanceRecord>& Items, const int32 InCapacity) const
{
	if (!IsServerComponent(this) || InCapacity <= 0 || InCapacity > 1000 || Items.Num() > InCapacity)
	{
		return false;
	}

	TSet<FGuid> InstanceIds;
	TSet<int32> SlotIndices;
	TSet<FPrimaryAssetId> UniqueDefinitionIds;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		if (!Item.IsValid() || Item.SlotIndex < 0 || Item.SlotIndex >= InCapacity || InstanceIds.Contains(Item.InstanceId) || SlotIndices.Contains(Item.SlotIndex))
		{
			return false;
		}

		const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
		if (!Definition
			|| Item.StackCount > FMath::Max(1, Definition->MaxStackSize)
			|| Item.UpgradeLevel < 0)
		{
			return false;
		}
		if (const UDOItemFragment_Equipment* EquipmentFragment = Definition->FindFragment<UDOItemFragment_Equipment>())
		{
			if (Item.StackCount != 1
				|| Item.CurrentDurability < 0
				|| Item.CurrentDurability > EquipmentFragment->MaxDurability)
			{
				return false;
			}
		}

		if (const UDOItemFragment_Inventory* InventoryFragment = Definition->FindFragment<UDOItemFragment_Inventory>())
		{
			if (InventoryFragment->bUnique && UniqueDefinitionIds.Contains(Item.DefinitionId))
			{
				return false;
			}
			if (InventoryFragment->bUnique)
			{
				UniqueDefinitionIds.Add(Item.DefinitionId);
			}
		}

		InstanceIds.Add(Item.InstanceId);
		SlotIndices.Add(Item.SlotIndex);
	}
	return true;
}

bool UDOInventoryComponent::RestoreInventorySnapshot(const TArray<FDOItemInstanceRecord>& Items, const int32 InCapacity)
{
	if (!ValidateInventorySnapshot(Items, InCapacity))
	{
		return false;
	}

	Capacity = InCapacity;
	InventoryList.Entries.Reset();
	InventoryList.Entries.Reserve(Items.Num());
	TArray<FGuid> ChangedIds;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		FDOInventoryEntry& Entry = InventoryList.Entries.AddDefaulted_GetRef();
		Entry.Item = Item;
		ChangedIds.Add(Item.InstanceId);
	}

	InventoryList.MarkArrayDirty();
	BroadcastChanged(ChangedIds);
	return true;
}

const FDOItemInstanceRecord* UDOInventoryComponent::FindItemByInstanceId(const FGuid& InstanceId) const
{
	for (const FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Item.InstanceId == InstanceId)
		{
			return &Entry.Item;
		}
	}
	return nullptr;
}

FDOItemInstanceRecord* UDOInventoryComponent::FindItemByInstanceId(const FGuid& InstanceId)
{
	for (FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Item.InstanceId == InstanceId)
		{
			return &Entry.Item;
		}
	}
	return nullptr;
}

UDOItemDefinition* UDOInventoryComponent::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
{
	return const_cast<UDOItemDefinition*>(UDOItemDefinitionSubsystem::ResolveItemDefinition(this, DefinitionId));
}

int32 UDOInventoryComponent::FindEntryIndexBySlot(const int32 SlotIndex) const
{
	for (int32 Index = 0; Index < InventoryList.Entries.Num(); ++Index)
	{
		if (InventoryList.Entries[Index].Item.SlotIndex == SlotIndex)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

int32 UDOInventoryComponent::FindEmptySlot() const
{
	for (int32 Slot = 0; Slot < Capacity; ++Slot)
	{
		if (FindEntryIndexBySlot(Slot) == INDEX_NONE)
		{
			return Slot;
		}
	}
	return INDEX_NONE;
}

bool UDOInventoryComponent::IsValidSlot(const int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < Capacity;
}

void UDOInventoryComponent::MarkEntryDirty(FDOInventoryEntry& Entry)
{
	InventoryList.MarkItemDirty(Entry);
}

FDOInventoryAddResult UDOInventoryComponent::TryAddItem(const FPrimaryAssetId& DefinitionId, const int32 Count)
{
	FDOInventoryAddResult Result;
	Result.RequestedCount = Count;
	Result.RemainingCount = FMath::Max(0, Count);

	if (!IsServerComponent(this))
	{
		Result.FailureReason = EDOInventoryFailureReason::NotOwner;
		return Result;
	}

	const UDOItemDefinition* Definition = ResolveItemDefinition(DefinitionId);
	if (!Definition)
	{
		Result.FailureReason = EDOInventoryFailureReason::InvalidDefinition;
		return Result;
	}

	if (Count <= 0)
	{
		Result.FailureReason = EDOInventoryFailureReason::InvalidCount;
		return Result;
	}

	if (const UDOItemFragment_Inventory* InventoryFragment = Definition->FindFragment<UDOItemFragment_Inventory>())
	{
		// 唯一物品只能拥有一个运行时实例，不能通过重复拾取绕过唯一规则。
		const ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
		if (InventoryFragment->bUnique && Count != 1)
		{
			Result.FailureReason = EDOInventoryFailureReason::NotAllowed;
			return Result;
		}
		if (InventoryFragment->bUnique && (InventoryList.Entries.ContainsByPredicate([&DefinitionId](const FDOInventoryEntry& Entry)
		{
			return Entry.Item.DefinitionId == DefinitionId;
		}) || IsDefinitionEquipped(PlayerState, DefinitionId)))
		{
			Result.FailureReason = EDOInventoryFailureReason::NotAllowed;
			return Result;
		}
	}

	const int32 MaxStackSize = FMath::Max(1, Definition->MaxStackSize);
	TArray<FGuid> ChangedIds;

	// 先填充同 DefinitionId 的未满堆栈，保证拾取行为稳定可预测。
	for (FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Result.RemainingCount <= 0)
		{
			break;
		}

		if (Entry.Item.DefinitionId == DefinitionId && Entry.Item.StackCount < MaxStackSize)
		{
			const int32 AddCount = FMath::Min(Result.RemainingCount, MaxStackSize - Entry.Item.StackCount);
			Entry.Item.StackCount += AddCount;
			Result.AddedCount += AddCount;
			Result.RemainingCount -= AddCount;
			ChangedIds.Add(Entry.Item.InstanceId);
			MarkEntryDirty(Entry);
		}
	}

	// 再按槽位顺序创建新堆栈。
	while (Result.RemainingCount > 0)
	{
		const int32 EmptySlot = FindEmptySlot();
		if (EmptySlot == INDEX_NONE)
		{
			break;
		}

		FDOInventoryEntry& NewEntry = InventoryList.Entries.AddDefaulted_GetRef();
		NewEntry.Item.InstanceId = FGuid::NewGuid();
		NewEntry.Item.DefinitionId = DefinitionId;
		NewEntry.Item.StackCount = FMath::Min(Result.RemainingCount, MaxStackSize);
		NewEntry.Item.SlotIndex = EmptySlot;
		if (const UDOItemFragment_Equipment* EquipmentFragment = Definition->FindFragment<UDOItemFragment_Equipment>())
		{
			NewEntry.Item.CurrentDurability = EquipmentFragment->MaxDurability;
		}

		Result.AddedCount += NewEntry.Item.StackCount;
		Result.RemainingCount -= NewEntry.Item.StackCount;
		ChangedIds.Add(NewEntry.Item.InstanceId);
		InventoryList.MarkItemDirty(NewEntry);
	}

	Result.FailureReason = Result.RemainingCount > 0
		? (Result.AddedCount > 0 ? EDOInventoryFailureReason::CapacityFull : EDOInventoryFailureReason::CapacityFull)
		: EDOInventoryFailureReason::None;

	if (Result.AddedCount > 0)
	{
		BroadcastChanged(ChangedIds);
	}
	return Result;
}

bool UDOInventoryComponent::TryConsumeItem(const FGuid& InstanceId, const int32 Count, EDOInventoryFailureReason& OutFailureReason)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsServerComponent(this))
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}
	if (Count <= 0)
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidCount;
		return false;
	}

	for (int32 Index = 0; Index < InventoryList.Entries.Num(); ++Index)
	{
		FDOInventoryEntry& Entry = InventoryList.Entries[Index];
		if (Entry.Item.InstanceId != InstanceId)
		{
			continue;
		}
		if (Entry.Item.StackCount < Count)
		{
			OutFailureReason = EDOInventoryFailureReason::NotEnoughQuantity;
			return false;
		}

		if (Entry.Item.StackCount == Count)
		{
			InventoryList.Entries.RemoveAt(Index);
			InventoryList.MarkArrayDirty();
		}
		else
		{
			Entry.Item.StackCount -= Count;
			MarkEntryDirty(Entry);
		}

		BroadcastChanged({ InstanceId });
		return true;
	}

	OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
	return false;
}

bool UDOInventoryComponent::TryConsumeByDefinition(const FPrimaryAssetId& DefinitionId, const int32 Count, EDOInventoryFailureReason& OutFailureReason)
{
	const FDOInventoryEntry* Candidate = nullptr;
	for (const FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Item.DefinitionId == DefinitionId && Entry.Item.StackCount >= Count && (!Candidate || Entry.Item.SlotIndex < Candidate->Item.SlotIndex))
		{
			Candidate = &Entry;
		}
	}

	if (Candidate)
	{
		return TryConsumeItem(Candidate->Item.InstanceId, Count, OutFailureReason);
	}

	for (const FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Item.DefinitionId == DefinitionId)
		{
			OutFailureReason = EDOInventoryFailureReason::NotEnoughQuantity;
			return false;
		}
	}

	OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
	return false;
}

bool UDOInventoryComponent::TryRemoveItemByInstanceId(const FGuid& InstanceId, FDOItemInstanceRecord& OutRemovedItem, EDOInventoryFailureReason& OutFailureReason)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsServerComponent(this))
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}

	for (int32 Index = 0; Index < InventoryList.Entries.Num(); ++Index)
	{
		if (InventoryList.Entries[Index].Item.InstanceId == InstanceId)
		{
			OutRemovedItem = InventoryList.Entries[Index].Item;
			InventoryList.Entries.RemoveAt(Index);
			InventoryList.MarkArrayDirty();
			BroadcastChanged({ InstanceId });
			return true;
		}
	}

	OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
	return false;
}

bool UDOInventoryComponent::CanInsertExistingItem(const FDOItemInstanceRecord& Item, EDOInventoryFailureReason& OutFailureReason) const
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsServerComponent(this) || !Item.IsValid())
	{
		OutFailureReason = !IsServerComponent(this) ? EDOInventoryFailureReason::NotOwner : EDOInventoryFailureReason::InvalidDefinition;
		return false;
	}

	const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
	if (!Definition
		|| Item.StackCount > FMath::Max(1, Definition->MaxStackSize)
		|| Item.UpgradeLevel < 0)
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidDefinition;
		return false;
	}
	if (const UDOItemFragment_Equipment* EquipmentFragment = Definition->FindFragment<UDOItemFragment_Equipment>())
	{
		if (Item.StackCount != 1
			|| Item.CurrentDurability < 0
			|| Item.CurrentDurability > EquipmentFragment->MaxDurability)
		{
			OutFailureReason = EDOInventoryFailureReason::InvalidDefinition;
			return false;
		}
	}

	if (FindItemByInstanceId(Item.InstanceId))
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidTarget;
		return false;
	}

	if (const UDOItemFragment_Inventory* InventoryFragment = Definition->FindFragment<UDOItemFragment_Inventory>())
	{
		if (InventoryFragment->bUnique
			&& (InventoryList.Entries.ContainsByPredicate([&Item](const FDOInventoryEntry& Entry)
			{
				return Entry.Item.DefinitionId == Item.DefinitionId;
			})))
		{
			OutFailureReason = EDOInventoryFailureReason::NotAllowed;
			return false;
		}
	}

	if (FindEmptySlot() == INDEX_NONE)
	{
		OutFailureReason = EDOInventoryFailureReason::CapacityFull;
		return false;
	}

	return true;
}

bool UDOInventoryComponent::TryInsertExistingItem(const FDOItemInstanceRecord& Item, EDOInventoryFailureReason& OutFailureReason)
{
	if (!CanInsertExistingItem(Item, OutFailureReason))
	{
		return false;
	}

	const int32 EmptySlot = FindEmptySlot();
	if (EmptySlot == INDEX_NONE)
	{
		OutFailureReason = EDOInventoryFailureReason::CapacityFull;
		return false;
	}

	FDOInventoryEntry& NewEntry = InventoryList.Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;
	NewEntry.Item.SlotIndex = EmptySlot;
	InventoryList.MarkItemDirty(NewEntry);
	BroadcastChanged({ Item.InstanceId });
	return true;
}

bool UDOInventoryComponent::TryMoveItemInternal(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot, const int32 RequestedCount, EDOInventoryFailureReason& OutFailureReason, TArray<FGuid>& OutChangedIds)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsValidSlot(TargetSlot))
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidSlot;
		return false;
	}

	FDOItemInstanceRecord* SourceItem = FindItemByInstanceId(InstanceId);
	if (!SourceItem)
	{
		OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
		return false;
	}
	if (!IsValidSlot(SourceSlot) || SourceItem->SlotIndex != SourceSlot)
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidSlot;
		return false;
	}

	const int32 SourceIndex = FindEntryIndexBySlot(SourceItem->SlotIndex);
	const int32 TargetIndex = FindEntryIndexBySlot(TargetSlot);
	if (!InventoryList.Entries.IsValidIndex(SourceIndex))
	{
		OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
		return false;
	}

	if (SourceItem->SlotIndex == TargetSlot)
	{
		return true;
	}

	const int32 MoveCount = RequestedCount > 0 ? RequestedCount : SourceItem->StackCount;
	if (MoveCount <= 0 || MoveCount > SourceItem->StackCount)
	{
		OutFailureReason = EDOInventoryFailureReason::NotEnoughQuantity;
		return false;
	}

	if (TargetIndex == INDEX_NONE)
	{
		if (MoveCount == SourceItem->StackCount)
		{
			SourceItem->SlotIndex = TargetSlot;
			MarkEntryDirty(InventoryList.Entries[SourceIndex]);
			OutChangedIds.Add(InstanceId);
			return true;
		}

		return TrySplitStackInternal(InstanceId, SourceSlot, TargetSlot, MoveCount, OutFailureReason, OutChangedIds);
	}

	FDOInventoryEntry& TargetEntry = InventoryList.Entries[TargetIndex];
	const UDOItemDefinition* SourceDefinition = ResolveItemDefinition(SourceItem->DefinitionId);
	const UDOItemDefinition* TargetDefinition = ResolveItemDefinition(TargetEntry.Item.DefinitionId);
	if (SourceDefinition && TargetDefinition && SourceItem->DefinitionId == TargetEntry.Item.DefinitionId && SourceDefinition->MaxStackSize > 1)
	{
		const int32 Space = FMath::Max(0, TargetDefinition->MaxStackSize - TargetEntry.Item.StackCount);
		if (Space < MoveCount)
		{
			OutFailureReason = EDOInventoryFailureReason::CannotStack;
			return false;
		}

		TargetEntry.Item.StackCount += MoveCount;
		MarkEntryDirty(TargetEntry);
		OutChangedIds.Add(TargetEntry.Item.InstanceId);

		if (MoveCount == SourceItem->StackCount)
		{
			const FGuid RemovedId = SourceItem->InstanceId;
			InventoryList.Entries.RemoveAt(SourceIndex);
			InventoryList.MarkArrayDirty();
			OutChangedIds.Add(RemovedId);
		}
		else
		{
			SourceItem->StackCount -= MoveCount;
			MarkEntryDirty(InventoryList.Entries[SourceIndex]);
			OutChangedIds.Add(SourceItem->InstanceId);
		}
		return true;
	}

	Swap(SourceItem->SlotIndex, TargetEntry.Item.SlotIndex);
	MarkEntryDirty(InventoryList.Entries[SourceIndex]);
	MarkEntryDirty(TargetEntry);
	OutChangedIds.Add(SourceItem->InstanceId);
	OutChangedIds.Add(TargetEntry.Item.InstanceId);
	return true;
}

bool UDOInventoryComponent::TrySplitStackInternal(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot, const int32 SplitCount, EDOInventoryFailureReason& OutFailureReason, TArray<FGuid>& OutChangedIds)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	FDOItemInstanceRecord* SourceItem = FindItemByInstanceId(InstanceId);
	if (!SourceItem)
	{
		OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
		return false;
	}
	if (!IsValidSlot(SourceSlot) || SourceItem->SlotIndex != SourceSlot)
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidSlot;
		return false;
	}
	const UDOItemDefinition* Definition = ResolveItemDefinition(SourceItem->DefinitionId);
	if (!Definition || Definition->MaxStackSize <= 1)
	{
		OutFailureReason = EDOInventoryFailureReason::CannotStack;
		return false;
	}
	if (!IsValidSlot(TargetSlot) || FindEntryIndexBySlot(TargetSlot) != INDEX_NONE)
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidTarget;
		return false;
	}
	if (SplitCount <= 0 || SplitCount >= SourceItem->StackCount)
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidCount;
		return false;
	}

	SourceItem->StackCount -= SplitCount;
	const FGuid SourceId = SourceItem->InstanceId;
	const FDOItemInstanceRecord SourceSnapshot = *SourceItem;
	MarkEntryDirty(InventoryList.Entries[FindEntryIndexBySlot(SourceItem->SlotIndex)]);

	FDOInventoryEntry& NewEntry = InventoryList.Entries.AddDefaulted_GetRef();
	NewEntry.Item = SourceSnapshot;
	NewEntry.Item.InstanceId = FGuid::NewGuid();
	NewEntry.Item.StackCount = SplitCount;
	NewEntry.Item.SlotIndex = TargetSlot;
	InventoryList.MarkItemDirty(NewEntry);
	OutChangedIds.Add(SourceId);
	OutChangedIds.Add(NewEntry.Item.InstanceId);
	return true;
}

bool UDOInventoryComponent::TrySortInventory(EDOInventoryFailureReason& OutFailureReason)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsServerComponent(this))
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}

	TArray<int32> SortedIndices;
	SortedIndices.Reserve(InventoryList.Entries.Num());
	for (int32 Index = 0; Index < InventoryList.Entries.Num(); ++Index)
	{
		SortedIndices.Add(Index);
	}
	SortedIndices.Sort([this](const int32 A, const int32 B)
	{
		const FDOItemInstanceRecord& ItemA = InventoryList.Entries[A].Item;
		const FDOItemInstanceRecord& ItemB = InventoryList.Entries[B].Item;
		const UDOItemDefinition* DefinitionA = ResolveItemDefinition(ItemA.DefinitionId);
		const UDOItemDefinition* DefinitionB = ResolveItemDefinition(ItemB.DefinitionId);
		const int32 TypeA = DefinitionA
			? (SortConfig ? SortConfig->GetItemTypeWeight(DefinitionA->ItemType, ItemTypeSortValue(DefinitionA->ItemType)) : ItemTypeSortValue(DefinitionA->ItemType))
			: 4;
		const int32 TypeB = DefinitionB
			? (SortConfig ? SortConfig->GetItemTypeWeight(DefinitionB->ItemType, ItemTypeSortValue(DefinitionB->ItemType)) : ItemTypeSortValue(DefinitionB->ItemType))
			: 4;
		if (TypeA != TypeB) return TypeA < TypeB;
		const int32 PriorityA = DefinitionA ? DefinitionA->SortPriority : 0;
		const int32 PriorityB = DefinitionB ? DefinitionB->SortPriority : 0;
		if (PriorityA != PriorityB) return PriorityA > PriorityB;

		const UDOItemFragment_Equipment* EquipmentA = DefinitionA ? DefinitionA->FindFragment<UDOItemFragment_Equipment>() : nullptr;
		const UDOItemFragment_Equipment* EquipmentB = DefinitionB ? DefinitionB->FindFragment<UDOItemFragment_Equipment>() : nullptr;
		const FGameplayTag SlotTagA = EquipmentA ? EquipmentA->EquipmentSlotTag : FGameplayTag();
		const FGameplayTag SlotTagB = EquipmentB ? EquipmentB->EquipmentSlotTag : FGameplayTag();
		const int32 SlotA = SortConfig
			? SortConfig->GetEquipmentSlotWeight(SlotTagA, EquipmentSlotSortValue(SlotTagA))
			: EquipmentSlotSortValue(SlotTagA);
		const int32 SlotB = SortConfig
			? SortConfig->GetEquipmentSlotWeight(SlotTagB, EquipmentSlotSortValue(SlotTagB))
			: EquipmentSlotSortValue(SlotTagB);
		if (SlotA != SlotB) return SlotA < SlotB;
		const int32 RarityA = DefinitionA
			? (SortConfig ? SortConfig->GetRarityWeight(DefinitionA->Rarity, RaritySortValue(DefinitionA->Rarity)) : RaritySortValue(DefinitionA->Rarity))
			: 0;
		const int32 RarityB = DefinitionB
			? (SortConfig ? SortConfig->GetRarityWeight(DefinitionB->Rarity, RaritySortValue(DefinitionB->Rarity)) : RaritySortValue(DefinitionB->Rarity))
			: 0;
		if (RarityA != RarityB) return RarityA > RarityB;
		const int32 RequiredLevelA = EquipmentA ? EquipmentA->RequiredLevel : 0;
		const int32 RequiredLevelB = EquipmentB ? EquipmentB->RequiredLevel : 0;
		if (RequiredLevelA != RequiredLevelB) return RequiredLevelA > RequiredLevelB;
		if (ItemA.DefinitionId != ItemB.DefinitionId) return ItemA.DefinitionId.ToString() < ItemB.DefinitionId.ToString();
		return ItemA.InstanceId.ToString() < ItemB.InstanceId.ToString();
	});

	TArray<FGuid> ChangedIds;
	for (int32 NewSlot = 0; NewSlot < SortedIndices.Num(); ++NewSlot)
	{
		FDOInventoryEntry& Entry = InventoryList.Entries[SortedIndices[NewSlot]];
		if (Entry.Item.SlotIndex != NewSlot)
		{
			Entry.Item.SlotIndex = NewSlot;
			MarkEntryDirty(Entry);
			ChangedIds.Add(Entry.Item.InstanceId);
		}
	}

	if (ChangedIds.Num() > 0)
	{
		BroadcastChanged(ChangedIds);
	}
	return true;
}

bool UDOInventoryComponent::CanUseConsumable(
	const FDOItemInstanceRecord& Item,
	const UDOItemFragment_Consumable& Fragment,
	EDOInventoryFailureReason& OutFailureReason) const
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsServerComponent(this))
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}
	if (!Item.InstanceId.IsValid() || Item.StackCount <= 0)
	{
		OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
		return false;
	}
	if (!ResolveItemDefinition(Item.DefinitionId))
	{
		OutFailureReason = EDOInventoryFailureReason::InvalidDefinition;
		return false;
	}

	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (!ASC || !ASC->AbilityActorInfo.IsValid())
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}

	// 死亡期间禁止使用消耗品，避免死亡流程和物品效果同时修改角色状态。
	if (ASC->HasMatchingGameplayTag(DragonOathGameplayTags::Status::Death_Dying)
		|| ASC->HasMatchingGameplayTag(DragonOathGameplayTags::Status::Death_Dead))
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}

	const FGameplayTag CooldownTag = GetConsumableCooldownTag(Fragment);
	if (CooldownTag.IsValid())
	{
		FGameplayTagContainer CooldownTags;
		CooldownTags.AddTag(CooldownTag);
		// 同时匹配 OwningTags 和 AssetTags，兼容旧插件冷却与新原生冷却 GE。
		if (ASC->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags)).Num() > 0
			|| ASC->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(CooldownTags)).Num() > 0)
		{
			OutFailureReason = EDOInventoryFailureReason::Locked;
			return false;
		}
	}

	if (Fragment.EffectKind == EDOConsumableEffectKind::None)
	{
		if (!Fragment.UseGameplayEffect && !Fragment.UseGameplayAbility && !Fragment.UseEventTag.IsValid())
		{
			OutFailureReason = EDOInventoryFailureReason::NotAllowed;
			return false;
		}
	}
	else if (Fragment.EffectKind == EDOConsumableEffectKind::InstantRestore)
	{
		if (Fragment.InstantRestore.IsNearlyZero())
		{
			OutFailureReason = EDOInventoryFailureReason::NotAllowed;
			return false;
		}
	}
	else if (Fragment.EffectKind == EDOConsumableEffectKind::TimedAttributeModifier)
	{
		if (!FMath::IsFinite(Fragment.TimedModifier.DurationSeconds)
			|| Fragment.TimedModifier.DurationSeconds <= 0.0f
			|| (Fragment.TimedModifier.Modifiers.IsNearlyZero() && Fragment.TimedModifier.GrantedTags.IsEmpty()))
		{
			OutFailureReason = EDOInventoryFailureReason::NotAllowed;
			return false;
		}
	}
	else if (Fragment.EffectKind == EDOConsumableEffectKind::GameplayAbility && !Fragment.UseGameplayAbility)
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}
	else if (Fragment.EffectKind == EDOConsumableEffectKind::GameplayEvent && !Fragment.UseEventTag.IsValid())
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}

	return true;
}

bool UDOInventoryComponent::ApplyConsumableCooldown(
	const UDOItemFragment_Consumable& Fragment,
	FActiveGameplayEffectHandle& OutCooldownHandle,
	EDOInventoryFailureReason& OutFailureReason)
{
	OutCooldownHandle = FActiveGameplayEffectHandle();
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!Fragment.Cooldown.IsEnabled())
	{
		return true;
	}

	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}

	FGameplayEffectSpecHandle CooldownSpec;
	if (!FDOItemEffectSpecBuilder::BuildCooldownSpec(*ASC, *this, Fragment.Cooldown, CooldownSpec))
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}

	OutCooldownHandle = ASC->ApplyGameplayEffectSpecToSelf(*CooldownSpec.Data.Get());
	if (!OutCooldownHandle.WasSuccessfullyApplied())
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}
	return true;
}

bool UDOInventoryComponent::ApplyDirectConsumableEffect(
	const FDOItemInstanceRecord& Item,
	const UDOItemFragment_Consumable& Fragment,
	FActiveGameplayEffectHandle& OutPersistentEffectHandle,
	EDOInventoryFailureReason& OutFailureReason)
{
	OutPersistentEffectHandle = FActiveGameplayEffectHandle();
	OutFailureReason = EDOInventoryFailureReason::None;
	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}

	if (Fragment.EffectKind == EDOConsumableEffectKind::InstantRestore)
	{
		FGameplayEffectSpecHandle Spec;
		if (!FDOItemEffectSpecBuilder::BuildInstantRestoreSpec(*ASC, *this, Fragment.InstantRestore, Spec))
		{
			OutFailureReason = EDOInventoryFailureReason::NotAllowed;
			return false;
		}
		OutPersistentEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		if (OutPersistentEffectHandle.WasSuccessfullyApplied())
		{
			return true;
		}
	}
	else if (Fragment.EffectKind == EDOConsumableEffectKind::TimedAttributeModifier)
	{
		FGameplayEffectSpecHandle Spec;
		if (!FDOItemEffectSpecBuilder::BuildTimedModifierSpec(*ASC, *this, Fragment.TimedModifier, Spec))
		{
			OutFailureReason = EDOInventoryFailureReason::NotAllowed;
			return false;
		}
		OutPersistentEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		if (OutPersistentEffectHandle.WasSuccessfullyApplied())
		{
			return true;
		}
	}
	else if (Fragment.EffectKind == EDOConsumableEffectKind::None && Fragment.UseGameplayEffect)
	{
		// 旧资产兼容：迁移完成前仍允许直接指定旧 GE。
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Fragment.UseGameplayEffect, 1.0f, EffectContext);
		if (Spec.IsValid() && Spec.Data.IsValid())
		{
			OutPersistentEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			if (OutPersistentEffectHandle.WasSuccessfullyApplied())
			{
				return true;
			}
		}
	}

	OutFailureReason = EDOInventoryFailureReason::NotAllowed;
	return false;
}

bool UDOInventoryComponent::BeginComplexConsumableUse(
	const FDOItemInstanceRecord& Item,
	const UDOItemFragment_Consumable& Fragment,
	EDOInventoryFailureReason& OutFailureReason,
	const int32 ClientOperationId)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (!ASC || !PlayerState)
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		return false;
	}

	UDOItemUseContext* Context = NewObject<UDOItemUseContext>(this);
	Context->InstanceId = Item.InstanceId;
	Context->DefinitionId = Item.DefinitionId;
	Context->ClientOperationId = ClientOperationId;
	if (ClientOperationId > 0)
	{
		PendingComplexConsumableOperations.Add(ClientOperationId);
	}

	FGameplayEventData EventData;
	EventData.Instigator = PlayerState->GetPawn();
	EventData.OptionalObject = Context;
	EventData.ContextHandle = ASC->MakeEffectContext();
	EventData.ContextHandle.AddSourceObject(Context);

	if (Fragment.EffectKind == EDOConsumableEffectKind::GameplayAbility
		|| (Fragment.EffectKind == EDOConsumableEffectKind::None && Fragment.UseGameplayAbility))
	{
		FGameplayAbilitySpec AbilitySpec(Fragment.UseGameplayAbility, 1, INDEX_NONE, Context);
		if (ASC->GiveAbilityAndActivateOnce(AbilitySpec, &EventData).IsValid())
		{
			return true;
		}
		if (ClientOperationId > 0)
		{
			PendingComplexConsumableOperations.Remove(ClientOperationId);
		}
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		return false;
	}

	const FGameplayTag EventTag = Fragment.UseEventTag;
	EventData.EventTag = EventTag;
	if (EventTag.IsValid() && ASC->HandleGameplayEvent(EventTag, &EventData) > 0)
	{
		return true;
	}

	if (ClientOperationId > 0)
	{
		PendingComplexConsumableOperations.Remove(ClientOperationId);
	}
	OutFailureReason = EDOInventoryFailureReason::NotAllowed;
	return false;
}

bool UDOInventoryComponent::TryUseItemInternal(const FDOItemInstanceRecord& Item, EDOInventoryFailureReason& OutFailureReason, const int32 ClientOperationId, bool* bOutDeferredCompletion)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (bOutDeferredCompletion)
	{
		*bOutDeferredCompletion = false;
	}
	const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
	const UDOItemFragment_Consumable* Fragment = Definition ? Definition->FindFragment<UDOItemFragment_Consumable>() : nullptr;
	if (!Fragment || !CanUseConsumable(Item, *Fragment, OutFailureReason))
	{
		return false;
	}

	if (Fragment->EffectKind == EDOConsumableEffectKind::GameplayAbility
		|| Fragment->EffectKind == EDOConsumableEffectKind::GameplayEvent
		|| (Fragment->EffectKind == EDOConsumableEffectKind::None && (Fragment->UseGameplayAbility || Fragment->UseEventTag.IsValid())))
	{
		// 复杂流程不会在 Ability/Event 刚启动时扣除，最终由 CommitConsumableUse 提交。
		if (bOutDeferredCompletion)
		{
			*bOutDeferredCompletion = true;
		}
		return BeginComplexConsumableUse(Item, *Fragment, OutFailureReason, ClientOperationId);
	}

	FActiveGameplayEffectHandle CooldownHandle;
	if (!ApplyConsumableCooldown(*Fragment, CooldownHandle, OutFailureReason))
	{
		return false;
	}

	FActiveGameplayEffectHandle PersistentEffectHandle;
	if (!ApplyDirectConsumableEffect(Item, *Fragment, PersistentEffectHandle, OutFailureReason))
	{
		if (CooldownHandle.IsValid())
		{
			if (ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner()))
			{
				if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
				{
					ASC->RemoveActiveGameplayEffect(CooldownHandle);
				}
			}
		}
		return false;
	}

	if (!TryConsumeItem(Item.InstanceId, 1, OutFailureReason))
	{
		if (ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner()))
		{
			if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
			{
				if (PersistentEffectHandle.IsValid())
				{
					ASC->RemoveActiveGameplayEffect(PersistentEffectHandle);
				}
				if (CooldownHandle.IsValid())
				{
					ASC->RemoveActiveGameplayEffect(CooldownHandle);
				}
			}
		}
		return false;
	}

	return true;
}

bool UDOInventoryComponent::CommitConsumableUse(const FGuid& InstanceId, const FPrimaryAssetId& ExpectedDefinitionId, EDOInventoryFailureReason& OutFailureReason, const int32 ClientOperationId)
{
	OutFailureReason = EDOInventoryFailureReason::None;
	if (!IsServerComponent(this))
	{
		OutFailureReason = EDOInventoryFailureReason::NotOwner;
		if (ClientOperationId > 0) Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, OutFailureReason, Revision);
		return false;
	}
	if (ClientOperationId > 0 && !PendingComplexConsumableOperations.Contains(ClientOperationId))
	{
		OutFailureReason = EDOInventoryFailureReason::NotAllowed;
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, OutFailureReason, Revision);
		return false;
	}

	const FDOItemInstanceRecord* Item = FindItemByInstanceId(InstanceId);
	if (!Item || Item->DefinitionId != ExpectedDefinitionId)
	{
		OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
		if (ClientOperationId > 0)
		{
			PendingComplexConsumableOperations.Remove(ClientOperationId);
			Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, OutFailureReason, Revision);
		}
		return false;
	}

	const UDOItemDefinition* Definition = ResolveItemDefinition(Item->DefinitionId);
	const UDOItemFragment_Consumable* Fragment = Definition ? Definition->FindFragment<UDOItemFragment_Consumable>() : nullptr;
	if (!Fragment || !CanUseConsumable(*Item, *Fragment, OutFailureReason))
	{
		if (ClientOperationId > 0)
		{
			PendingComplexConsumableOperations.Remove(ClientOperationId);
			Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, OutFailureReason, Revision);
		}
		return false;
	}

	FActiveGameplayEffectHandle CooldownHandle;
	if (!ApplyConsumableCooldown(*Fragment, CooldownHandle, OutFailureReason))
	{
		if (ClientOperationId > 0)
		{
			PendingComplexConsumableOperations.Remove(ClientOperationId);
			Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, OutFailureReason, Revision);
		}
		return false;
	}

	if (TryConsumeItem(InstanceId, 1, OutFailureReason))
	{
		if (ClientOperationId > 0)
		{
			PendingComplexConsumableOperations.Remove(ClientOperationId);
			Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
		}
		return true;
	}

	if (CooldownHandle.IsValid())
	{
		if (ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner()))
		{
			if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(CooldownHandle);
			}
		}
	}
	if (ClientOperationId > 0)
	{
		PendingComplexConsumableOperations.Remove(ClientOperationId);
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, OutFailureReason, Revision);
	}
	return false;
}

void UDOInventoryComponent::CancelConsumableUse(const int32 ClientOperationId)
{
	if (ClientOperationId > 0 && PendingComplexConsumableOperations.Remove(ClientOperationId) > 0)
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Cancelled, EDOInventoryFailureReason::None, Revision);
	}
}

bool UDOInventoryComponent::TryUseItemByInstanceId(const FGuid& InstanceId, EDOInventoryFailureReason& OutFailureReason, const int32 ClientOperationId, bool* bOutDeferredCompletion)
{
	if (bOutDeferredCompletion)
	{
		*bOutDeferredCompletion = false;
	}
	const FDOItemInstanceRecord* Item = FindItemByInstanceId(InstanceId);
	if (!Item)
	{
		OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
		return false;
	}
	return TryUseItemInternal(*Item, OutFailureReason, ClientOperationId, bOutDeferredCompletion);
}

bool UDOInventoryComponent::TryUseItemByDefinition(const FPrimaryAssetId& DefinitionId, EDOInventoryFailureReason& OutFailureReason, const int32 ClientOperationId, bool* bOutDeferredCompletion)
{
	if (bOutDeferredCompletion)
	{
		*bOutDeferredCompletion = false;
	}
	const FDOInventoryEntry* Candidate = nullptr;
	for (const FDOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Item.DefinitionId == DefinitionId && (!Candidate || Entry.Item.SlotIndex < Candidate->Item.SlotIndex))
		{
			Candidate = &Entry;
		}
	}

	if (Candidate)
	{
		return TryUseItemInternal(Candidate->Item, OutFailureReason, ClientOperationId, bOutDeferredCompletion);
	}

	OutFailureReason = EDOInventoryFailureReason::ItemNotFound;
	return false;
}

void UDOInventoryComponent::HandleFastArrayChanged(const TArray<FGuid>& ChangedInstanceIds)
{
	InventoryList.OwnerComponent = this;
	BroadcastChanged(ChangedInstanceIds, false);
}

void UDOInventoryComponent::BeginMutation()
{
	if (IsServerComponent(this))
	{
		++MutationDepth;
	}
}

void UDOInventoryComponent::EndMutation()
{
	if (!IsServerComponent(this) || MutationDepth <= 0)
	{
		return;
	}

	--MutationDepth;
	if (MutationDepth == 0 && DeferredChangedInstanceIds.Num() > 0)
	{
		TArray<FGuid> ChangedInstanceIds = MoveTemp(DeferredChangedInstanceIds);
		const bool bAdvanceRevision = bDeferredMutationAdvancesRevision;
		bDeferredMutationAdvancesRevision = false;
		BroadcastChanged(ChangedInstanceIds, bAdvanceRevision);
	}
}

void UDOInventoryComponent::BroadcastChanged(const TArray<FGuid>& ChangedInstanceIds, const bool bAdvanceRevision)
{
	if (MutationDepth > 0)
	{
		for (const FGuid& ChangedInstanceId : ChangedInstanceIds)
		{
			DeferredChangedInstanceIds.AddUnique(ChangedInstanceId);
		}
		bDeferredMutationAdvancesRevision |= bAdvanceRevision;
		return;
	}

	if (bAdvanceRevision && GetOwner() && GetOwner()->HasAuthority())
	{
		++Revision;
	}
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOInventoryChangedMessage Message;
	Message.InventoryComponent = this;
	Message.ChangedInstanceIds = ChangedInstanceIds;
	Message.Revision = Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Inventory::Changed, Message);
}

void UDOInventoryComponent::BroadcastOperationFailure(const int32 ClientOperationId, const EDOInventoryFailureReason FailureReason)
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOInventoryOperationFailedMessage Message;
	Message.InventoryComponent = this;
	Message.ClientOperationId = ClientOperationId;
	Message.FailureReason = FailureReason;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Inventory::OperationFailed, Message);
}

void UDOInventoryComponent::RequestMoveItem(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot, const int32 RequestedCount, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestMoveItem_Implementation(InstanceId, SourceSlot, TargetSlot, RequestedCount, ClientOperationId);
	}
	else
	{
		Server_RequestMoveItem(InstanceId, SourceSlot, TargetSlot, RequestedCount, ClientOperationId);
	}
}

void UDOInventoryComponent::RequestSplitStack(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot, const int32 SplitCount, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestSplitStack_Implementation(InstanceId, SourceSlot, TargetSlot, SplitCount, ClientOperationId);
	}
	else
	{
		Server_RequestSplitStack(InstanceId, SourceSlot, TargetSlot, SplitCount, ClientOperationId);
	}
}

void UDOInventoryComponent::RequestSortInventory(const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestSortInventory_Implementation(ClientOperationId);
	}
	else
	{
		Server_RequestSortInventory(ClientOperationId);
	}
}

void UDOInventoryComponent::RequestDiscardItem(const FGuid& InstanceId, const int32 Count, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestDiscardItem_Implementation(InstanceId, Count, ClientOperationId);
	}
	else
	{
		Server_RequestDiscardItem(InstanceId, Count, ClientOperationId);
	}
}

void UDOInventoryComponent::RequestUseItem(const FGuid& InstanceId, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestUseItem_Implementation(InstanceId, ClientOperationId);
	}
	else
	{
		Server_RequestUseItem(InstanceId, ClientOperationId);
	}
}

void UDOInventoryComponent::Server_RequestMoveItem_Implementation(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot, const int32 RequestedCount, const int32 ClientOperationId)
{
	TArray<FGuid> ChangedIds;
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	const bool bSuccess = TryMoveItemInternal(InstanceId, SourceSlot, TargetSlot, RequestedCount, FailureReason, ChangedIds);
	if (bSuccess)
	{
		if (ChangedIds.Num() > 0)
		{
			BroadcastChanged(ChangedIds);
		}
		Client_InventoryOperationResultEx(ClientOperationId, ChangedIds.Num() > 0 ? EDOItemOperationOutcome::Success : EDOItemOperationOutcome::NoOp, EDOInventoryFailureReason::None, Revision);
	}
	else
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, FailureReason, Revision);
	}
}

void UDOInventoryComponent::Server_RequestSplitStack_Implementation(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot, const int32 SplitCount, const int32 ClientOperationId)
{
	TArray<FGuid> ChangedIds;
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	const bool bSuccess = TrySplitStackInternal(InstanceId, SourceSlot, TargetSlot, SplitCount, FailureReason, ChangedIds);
	if (bSuccess)
	{
		BroadcastChanged(ChangedIds);
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
	}
	else
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, FailureReason, Revision);
	}
}

void UDOInventoryComponent::Server_RequestSortInventory_Implementation(const int32 ClientOperationId)
{
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	const int32 RevisionBefore = Revision;
	if (!TrySortInventory(FailureReason))
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, FailureReason, Revision);
		return;
	}
	Client_InventoryOperationResultEx(ClientOperationId, Revision != RevisionBefore ? EDOItemOperationOutcome::Success : EDOItemOperationOutcome::NoOp, EDOInventoryFailureReason::None, Revision);
}

void UDOInventoryComponent::Server_RequestDiscardItem_Implementation(const FGuid& InstanceId, const int32 Count, const int32 ClientOperationId)
{
	const FDOItemInstanceRecord* Item = FindItemByInstanceId(InstanceId);
	const UDOItemDefinition* Definition = Item ? ResolveItemDefinition(Item->DefinitionId) : nullptr;
	const UDOItemFragment_Inventory* InventoryFragment = Definition ? Definition->FindFragment<UDOItemFragment_Inventory>() : nullptr;
	if (!Item || !Definition)
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, EDOInventoryFailureReason::ItemNotFound, Revision);
		return;
	}
	if (InventoryFragment && !InventoryFragment->bCanDiscard)
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, EDOInventoryFailureReason::NotAllowed, Revision);
		return;
	}

	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	if (!TryConsumeItem(InstanceId, Count, FailureReason))
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, FailureReason, Revision);
		return;
	}
	Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
}

void UDOInventoryComponent::Server_RequestUseItem_Implementation(const FGuid& InstanceId, const int32 ClientOperationId)
{
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	const FDOItemInstanceRecord* Item = FindItemByInstanceId(InstanceId);
	bool bDeferredCompletion = false;
	if (!Item)
	{
		FailureReason = EDOInventoryFailureReason::ItemNotFound;
	}
	if (!Item || !TryUseItemInternal(*Item, FailureReason, ClientOperationId, &bDeferredCompletion))
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Failure, FailureReason, Revision);
		return;
	}
	if (!bDeferredCompletion)
	{
		Client_InventoryOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
	}
}

void UDOInventoryComponent::Client_InventoryOperationResult_Implementation(const int32 ClientOperationId, const bool bSuccess, const EDOInventoryFailureReason FailureReason)
{
	if (!bSuccess)
	{
		BroadcastOperationFailure(ClientOperationId, FailureReason);
	}
	BroadcastOperationResult(ClientOperationId, bSuccess ? EDOItemOperationOutcome::Success : EDOItemOperationOutcome::Failure, FailureReason, Revision);
}

void UDOInventoryComponent::Client_InventoryOperationResultEx_Implementation(const int32 ClientOperationId, const EDOItemOperationOutcome Outcome, const EDOInventoryFailureReason FailureReason, const int32 AuthoritativeRevision)
{
	if (Outcome == EDOItemOperationOutcome::Failure)
	{
		BroadcastOperationFailure(ClientOperationId, FailureReason);
	}
	BroadcastOperationResult(ClientOperationId, Outcome, FailureReason, AuthoritativeRevision);

	if (AuthoritativeRevision > Revision && GetOwner() && GetOwner()->HasAuthority())
	{
		Revision = AuthoritativeRevision;
	}
}

void UDOInventoryComponent::BroadcastOperationResult(const int32 ClientOperationId, const EDOItemOperationOutcome Outcome, const EDOInventoryFailureReason FailureReason, const int32 AuthoritativeRevision)
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOInventoryOperationResultMessage Message;
	Message.InventoryComponent = this;
	Message.Result.Domain = EDOItemOperationDomain::Inventory;
	Message.Result.Outcome = Outcome;
	Message.Result.ClientOperationId = ClientOperationId;
	Message.Result.FailureReason = FailureReason;
	Message.Result.AuthoritativeRevision = AuthoritativeRevision >= 0 ? AuthoritativeRevision : Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Inventory::OperationResult, Message);
}
