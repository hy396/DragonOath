#include "UI/Inventory/DOInventoryViewModel.h"

#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "Player/DOPlayerCharacter.h"
#include "Player/DOPlayerState.h"
#include "UI/Inventory/DOCombatRatingConfig.h"
#include "UI/Inventory/DOInventoryPreviewComponent.h"

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
		QuickBarChangedHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarChangedMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::Changed,
			this,
			&UDOInventoryViewModel::HandleQuickBarChanged);
		EquipmentOperationFailedHandle = MessageSubsystem.RegisterListener<FDOEquipmentOperationFailedMessage>(
			DragonOathGameplayTags::Message::UI::Equipment::OperationFailed,
			this,
			&UDOInventoryViewModel::HandleEquipmentOperationFailed);
		QuickBarOperationFailedHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarOperationFailedMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationFailed,
			this,
			&UDOInventoryViewModel::HandleQuickBarOperationFailed);
	}

	RegisterAttributeListeners();
	Refresh();
}

void UDOInventoryViewModel::Shutdown()
{
	InventoryChangedHandle.Unregister();
	EquipmentChangedHandle.Unregister();
	OperationFailedHandle.Unregister();
	QuickBarChangedHandle.Unregister();
	EquipmentOperationFailedHandle.Unregister();
	QuickBarOperationFailedHandle.Unregister();
	UnregisterAttributeListeners();
	PendingOperations.Reset();
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
	if (!DefinitionId.IsValid())
	{
		return nullptr;
	}
	UAssetManager& AssetManager = UAssetManager::Get();
	if (const UDOItemDefinition* LoadedDefinition = AssetManager.GetPrimaryAssetObject<UDOItemDefinition>(DefinitionId))
	{
		return LoadedDefinition;
	}
	const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(DefinitionId);
	return AssetPath.IsValid() ? Cast<UDOItemDefinition>(AssetPath.TryLoad()) : nullptr;
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
	BroadcastChanged();
}

void UDOInventoryViewModel::RebuildVisibleSlots()
{
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

	TArray<TSharedPtr<FDOInventorySlotViewModel>> FilteredSlots;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
		if (!MatchesFilter(Item, Definition))
		{
			continue;
		}

		TSharedPtr<FDOInventorySlotViewModel> Slot = MakeShared<FDOInventorySlotViewModel>();
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
		ClearPendingOperationsForDomain(EDOInventoryPendingDomain::Inventory);
		Refresh();
	}
}

void UDOInventoryViewModel::HandleEquipmentChanged(FGameplayTag /*Channel*/, const FDOEquipmentChangedMessage& Message)
{
	if (Message.EquipmentComponent == EquipmentComponent.Get())
	{
		ClearPendingOperationsForDomain(EDOInventoryPendingDomain::Equipment);
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

void UDOInventoryViewModel::HandleQuickBarChanged(FGameplayTag /*Channel*/, const FDOItemQuickBarChangedMessage& Message)
{
	if (PlayerState.IsValid() && Message.QuickBarComponent == PlayerState->GetItemQuickBarComponent())
	{
		ClearPendingOperationsForDomain(EDOInventoryPendingDomain::QuickBar);
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

void UDOInventoryViewModel::BroadcastChanged()
{
	ChangedDelegate.Broadcast();
}
