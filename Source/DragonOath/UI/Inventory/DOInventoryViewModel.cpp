#include "UI/Inventory/DOInventoryViewModel.h"

#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "Player/DOPlayerCharacter.h"
#include "Player/DOPlayerState.h"
#include "UI/Inventory/DOCombatRatingConfig.h"
#include "UI/Inventory/DOInventoryPreviewComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOInventoryViewModel)

void UDOInventoryViewModel::Initialize(ADOPlayerState* InPlayerState)
{
	Shutdown();
	PlayerState = InPlayerState;
	InventoryComponent = InPlayerState ? InPlayerState->GetInventoryComponent() : nullptr;
	EquipmentComponent = InPlayerState ? InPlayerState->GetEquipmentComponent() : nullptr;
	AbilitySystemComponent = InPlayerState ? InPlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (InPlayerState)
	{
		if (ADOPlayerCharacter* PlayerCharacter = Cast<ADOPlayerCharacter>(InPlayerState->GetPawn()))
		{
			PreviewComponent = PlayerCharacter->GetInventoryPreviewComponent();
			if (PreviewComponent.IsValid())
			{
				PreviewComponent->ActivatePreview();
			}
		}
	}

	Categories = {
		{ FGameplayTag(), FText::FromString(TEXT("全部")) },
		{ DragonOathGameplayTags::Item::Category::Weapon, FText::FromString(TEXT("武器")) },
		{ DragonOathGameplayTags::Item::Category::Armor, FText::FromString(TEXT("防具")) },
		{ DragonOathGameplayTags::Item::Category::Accessory, FText::FromString(TEXT("饰品")) },
		{ DragonOathGameplayTags::Equipment::Slot::Head, FText::FromString(TEXT("头部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Shoulder, FText::FromString(TEXT("肩部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Back, FText::FromString(TEXT("背部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Chest, FText::FromString(TEXT("胸部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Hands, FText::FromString(TEXT("手部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Legs, FText::FromString(TEXT("腿部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Feet, FText::FromString(TEXT("脚部")) },
		{ DragonOathGameplayTags::Equipment::Slot::Accessory, FText::FromString(TEXT("饰品装备")) },
		{ DragonOathGameplayTags::Equipment::Slot::Weapon, FText::FromString(TEXT("武器装备")) },
		{ DragonOathGameplayTags::Item::Type::Consumable, FText::FromString(TEXT("消耗品")) },
		{ DragonOathGameplayTags::Item::Type::Material, FText::FromString(TEXT("材料")) },
		{ DragonOathGameplayTags::Item::Type::Quest, FText::FromString(TEXT("任务物品")) }
	};

	if (InPlayerState && UGameplayMessageSubsystem::HasInstance(this))
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		InventoryChangedHandle = MessageSubsystem.RegisterListener<FDOInventoryChangedMessage>(
			DragonOathGameplayTags::Message::UI::Inventory::Changed,
			this,
			&UDOInventoryViewModel::HandleInventoryChanged);
		EquipmentChangedHandle = MessageSubsystem.RegisterListener<FDOEquipmentChangedMessage>(
			DragonOathGameplayTags::Message::UI::Equipment::Changed,
			this,
			&UDOInventoryViewModel::HandleEquipmentChanged);
		OperationFailedHandle = MessageSubsystem.RegisterListener<FDOInventoryOperationFailedMessage>(
			DragonOathGameplayTags::Message::UI::Inventory::OperationFailed,
			this,
			&UDOInventoryViewModel::HandleOperationFailed);
		OperationResultHandle = MessageSubsystem.RegisterListener<FDOInventoryOperationResultMessage>(
			DragonOathGameplayTags::Message::UI::Inventory::OperationResult,
			this,
			&UDOInventoryViewModel::HandleOperationResult);
		QuickBarChangedHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarChangedMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::Changed,
			this,
			&UDOInventoryViewModel::HandleQuickBarChanged);
		EquipmentOperationFailedHandle = MessageSubsystem.RegisterListener<FDOEquipmentOperationFailedMessage>(
			DragonOathGameplayTags::Message::UI::Equipment::OperationFailed,
			this,
			&UDOInventoryViewModel::HandleEquipmentOperationFailed);
		EquipmentOperationResultHandle = MessageSubsystem.RegisterListener<FDOEquipmentOperationResultMessage>(
			DragonOathGameplayTags::Message::UI::Equipment::OperationResult,
			this,
			&UDOInventoryViewModel::HandleEquipmentOperationResult);
		QuickBarOperationFailedHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarOperationFailedMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationFailed,
			this,
			&UDOInventoryViewModel::HandleQuickBarOperationFailed);
		QuickBarOperationResultHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarOperationResultMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationResult,
			this,
			&UDOInventoryViewModel::HandleQuickBarOperationResult);
	}

	if (InPlayerState && InPlayerState->GetWorld())
	{
		InPlayerState->GetWorld()->GetTimerManager().SetTimer(
			PendingTimeoutTimerHandle,
			FTimerDelegate::CreateUObject(this, &UDOInventoryViewModel::ProcessPendingTimeouts),
			0.5f,
			true);
	}

	RegisterAttributeListeners();
	Refresh();
}

void UDOInventoryViewModel::Shutdown()
{
	InventoryChangedHandle.Unregister();
	EquipmentChangedHandle.Unregister();
	OperationFailedHandle.Unregister();
	OperationResultHandle.Unregister();
	QuickBarChangedHandle.Unregister();
	EquipmentOperationFailedHandle.Unregister();
	EquipmentOperationResultHandle.Unregister();
	QuickBarOperationFailedHandle.Unregister();
	QuickBarOperationResultHandle.Unregister();
	if (PlayerState.IsValid() && PlayerState->GetWorld())
	{
		PlayerState->GetWorld()->GetTimerManager().ClearTimer(PendingTimeoutTimerHandle);
	}
	UnregisterAttributeListeners();
	PendingOperations.Reset();
	SlotViewModelCache.Reset();
	EquipmentSlotViewModelCache.Reset();
	EquipmentSlots.Reset();
	PlayerState.Reset();
	InventoryComponent.Reset();
	EquipmentComponent.Reset();
	AbilitySystemComponent.Reset();
	if (PreviewComponent.IsValid())
	{
		PreviewComponent->DeactivatePreview();
	}
	PreviewComponent.Reset();
}

UTextureRenderTarget2D* UDOInventoryViewModel::GetPreviewRenderTarget() const
{
	return PreviewComponent.IsValid() ? PreviewComponent->GetRenderTarget() : nullptr;
}

void UDOInventoryViewModel::BeginDestroy()
{
	Shutdown();
	Super::BeginDestroy();
}

const UDOItemDefinition* UDOInventoryViewModel::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
{
	return UDOItemDefinitionSubsystem::ResolveItemDefinition(this, DefinitionId);
}

bool UDOInventoryViewModel::MatchesFilter(const FDOItemInstanceRecord& Item, const UDOItemDefinition* Definition) const
{
	if (!CurrentFilter.IsValid())
	{
		return true;
	}
	if (!Definition)
	{
		return false;
	}
	if (Definition->ItemType == CurrentFilter || Definition->ItemTags.HasTagExact(CurrentFilter) || Definition->ItemTags.HasTag(CurrentFilter))
	{
		return true;
	}
	if (const UDOItemFragment_Equipment* EquipmentFragment = Definition->FindFragment<UDOItemFragment_Equipment>())
	{
		return EquipmentFragment->EquipmentSlotTag == CurrentFilter;
	}
	return false;
}

void UDOInventoryViewModel::Refresh()
{
	RefreshAttributeSnapshot();
	RebuildVisibleSlots();
	RebuildEquipmentSlots();
	BroadcastChanged();
}

void UDOInventoryViewModel::RebuildVisibleSlots()
{
	TArray<FGuid> PreviousInstanceIds;
	TArray<int32> PreviousSlotIndices;
	TArray<bool> PreviousEmptyStates;
	PreviousInstanceIds.Reserve(VisibleSlots.Num());
	PreviousSlotIndices.Reserve(VisibleSlots.Num());
	PreviousEmptyStates.Reserve(VisibleSlots.Num());
	for (const TSharedPtr<FDOInventorySlotViewModel>& PreviousSlot : VisibleSlots)
	{
		PreviousInstanceIds.Add(PreviousSlot.IsValid() ? PreviousSlot->GetInstanceId() : FGuid());
		PreviousSlotIndices.Add(PreviousSlot.IsValid() ? PreviousSlot->GetSlotIndex() : INDEX_NONE);
		PreviousEmptyStates.Add(!PreviousSlot.IsValid() || PreviousSlot->bIsEmpty);
	}

	VisibleSlots.Reset();
	TArray<FDOItemInstanceRecord> Items;
	TSet<FGuid> EquippedInstanceIds;
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->GetInventorySnapshot(Items);
	}
	if (EquipmentComponent.IsValid())
	{
		TArray<FDOEquippedItemEntry> EquippedItems;
		EquipmentComponent->GetEquippedSnapshot(EquippedItems);
		for (const FDOEquippedItemEntry& EquippedItem : EquippedItems)
		{
			EquippedInstanceIds.Add(EquippedItem.Item.InstanceId);
		}
	}

	TSet<FGuid> CurrentInstanceIds;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		CurrentInstanceIds.Add(Item.InstanceId);
	}
	for (auto It = SlotViewModelCache.CreateIterator(); It; ++It)
	{
		if (!CurrentInstanceIds.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}

	TArray<TSharedPtr<FDOInventorySlotViewModel>> FilteredSlots;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
		if (!MatchesFilter(Item, Definition))
		{
			continue;
		}

		TSharedPtr<FDOInventorySlotViewModel>& CachedSlot = SlotViewModelCache.FindOrAdd(Item.InstanceId);
		if (!CachedSlot.IsValid())
		{
			CachedSlot = MakeShared<FDOInventorySlotViewModel>();
		}
		TSharedPtr<FDOInventorySlotViewModel> Slot = CachedSlot;
		*Slot = FDOInventorySlotViewModel();
		Slot->Item = Item;
		Slot->bIsEmpty = false;
		Slot->bIsEquipped = EquippedInstanceIds.Contains(Item.InstanceId);
		for (const TPair<int32, FDOInventoryPendingOperation>& PendingPair : PendingOperations)
		{
			if (PendingPair.Value.InstanceId == Item.InstanceId)
			{
				Slot->bIsPending = true;
				break;
			}
		}
		if (Definition)
		{
			Slot->DisplayName = Definition->DisplayName;
			Slot->Description = Definition->Description;
			Slot->Icon = Definition->Icon;
			Slot->ItemType = Definition->ItemType;
			Slot->Rarity = Definition->Rarity;
			Slot->bIsUsable = Definition->FindFragment<UDOItemFragment_Consumable>() != nullptr;
			if (const UDOItemFragment_Inventory* InventoryFragment = Definition->FindFragment<UDOItemFragment_Inventory>())
			{
				Slot->bCanDiscard = InventoryFragment->bCanDiscard;
			}
			if (const UDOItemFragment_Equipment* EquipmentFragment = Definition->FindFragment<UDOItemFragment_Equipment>())
			{
				Slot->EquipmentSlotTag = EquipmentFragment->EquipmentSlotTag;
			}
		}
		FilteredSlots.Add(Slot);
	}

	if (SelectedInstanceId.IsValid() && !FilteredSlots.ContainsByPredicate([this](const TSharedPtr<FDOInventorySlotViewModel>& Slot)
	{
		return Slot.IsValid() && Slot->GetInstanceId() == SelectedInstanceId;
	}))
	{
		SelectedInstanceId.Invalidate();
	}

	PageCount = FMath::Max(1, FMath::DivideAndRoundUp(FilteredSlots.Num(), PageSize));
	CurrentPage = FMath::Clamp(CurrentPage, 0, PageCount - 1);
	const int32 StartIndex = CurrentPage * PageSize;
	for (int32 LocalIndex = 0; LocalIndex < PageSize; ++LocalIndex)
	{
		const int32 ItemIndex = StartIndex + LocalIndex;
		if (FilteredSlots.IsValidIndex(ItemIndex))
		{
			VisibleSlots.Add(FilteredSlots[ItemIndex]);
		}
		else
		{
			TSharedPtr<FDOInventorySlotViewModel> EmptySlot = MakeShared<FDOInventorySlotViewModel>();
			// 只有“全部”分类能可靠映射到真实物理槽位；筛选分类的空白格不参与拖放。
			if (!CurrentFilter.IsValid())
			{
				EmptySlot->Item.SlotIndex = ItemIndex;
			}
			VisibleSlots.Add(EmptySlot);
		}
	}

	// 翻页或筛选后，隐藏页中的旧选中项不能继续驱动底部操作栏。
	if (SelectedInstanceId.IsValid() && !VisibleSlots.ContainsByPredicate([this](const TSharedPtr<FDOInventorySlotViewModel>& Slot)
	{
		return Slot.IsValid() && !Slot->bIsEmpty && Slot->GetInstanceId() == SelectedInstanceId;
	}))
	{
		SelectedInstanceId.Invalidate();
	}

	bVisibleSlotsStructureChanged = PreviousInstanceIds.Num() != VisibleSlots.Num();
	if (!bVisibleSlotsStructureChanged)
	{
		for (int32 Index = 0; Index < VisibleSlots.Num(); ++Index)
		{
			const TSharedPtr<FDOInventorySlotViewModel>& CurrentSlot = VisibleSlots[Index];
			const FGuid CurrentInstanceId = CurrentSlot.IsValid() ? CurrentSlot->GetInstanceId() : FGuid();
			const int32 CurrentSlotIndex = CurrentSlot.IsValid() ? CurrentSlot->GetSlotIndex() : INDEX_NONE;
			const bool bCurrentEmpty = !CurrentSlot.IsValid() || CurrentSlot->bIsEmpty;
			if (PreviousInstanceIds[Index] != CurrentInstanceId
				|| PreviousSlotIndices[Index] != CurrentSlotIndex
				|| PreviousEmptyStates[Index] != bCurrentEmpty)
			{
				bVisibleSlotsStructureChanged = true;
				break;
			}
		}
	}
}

