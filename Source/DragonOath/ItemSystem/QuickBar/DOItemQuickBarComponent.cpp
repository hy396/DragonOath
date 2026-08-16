#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "Player/DOPlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemQuickBarComponent)

UDOItemQuickBarComponent::UDOItemQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	QuickBarDefinitions.SetNum(QuickBarSlotCount);
}

void UDOItemQuickBarComponent::BeginPlay()
{
	Super::BeginPlay();
	QuickBarDefinitions.SetNum(QuickBarSlotCount);
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		InventoryOperationResultHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FDOInventoryOperationResultMessage>(
			DragonOathGameplayTags::Message::UI::Inventory::OperationResult,
			this,
			&UDOItemQuickBarComponent::HandleInventoryOperationResult);
	}
}

void UDOItemQuickBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InventoryOperationResultHandle.Unregister();
	DelegatedUseOperationIds.Reset();
	Super::EndPlay(EndPlayReason);
}

void UDOItemQuickBarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UDOItemQuickBarComponent, Revision, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDOItemQuickBarComponent, QuickBarDefinitions, COND_OwnerOnly);
}

FPrimaryAssetId UDOItemQuickBarComponent::GetDefinitionForSlot(const int32 SlotIndex) const
{
	return QuickBarDefinitions.IsValidIndex(SlotIndex) ? QuickBarDefinitions[SlotIndex] : FPrimaryAssetId();
}

void UDOItemQuickBarComponent::GetQuickBarSnapshot(TArray<FPrimaryAssetId>& OutDefinitions) const
{
	OutDefinitions = QuickBarDefinitions;
}

bool UDOItemQuickBarComponent::RestoreQuickBarSnapshot(const TArray<FPrimaryAssetId>& Definitions)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Definitions.Num() != QuickBarSlotCount)
	{
		return false;
	}

	for (const FPrimaryAssetId& DefinitionId : Definitions)
	{
		if (!DefinitionId.IsValid())
		{
			continue;
		}

		const UDOItemDefinition* Definition = ResolveItemDefinition(DefinitionId);
		if (!Definition || !Definition->FindFragment<UDOItemFragment_Consumable>())
		{
			return false;
		}

	}

	QuickBarDefinitions = Definitions;
	++Revision;
	BroadcastChanged();
	return true;
}

const UDOItemDefinition* UDOItemQuickBarComponent::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
{
	return UDOItemDefinitionSubsystem::ResolveItemDefinition(this, DefinitionId);
}

void UDOItemQuickBarComponent::RequestAssignDefinition(const int32 SlotIndex, const FPrimaryAssetId DefinitionId, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestAssignDefinition_Implementation(SlotIndex, DefinitionId, ClientOperationId);
	}
	else
	{
		Server_RequestAssignDefinition(SlotIndex, DefinitionId, ClientOperationId);
	}
}

void UDOItemQuickBarComponent::RequestUseSlot(const int32 SlotIndex, const int32 ClientOperationId)
{
	if (ClientOperationId > 0)
	{
		DelegatedUseOperationIds.Add(ClientOperationId);
	}
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestUseSlot_Implementation(SlotIndex, ClientOperationId);
	}
	else
	{
		Server_RequestUseSlot(SlotIndex, ClientOperationId);
	}
}

void UDOItemQuickBarComponent::Server_RequestAssignDefinition_Implementation(const int32 SlotIndex, const FPrimaryAssetId DefinitionId, const int32 ClientOperationId)
{
	if (!QuickBarDefinitions.IsValidIndex(SlotIndex))
	{
		Client_QuickBarOperationResult(ClientOperationId, false, EDOInventoryFailureReason::InvalidSlot);
		return;
	}

	if (!DefinitionId.IsValid())
	{
		if (!QuickBarDefinitions[SlotIndex].IsValid())
		{
			Client_QuickBarOperationResultEx(ClientOperationId, EDOItemOperationOutcome::NoOp, EDOInventoryFailureReason::None, Revision);
			return;
		}
		QuickBarDefinitions[SlotIndex] = FPrimaryAssetId();
		++Revision;
		BroadcastChanged();
		Client_QuickBarOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
		return;
	}

	const UDOItemDefinition* Definition = ResolveItemDefinition(DefinitionId);
	const UDOItemFragment_Consumable* ConsumableFragment = Definition ? Definition->FindFragment<UDOItemFragment_Consumable>() : nullptr;
	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Definition || !ConsumableFragment || !Inventory)
	{
		Client_QuickBarOperationResult(ClientOperationId, false, Inventory ? EDOInventoryFailureReason::InvalidDefinition : EDOInventoryFailureReason::NotOwner);
		return;
	}

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!Items.ContainsByPredicate([&DefinitionId](const FDOItemInstanceRecord& Item)
	{
		return Item.DefinitionId == DefinitionId;
	}))
	{
		Client_QuickBarOperationResult(ClientOperationId, false, EDOInventoryFailureReason::ItemNotFound);
		return;
	}
	if (QuickBarDefinitions[SlotIndex] == DefinitionId)
	{
		Client_QuickBarOperationResultEx(ClientOperationId, EDOItemOperationOutcome::NoOp, EDOInventoryFailureReason::None, Revision);
		return;
	}

	QuickBarDefinitions[SlotIndex] = DefinitionId;
	++Revision;
	BroadcastChanged();
	Client_QuickBarOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
}

