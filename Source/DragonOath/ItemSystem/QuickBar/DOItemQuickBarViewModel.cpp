#include "ItemSystem/QuickBar/DOItemQuickBarViewModel.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "Player/DOPlayerState.h"
#include "TimerManager.h"

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
		OperationResultHandle = MessageSubsystem.RegisterListener<FDOItemQuickBarOperationResultMessage>(
			DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationResult,
			this,
			&UDOItemQuickBarViewModel::HandleOperationResult);
	}
	if (InPlayerState && InPlayerState->GetWorld())
	{
		InPlayerState->GetWorld()->GetTimerManager().SetTimer(
			PendingTimeoutTimerHandle,
			FTimerDelegate::CreateUObject(this, &UDOItemQuickBarViewModel::ProcessPendingTimeouts),
			0.5f,
			true);
	}

	Refresh();
}

void UDOItemQuickBarViewModel::Shutdown()
{
	InventoryChangedHandle.Unregister();
	QuickBarChangedHandle.Unregister();
	OperationFailedHandle.Unregister();
	OperationResultHandle.Unregister();
	if (PlayerState.IsValid() && PlayerState->GetWorld())
	{
		PlayerState->GetWorld()->GetTimerManager().ClearTimer(PendingTimeoutTimerHandle);
	}
	Slots.Reset();
	SlotViewModelCache.Reset();
	PendingOperations.Reset();
	PendingOperationTimes.Reset();
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
		TSharedPtr<FDOQuickBarSlotViewModel>& CachedSlot = SlotViewModelCache.FindOrAdd(SlotIndex);
		if (!CachedSlot.IsValid())
		{
			CachedSlot = MakeShared<FDOQuickBarSlotViewModel>();
		}
		TSharedPtr<FDOQuickBarSlotViewModel> Slot = CachedSlot;
		*Slot = FDOQuickBarSlotViewModel();
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
		PendingOperationTimes.Add(OperationId, FPlatformTime::Seconds());
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
		Refresh();
	}
}

void UDOItemQuickBarViewModel::HandleQuickBarChanged(FGameplayTag /*Channel*/, const FDOItemQuickBarChangedMessage& Message)
{
	if (Message.QuickBarComponent == QuickBarComponent.Get())
	{
		Refresh();
	}
}

void UDOItemQuickBarViewModel::HandleOperationFailed(FGameplayTag /*Channel*/, const FDOItemQuickBarOperationFailedMessage& Message)
{
	if (Message.QuickBarComponent == QuickBarComponent.Get())
	{
		PendingOperations.Remove(Message.ClientOperationId);
		PendingOperationTimes.Remove(Message.ClientOperationId);
		Refresh();
	}
}

void UDOItemQuickBarViewModel::HandleOperationResult(FGameplayTag /*Channel*/, const FDOItemQuickBarOperationResultMessage& Message)
{
	if (Message.QuickBarComponent == QuickBarComponent.Get())
	{
		PendingOperations.Remove(Message.Result.ClientOperationId);
		PendingOperationTimes.Remove(Message.Result.ClientOperationId);
		Refresh();
	}
}

const UDOItemDefinition* UDOItemQuickBarViewModel::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
{
	return UDOItemDefinitionSubsystem::ResolveItemDefinition(this, DefinitionId);
}

void UDOItemQuickBarViewModel::BroadcastChanged()
{
	ChangedDelegate.Broadcast();
}

void UDOItemQuickBarViewModel::ProcessPendingTimeouts()
{
	const double Now = FPlatformTime::Seconds();
	bool bChanged = false;
	for (auto It = PendingOperationTimes.CreateIterator(); It; ++It)
	{
		if (Now - It.Value() > 5.0)
		{
			PendingOperations.Remove(It.Key());
			It.RemoveCurrent();
			bChanged = true;
		}
	}
	if (bChanged)
	{
		Refresh();
	}
}