void UDOInventoryViewModel::RebuildEquipmentSlots()
{
	TArray<FDOEquippedItemEntry> EquippedItems;
	if (EquipmentComponent.IsValid())
	{
		EquipmentComponent->GetEquippedSnapshot(EquippedItems);
	}

	TSet<FGameplayTag> CurrentSlotTags;
	TArray<FGameplayTag> SupportedSlotTags;
	if (EquipmentComponent.IsValid())
	{
		EquipmentComponent->GetSupportedSlotTags(SupportedSlotTags);
	}
	for (const FGameplayTag& SlotTag : SupportedSlotTags)
	{
		CurrentSlotTags.Add(SlotTag);
		TSharedPtr<FDOEquipmentSlotViewModel>& CachedSlot = EquipmentSlotViewModelCache.FindOrAdd(SlotTag);
		if (!CachedSlot.IsValid())
		{
			CachedSlot = MakeShared<FDOEquipmentSlotViewModel>();
		}
		*CachedSlot = FDOEquipmentSlotViewModel();
		CachedSlot->SlotTag = SlotTag;
		CachedSlot->DisplayName = FText::FromName(SlotTag.GetTagName());
		CachedSlot->ItemDisplayName = FText::GetEmpty();
		CachedSlot->bIsEmpty = true;
		CachedSlot->bIsPending = false;
	}
	for (const FDOEquippedItemEntry& Entry : EquippedItems)
	{
		CurrentSlotTags.Add(Entry.SlotTag);
		TSharedPtr<FDOEquipmentSlotViewModel>& CachedSlot = EquipmentSlotViewModelCache.FindOrAdd(Entry.SlotTag);
		if (!CachedSlot.IsValid())
		{
			CachedSlot = MakeShared<FDOEquipmentSlotViewModel>();
		}
		*CachedSlot = FDOEquipmentSlotViewModel();
		CachedSlot->SlotTag = Entry.SlotTag;
		CachedSlot->DisplayName = FText::FromName(Entry.SlotTag.GetTagName());
		CachedSlot->Item = Entry.Item;
		CachedSlot->bIsEmpty = false;
		if (const UDOItemDefinition* Definition = ResolveItemDefinition(Entry.Item.DefinitionId))
		{
			CachedSlot->ItemDisplayName = Definition->DisplayName;
			CachedSlot->Icon = Definition->Icon;
			CachedSlot->Rarity = Definition->Rarity;
		}
		for (const TPair<int32, FDOInventoryPendingOperation>& PendingPair : PendingOperations)
		{
			if (PendingPair.Value.Domain == EDOInventoryPendingDomain::Equipment && PendingPair.Value.SlotTag == Entry.SlotTag)
			{
				CachedSlot->bIsPending = true;
				break;
			}
		}
	}
	for (auto It = EquipmentSlotViewModelCache.CreateIterator(); It; ++It)
	{
		if (!CurrentSlotTags.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}

	EquipmentSlots.Reset();
	for (const TPair<FGameplayTag, TSharedPtr<FDOEquipmentSlotViewModel>>& Pair : EquipmentSlotViewModelCache)
	{
		EquipmentSlots.Add(Pair.Value);
	}
	EquipmentSlots.Sort([](const TSharedPtr<FDOEquipmentSlotViewModel>& A, const TSharedPtr<FDOEquipmentSlotViewModel>& B)
	{
		return A.IsValid() && B.IsValid() && A->SlotTag.ToString() < B->SlotTag.ToString();
	});
}

const FDOEquipmentSlotViewModel* UDOInventoryViewModel::FindEquipmentSlot(const FGameplayTag& SlotTag) const
{
	if (const TSharedPtr<FDOEquipmentSlotViewModel>* Slot = EquipmentSlotViewModelCache.Find(SlotTag))
	{
		return Slot->Get();
	}
	return nullptr;
}

TSharedPtr<FDOEquipmentSlotViewModel> UDOInventoryViewModel::GetOrCreateEquipmentSlot(const FGameplayTag& SlotTag)
{
	TSharedPtr<FDOEquipmentSlotViewModel>& Slot = EquipmentSlotViewModelCache.FindOrAdd(SlotTag);
	if (!Slot.IsValid())
	{
		Slot = MakeShared<FDOEquipmentSlotViewModel>();
		Slot->SlotTag = SlotTag;
	}
	return Slot;
}

void UDOInventoryViewModel::SetFilter(const FGameplayTag NewFilter)
{
	CurrentFilter = NewFilter;
	CurrentPage = 0;
	Refresh();
}

void UDOInventoryViewModel::SetPage(const int32 NewPage)
{
	CurrentPage = FMath::Clamp(NewPage, 0, PageCount - 1);
	Refresh();
}

void UDOInventoryViewModel::SelectInstance(const FGuid& InstanceId)
{
	SelectedInstanceId = InstanceId;
	BroadcastChanged();
}

const FDOInventorySlotViewModel* UDOInventoryViewModel::FindVisibleSlot(const FGuid& InstanceId) const
{
	for (const TSharedPtr<FDOInventorySlotViewModel>& Slot : VisibleSlots)
	{
		if (Slot.IsValid() && Slot->GetInstanceId() == InstanceId)
		{
			return Slot.Get();
		}
	}
	return nullptr;
}

void UDOInventoryViewModel::RequestMoveOrEquip(const FGuid& InstanceId, const int32 SourceSlot, const int32 TargetSlot)
{
	if (InventoryComponent.IsValid())
	{
		const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Inventory, InstanceId);
		InventoryComponent->RequestMoveItem(InstanceId, SourceSlot, TargetSlot, 0, OperationId);
	}
}