void UDOItemQuickBarComponent::Server_RequestUseSlot_Implementation(const int32 SlotIndex, const int32 ClientOperationId)
{
	if (!QuickBarDefinitions.IsValidIndex(SlotIndex))
	{
		Client_QuickBarOperationResult(ClientOperationId, false, EDOInventoryFailureReason::InvalidSlot);
		return;
	}

	const FPrimaryAssetId DefinitionId = QuickBarDefinitions[SlotIndex];
	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		Client_QuickBarOperationResult(ClientOperationId, false, EDOInventoryFailureReason::NotOwner);
		return;
	}

	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	bool bDeferredCompletion = false;
	if (!Inventory->TryUseItemByDefinition(DefinitionId, FailureReason, ClientOperationId, &bDeferredCompletion))
	{
		Client_QuickBarOperationResult(ClientOperationId, false, FailureReason);
		return;
	}
	if (bDeferredCompletion)
	{
		// Ability/Event 会在 CommitItemUse 或取消时由 Inventory 广播终态，不能提前回 Success。
		return;
	}
	Client_QuickBarOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
}

void UDOItemQuickBarComponent::Client_QuickBarOperationResult_Implementation(const int32 ClientOperationId, const bool bSuccess, const EDOInventoryFailureReason FailureReason)
{
	DelegatedUseOperationIds.Remove(ClientOperationId);
	if (!bSuccess)
	{
		BroadcastOperationFailure(ClientOperationId, FailureReason);
	}
	BroadcastOperationResult(ClientOperationId, bSuccess ? EDOItemOperationOutcome::Success : EDOItemOperationOutcome::Failure, FailureReason, Revision);
}

void UDOItemQuickBarComponent::Client_QuickBarOperationResultEx_Implementation(const int32 ClientOperationId, const EDOItemOperationOutcome Outcome, const EDOInventoryFailureReason FailureReason, const int32 AuthoritativeRevision)
{
	DelegatedUseOperationIds.Remove(ClientOperationId);
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

void UDOItemQuickBarComponent::HandleInventoryOperationResult(FGameplayTag /*Channel*/, const FDOInventoryOperationResultMessage& Message)
{
	if (!Message.InventoryComponent || !GetOwner() || GetOwner()->HasAuthority())
	{
		return;
	}

	const int32 OperationId = Message.Result.ClientOperationId;
	if (OperationId <= 0 || !DelegatedUseOperationIds.Contains(OperationId))
	{
		return;
	}

	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	if (!PlayerState || Message.InventoryComponent != PlayerState->GetInventoryComponent())
	{
		return;
	}

	DelegatedUseOperationIds.Remove(OperationId);
	if (Message.Result.Outcome == EDOItemOperationOutcome::Failure)
	{
		BroadcastOperationFailure(OperationId, Message.Result.FailureReason);
	}
	// QuickBar 是对外的操作域，Revision 使用 QuickBar 自己的复制版本；
	// Inventory 的权威 Revision 仍保留在原始 Inventory 消息中。
	BroadcastOperationResult(OperationId, Message.Result.Outcome, Message.Result.FailureReason, Revision);
}

void UDOItemQuickBarComponent::OnRep_QuickBarDefinitions()
{
	QuickBarDefinitions.SetNum(QuickBarSlotCount);
	BroadcastChanged();
}

void UDOItemQuickBarComponent::BroadcastChanged()
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOItemQuickBarChangedMessage Message;
	Message.QuickBarComponent = this;
	Message.Revision = Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::ItemQuickBar::Changed, Message);
}

void UDOItemQuickBarComponent::BroadcastOperationFailure(const int32 ClientOperationId, const EDOInventoryFailureReason FailureReason)
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOItemQuickBarOperationFailedMessage Message;
	Message.QuickBarComponent = this;
	Message.ClientOperationId = ClientOperationId;
	Message.FailureReason = FailureReason;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationFailed, Message);
}

void UDOItemQuickBarComponent::BroadcastOperationResult(const int32 ClientOperationId, const EDOItemOperationOutcome Outcome, const EDOInventoryFailureReason FailureReason, const int32 AuthoritativeRevision)
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOItemQuickBarOperationResultMessage Message;
	Message.QuickBarComponent = this;
	Message.Result.Domain = EDOItemOperationDomain::QuickBar;
	Message.Result.Outcome = Outcome;
	Message.Result.ClientOperationId = ClientOperationId;
	Message.Result.FailureReason = FailureReason;
	Message.Result.AuthoritativeRevision = AuthoritativeRevision >= 0 ? AuthoritativeRevision : Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::ItemQuickBar::OperationResult, Message);
}
