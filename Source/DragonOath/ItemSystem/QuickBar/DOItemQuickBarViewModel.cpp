#include "ItemSystem/QuickBar/DOItemQuickBarViewModel.h"

#include "Engine/AssetManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "Player/DOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemQuickBarViewModel)

void UDOItemQuickBarViewModel::Initialize(ADOPlayerState* InPlayerState)
{
	Shutdown();
	PlayerState = InPlayerState;
	InventoryComponent = InPlayerState ? InPlayerState->GetInventoryComponent() : nullptr;
	QuickBarComponent = InPlayerState ? InPlayerState->GetItemQuickBarComponent() : nullptr;

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		InventoryChangedHandle = MessageSubsystem.RegisterListener<FDOInventoryChangedMessage>(
			DragonOathGameplayTags::Message::UI::Inventory::Changed,
			this,
			&UDOItemQuickBarViewModel::HandleInventoryChanged);
		QuickBarChangedHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarChangedMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::Changed,
			this,
			&UDOItemQuickBarViewModel::HandleQuickBarChanged);
		OperationFailedHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarOperationFailedMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationFailed,
			this,
			&UDOItemQuickBarViewModel::HandleOperationFailed);
	}

	Refresh();
}

void UDOItemQuickBarViewModel::Shutdown()
{
	InventoryChangedHandle.Unregister();
	QuickBarChangedHandle.Unregister();
	OperationFailedHandle.Unregister();
	Slots.Reset();
	PendingOperations.Reset();
	PlayerState.Reset();
	InventoryComponent.Reset();
	QuickBarComponent.Reset();
}

void UDOItemQuickBarViewModel::Refresh()
{
	Slots.Reset();

	TArray<FDOItemInstanceRecord> Items;
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->GetInventorySnapshot(Items);
	}

	for (int32 SlotIndex = 0; SlotIndex < UDOItemQuickBarComponent::QuickBarSlotCount; ++SlotIndex)
	{
		TSharedPtr<FDOQuickBarSlotViewModel> Slot = MakeShared<FDOQuickBarSlotViewModel>();
		if (QuickBarComponent.IsValid())
		{
			Slot->DefinitionId = QuickBarComponent->GetDefinitionForSlot(SlotIndex);
			Slot->bIsBound = Slot->DefinitionId.IsValid();
		}

		if (Slot->bIsBound)
		{
			if (const UDOItemDefinition* Definition = ResolveItemDefinition(Slot->DefinitionId))
			{
				Slot->DisplayName = Definition->DisplayName;
				Slot->Description = Definition->Description;
				Slot->Icon = Definition->Icon;
			}

			for (const FDOItemInstanceRecord& Item : Items)
			{
				if (Item.DefinitionId == Slot->DefinitionId)
				{
					Slot->StackCount += Item.StackCount;
				}
			}
		}
		for (const TPair<int32, int32>& PendingPair : PendingOperations)
		{
			if (PendingPair.Value == SlotIndex)
			{
				Slot->bIsPending = true;
				break;
			}
		}

		Slot->bIsEmpty = !Slot->bIsBound;
		Slots.Add(Slot);
	}

	BroadcastChanged();
}

void UDOItemQuickBarViewModel::RequestUseSlot(const int32 SlotIndex)
{
	if (QuickBarComponent.IsValid()
		&& Slots.IsValidIndex(SlotIndex)
		&& Slots[SlotIndex].IsValid()
		&& Slots[SlotIndex]->bIsBound
		&& Slots[SlotIndex]->StackCount > 0
		&& !Slots[SlotIndex]->bIsPending)
	{
		const int32 OperationId = NextClientOperationId++;
		PendingOperations.Add(OperationId, SlotIndex);
		BroadcastChanged();
		QuickBarComponent->RequestUseSlot(SlotIndex, OperationId);
	}
}

void UDOItemQuickBarViewModel::BeginDestroy()
{
	Shutdown();
	Super::BeginDestroy();
}

void UDOItemQuickBarViewModel::HandleInventoryChanged(FGameplayTag /*Channel*/, const FDOInventoryChangedMessage& Message)
{
	if (Message.InventoryComponent == InventoryComponent.Get())
	{
		PendingOperations.Reset();
		Refresh();
	}
}

void UDOItemQuickBarViewModel::HandleQuickBarChanged(FGameplayTag /*Channel*/, const FDOItemQuickBarChangedMessage& Message)
{
	if (Message.QuickBarComponent == QuickBarComponent.Get())
	{
		PendingOperations.Reset();
		Refresh();
	}
}

void UDOItemQuickBarViewModel::HandleOperationFailed(FGameplayTag /*Channel*/, const FDOItemQuickBarOperationFailedMessage& Message)
{
	if (Message.QuickBarComponent == QuickBarComponent.Get())
	{
		PendingOperations.Remove(Message.ClientOperationId);
		Refresh();
	}
}

const UDOItemDefinition* UDOItemQuickBarViewModel::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
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

void UDOItemQuickBarViewModel::BroadcastChanged()
{
	ChangedDelegate.Broadcast();
}