void UDOInventoryViewModel::RequestSplitStack(const FGuid& InstanceId, const int32 TargetSlot, const int32 SplitCount)
{
	const FDOInventorySlotViewModel* Slot = FindVisibleSlot(InstanceId);
	if (InventoryComponent.IsValid() && Slot && !Slot->bIsEmpty && InstanceId.IsValid() && SplitCount > 0)
	{
		const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Inventory, InstanceId);
		InventoryComponent->RequestSplitStack(InstanceId, Slot->GetSlotIndex(), TargetSlot, SplitCount, OperationId);
	}
}

void UDOInventoryViewModel::RequestActivateSelected()
{
	if (InventoryComponent.IsValid() && SelectedInstanceId.IsValid())
	{
		const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId);
		if (Slot && Slot->bIsUsable)
		{
			const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Inventory, SelectedInstanceId);
			InventoryComponent->RequestUseItem(SelectedInstanceId, OperationId);
		}
	}
}

void UDOInventoryViewModel::RequestAssignSelectedToQuickBar(const int32 QuickBarSlot)
{
	if (!PlayerState.IsValid() || !SelectedInstanceId.IsValid())
	{
		return;
	}

	const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId);
	UDOItemQuickBarComponent* QuickBar = PlayerState->GetItemQuickBarComponent();
	if (Slot && !Slot->bIsEmpty && QuickBar)
	{
		const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::QuickBar, SelectedInstanceId);
		QuickBar->RequestAssignDefinition(QuickBarSlot, Slot->Item.DefinitionId, OperationId);
	}
}

