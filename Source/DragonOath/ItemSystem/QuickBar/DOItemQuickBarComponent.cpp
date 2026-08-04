#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
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

	const ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	const UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);

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

		if (!Items.ContainsByPredicate([&DefinitionId](const FDOItemInstanceRecord& Item)
		{
			return Item.DefinitionId == DefinitionId;
		}))
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
		QuickBarDefinitions[SlotIndex] = FPrimaryAssetId();
		++Revision;
		BroadcastChanged();
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

	QuickBarDefinitions[SlotIndex] = DefinitionId;
	++Revision;
	BroadcastChanged();
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
	if (!Inventory->TryUseItemByDefinition(DefinitionId, FailureReason))
	{
		Client_QuickBarOperationResult(ClientOperationId, false, FailureReason);
	}
}

void UDOItemQuickBarComponent::Client_QuickBarOperationResult_Implementation(const int32 ClientOperationId, const bool bSuccess, const EDOInventoryFailureReason FailureReason)
{
	if (!bSuccess)
	{
		BroadcastOperationFailure(ClientOperationId, FailureReason);
	}
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