void UDOInventoryViewModel::RequestEquipSelected()
{
	RequestEquipInstance(SelectedInstanceId);
}

void UDOInventoryViewModel::RequestEquipInstance(const FGuid& InstanceId)
{
	if (EquipmentComponent.IsValid() && InstanceId.IsValid())
	{
		const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Equipment, InstanceId);
		EquipmentComponent->RequestEquipItem(InstanceId, OperationId);
	}
}

void UDOInventoryViewModel::RequestUnequip(const FGameplayTag SlotTag)
{
	if (EquipmentComponent.IsValid())
	{
		const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Equipment, FGuid(), SlotTag);
		EquipmentComponent->RequestUnequipItem(SlotTag, OperationId);
	}
}

void UDOInventoryViewModel::RequestSort()
{
	if (InventoryComponent.IsValid())
	{
		const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Inventory);
		InventoryComponent->RequestSortInventory(OperationId);
	}
}

void UDOInventoryViewModel::RequestDiscardSelected()
{
	if (InventoryComponent.IsValid() && SelectedInstanceId.IsValid())
	{
		const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId);
		if (Slot && !Slot->bIsEmpty)
		{
			RequestDiscardSelectedCount(Slot->Item.StackCount);
		}
	}
}

void UDOInventoryViewModel::RequestDiscardSelectedCount(const int32 Count)
{
	if (InventoryComponent.IsValid() && SelectedInstanceId.IsValid() && Count > 0)
	{
		if (const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId))
		{
			if (!Slot->bIsEmpty)
			{
				const int32 OperationId = BeginPendingOperation(EDOInventoryPendingDomain::Inventory, SelectedInstanceId);
				InventoryComponent->RequestDiscardItem(SelectedInstanceId, FMath::Min(Count, Slot->Item.StackCount), OperationId);
			}
		}
	}
}

FText UDOInventoryViewModel::GetSelectedDisplayName() const
{
	const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId);
	return Slot ? Slot->DisplayName : FText::FromString(TEXT("未选择物品"));
}

FText UDOInventoryViewModel::GetSelectedDescription() const
{
	const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId);
	return Slot ? Slot->Description : FText::GetEmpty();
}

int32 UDOInventoryViewModel::GetSelectedStackCount() const
{
	const FDOInventorySlotViewModel* Slot = FindVisibleSlot(SelectedInstanceId);
	return Slot ? Slot->Item.StackCount : 0;
}

void UDOInventoryViewModel::HandleInventoryChanged(FGameplayTag /*Channel*/, const FDOInventoryChangedMessage& Message)
{
	if (Message.InventoryComponent == InventoryComponent.Get())
	{
		Refresh();
	}
}

void UDOInventoryViewModel::HandleEquipmentChanged(FGameplayTag /*Channel*/, const FDOEquipmentChangedMessage& Message)
{
	if (Message.EquipmentComponent == EquipmentComponent.Get())
	{
		Refresh();
	}
}

void UDOInventoryViewModel::HandleOperationFailed(FGameplayTag /*Channel*/, const FDOInventoryOperationFailedMessage& Message)
{
	if (Message.InventoryComponent == InventoryComponent.Get())
	{
		ClearPendingOperation(Message.ClientOperationId);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleOperationResult(FGameplayTag /*Channel*/, const FDOInventoryOperationResultMessage& Message)
{
	if (Message.InventoryComponent == InventoryComponent.Get())
	{
		ClearPendingOperation(Message.Result.ClientOperationId);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleQuickBarChanged(FGameplayTag /*Channel*/, const FDOItemQuickBarChangedMessage& Message)
{
	if (PlayerState.IsValid() && Message.QuickBarComponent == PlayerState->GetItemQuickBarComponent())
	{
		Refresh();
	}
}

void UDOInventoryViewModel::HandleEquipmentOperationResult(FGameplayTag /*Channel*/, const FDOEquipmentOperationResultMessage& Message)
{
	if (Message.EquipmentComponent == EquipmentComponent.Get())
	{
		ClearPendingOperation(Message.Result.ClientOperationId);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleEquipmentOperationFailed(FGameplayTag /*Channel*/, const FDOEquipmentOperationFailedMessage& Message)
{
	if (Message.EquipmentComponent == EquipmentComponent.Get())
	{
		ClearPendingOperation(Message.ClientOperationId);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleQuickBarOperationResult(FGameplayTag /*Channel*/, const FDOItemQuickBarOperationResultMessage& Message)
{
	if (PlayerState.IsValid() && Message.QuickBarComponent == PlayerState->GetItemQuickBarComponent())
	{
		ClearPendingOperation(Message.Result.ClientOperationId);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleQuickBarOperationFailed(FGameplayTag /*Channel*/, const FDOItemQuickBarOperationFailedMessage& Message)
{
	if (PlayerState.IsValid() && Message.QuickBarComponent == PlayerState->GetItemQuickBarComponent())
	{
		ClearPendingOperation(Message.ClientOperationId);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleAttributeChanged(const FOnAttributeChangeData& /*ChangeData*/)
{
	RefreshAttributeSnapshot();
	BroadcastChanged();
}

void UDOInventoryViewModel::RegisterAttributeListeners()
{
	UnregisterAttributeListeners();
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	const TArray<FGameplayAttribute> Attributes = {
		UDOCombatSet::GetAttackPowerAttribute(),
		UDOCombatSet::GetDefensePowerAttribute(),
		UDOHealthSet::GetMaxHealthAttribute(),
		UDOResourceSet::GetMaxManaAttribute(),
		UDOCombatSet::GetCriticalRatingAttribute(),
		UDOCombatSet::GetHitRatingAttribute(),
		UDOCombatSet::GetEvasionRatingAttribute(),
		UDOCombatSet::GetAttackSpeedAttribute(),
		UDOCombatSet::GetMoveSpeedAttribute(),
		UDOCombatSet::GetLifeStealRateAttribute()
	};

	for (const FGameplayAttribute& Attribute : Attributes)
	{
		FDelegateHandle Handle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(
			this,
			&UDOInventoryViewModel::HandleAttributeChanged);
		AttributeChangeHandles.Emplace(Attribute, Handle);
	}
}

void UDOInventoryViewModel::UnregisterAttributeListeners()
{
	if (AbilitySystemComponent.IsValid())
	{
		for (const TPair<FGameplayAttribute, FDelegateHandle>& Pair : AttributeChangeHandles)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Key).Remove(Pair.Value);
		}
	}
	AttributeChangeHandles.Reset();
}

void UDOInventoryViewModel::RefreshAttributeSnapshot()
{
	AttributeSnapshot = FDOInventoryAttributeSnapshot();
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	const UDOAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	AttributeSnapshot.AttackPower = ASC->GetNumericAttribute(UDOCombatSet::GetAttackPowerAttribute());
	AttributeSnapshot.DefensePower = ASC->GetNumericAttribute(UDOCombatSet::GetDefensePowerAttribute());
	AttributeSnapshot.MaxHealth = ASC->GetNumericAttribute(UDOHealthSet::GetMaxHealthAttribute());
	AttributeSnapshot.MaxMana = ASC->GetNumericAttribute(UDOResourceSet::GetMaxManaAttribute());
	AttributeSnapshot.CriticalRating = ASC->GetNumericAttribute(UDOCombatSet::GetCriticalRatingAttribute());
	AttributeSnapshot.HitRating = ASC->GetNumericAttribute(UDOCombatSet::GetHitRatingAttribute());
	AttributeSnapshot.EvasionRating = ASC->GetNumericAttribute(UDOCombatSet::GetEvasionRatingAttribute());
	AttributeSnapshot.AttackSpeed = ASC->GetNumericAttribute(UDOCombatSet::GetAttackSpeedAttribute());
	AttributeSnapshot.MoveSpeed = ASC->GetNumericAttribute(UDOCombatSet::GetMoveSpeedAttribute());
	AttributeSnapshot.LifeStealRate = ASC->GetNumericAttribute(UDOCombatSet::GetLifeStealRateAttribute());

	FDOCombatRatingInput RatingInput;
	RatingInput.AttackPower = AttributeSnapshot.AttackPower;
	RatingInput.DefensePower = AttributeSnapshot.DefensePower;
	RatingInput.MaxHealth = AttributeSnapshot.MaxHealth;
	RatingInput.MaxMana = AttributeSnapshot.MaxMana;
	RatingInput.CriticalRating = AttributeSnapshot.CriticalRating;
	RatingInput.HitRating = AttributeSnapshot.HitRating;
	RatingInput.EvasionRating = AttributeSnapshot.EvasionRating;
	RatingInput.AttackSpeed = AttributeSnapshot.AttackSpeed;
	RatingInput.MoveSpeed = AttributeSnapshot.MoveSpeed;
	RatingInput.LifeStealRate = AttributeSnapshot.LifeStealRate;
	AttributeSnapshot.CombatPower = UDOCombatRatingLibrary::CalculateCombatPower(RatingInput);
	AttributeSnapshot.GuardPower = UDOCombatRatingLibrary::CalculateGuardPower(RatingInput);
}

int32 UDOInventoryViewModel::BeginPendingOperation(const EDOInventoryPendingDomain Domain, const FGuid& InstanceId, const FGameplayTag& SlotTag)
{
	int32 OperationId = NextClientOperationId++;
	if (OperationId <= 0)
	{
		NextClientOperationId = 1;
		OperationId = NextClientOperationId++;
	}

	FDOInventoryPendingOperation& Operation = PendingOperations.Add(OperationId);
	Operation.Domain = Domain;
	Operation.InstanceId = InstanceId;
	Operation.SlotTag = SlotTag;
	Operation.CreatedAtSeconds = FPlatformTime::Seconds();
	BroadcastChanged();
	return OperationId;
}

void UDOInventoryViewModel::ClearPendingOperation(const int32 ClientOperationId)
{
	if (ClientOperationId > 0)
	{
		PendingOperations.Remove(ClientOperationId);
	}
}

void UDOInventoryViewModel::ClearPendingOperationsForDomain(const EDOInventoryPendingDomain Domain)
{
	for (auto It = PendingOperations.CreateIterator(); It; ++It)
	{
		if (It.Value().Domain == Domain)
		{
			It.RemoveCurrent();
		}
	}
}

void UDOInventoryViewModel::ProcessPendingTimeouts()
{
	const double Now = FPlatformTime::Seconds();
	bool bChanged = false;
	for (auto It = PendingOperations.CreateIterator(); It; ++It)
	{
		if (Now - It.Value().CreatedAtSeconds > 5.0)
		{
			It.RemoveCurrent();
			bChanged = true;
		}
	}
	if (bChanged)
	{
		Refresh();
	}
}

void UDOInventoryViewModel::BroadcastChanged()
{
	ChangedDelegate.Broadcast();
}
