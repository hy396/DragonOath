#include "ItemSystem/Equipment/DOEquipmentPresentationComponent.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "Player/DOPlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOEquipmentPresentationComponent)

void FDOEquipmentPublicList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : AddedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedSlotTags.AddUnique(Entries[Index].SlotTag);
		}
	}
}

void FDOEquipmentPublicList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : ChangedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedSlotTags.AddUnique(Entries[Index].SlotTag);
		}
	}
}

void FDOEquipmentPublicList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : RemovedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedSlotTags.AddUnique(Entries[Index].SlotTag);
		}
	}
}

void FDOEquipmentPublicList::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& /*Parameters*/)
{
	if (OwnerComponent && PendingChangedSlotTags.Num() > 0)
	{
		OwnerComponent->HandleFastArrayChanged(PendingChangedSlotTags);
	}
	PendingChangedSlotTags.Reset();
}

UDOEquipmentPresentationComponent::UDOEquipmentPresentationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PublicEquipmentList.OwnerComponent = this;
}

void UDOEquipmentPresentationComponent::BeginPlay()
{
	Super::BeginPlay();
	PublicEquipmentList.OwnerComponent = this;
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RebuildFromOwnerEquipment();
	}
}

void UDOEquipmentPresentationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDOEquipmentPresentationComponent, Revision);
	DOREPLIFETIME(UDOEquipmentPresentationComponent, PublicEquipmentList);
}

bool UDOEquipmentPresentationComponent::AreEntriesEqual(const FDOEquipmentPublicEntry& A, const FDOEquipmentPublicEntry& B)
{
	return A.SlotTag == B.SlotTag
		&& A.AppearanceId == B.AppearanceId
		&& A.VariantTag == B.VariantTag
		&& A.Tint.Equals(B.Tint);
}

void UDOEquipmentPresentationComponent::RebuildFromEquipment(const UDOEquipmentComponent& EquipmentComponent)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	TArray<FDOEquippedItemEntry> EquippedEntries;
	EquipmentComponent.GetEquippedSnapshot(EquippedEntries);

	TArray<FDOEquipmentPublicEntry> DesiredEntries;
	DesiredEntries.Reserve(EquippedEntries.Num());
	for (const FDOEquippedItemEntry& EquippedEntry : EquippedEntries)
	{
		FDOEquipmentPublicEntry& PublicEntry = DesiredEntries.AddDefaulted_GetRef();
		PublicEntry.SlotTag = EquippedEntry.SlotTag;

		const UDOItemDefinition* Definition = UDOItemDefinitionSubsystem::ResolveItemDefinition(this, EquippedEntry.Item.DefinitionId);
		if (Definition)
		{
			if (const UDOItemFragment_EquipmentAppearance* Appearance = Definition->FindFragment<UDOItemFragment_EquipmentAppearance>())
			{
				PublicEntry.AppearanceId = Appearance->AppearanceId;
				PublicEntry.VariantTag = Appearance->VariantTag;
				PublicEntry.Tint = Appearance->Tint;
			}
		}
	}

	DesiredEntries.Sort([](const FDOEquipmentPublicEntry& A, const FDOEquipmentPublicEntry& B)
	{
		return A.SlotTag.ToString() < B.SlotTag.ToString();
	});

	TArray<FGameplayTag> ChangedSlotTags;
	for (const FDOEquipmentPublicEntry& Existing : PublicEquipmentList.Entries)
	{
		if (!DesiredEntries.ContainsByPredicate([&Existing](const FDOEquipmentPublicEntry& Desired)
		{
			return Desired.SlotTag == Existing.SlotTag;
		}))
		{
			ChangedSlotTags.AddUnique(Existing.SlotTag);
		}
	}

	for (FDOEquipmentPublicEntry& Desired : DesiredEntries)
	{
		const FDOEquipmentPublicEntry* Existing = PublicEquipmentList.Entries.FindByPredicate([&Desired](const FDOEquipmentPublicEntry& Candidate)
		{
			return Candidate.SlotTag == Desired.SlotTag;
		});
		if (Existing && AreEntriesEqual(*Existing, Desired))
		{
			Desired.VisualRevision = Existing->VisualRevision;
		}
		else
		{
			ChangedSlotTags.AddUnique(Desired.SlotTag);
		}
	}

	if (ChangedSlotTags.Num() == 0)
	{
		return;
	}

	++Revision;
	for (FDOEquipmentPublicEntry& Desired : DesiredEntries)
	{
		if (const FDOEquipmentPublicEntry* Existing = PublicEquipmentList.Entries.FindByPredicate([&Desired](const FDOEquipmentPublicEntry& Candidate)
		{
			return Candidate.SlotTag == Desired.SlotTag;
		}))
		{
			if (AreEntriesEqual(*Existing, Desired))
			{
				Desired.VisualRevision = Existing->VisualRevision;
			}
		}
		if (Desired.VisualRevision == 0 || ChangedSlotTags.Contains(Desired.SlotTag))
		{
			Desired.VisualRevision = Revision;
		}
	}

	PublicEquipmentList.Entries = MoveTemp(DesiredEntries);
	PublicEquipmentList.MarkArrayDirty();
	BroadcastChanged(ChangedSlotTags);
}

void UDOEquipmentPresentationComponent::RebuildFromOwnerEquipment()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const ADOPlayerState* PawnPlayerState = Cast<ADOPlayerState>(Pawn->GetPlayerState()))
		{
			if (const UDOEquipmentComponent* Equipment = PawnPlayerState->GetEquipmentComponent())
			{
				RebuildFromEquipment(*Equipment);
			}
		}
	}
}

void UDOEquipmentPresentationComponent::GetPublicSnapshot(TArray<FDOEquipmentPublicEntry>& OutEntries) const
{
	OutEntries = PublicEquipmentList.Entries;
}

const FDOEquipmentPublicEntry* UDOEquipmentPresentationComponent::FindPublicEntry(const FGameplayTag& SlotTag) const
{
	return PublicEquipmentList.Entries.FindByPredicate([&SlotTag](const FDOEquipmentPublicEntry& Entry)
	{
		return Entry.SlotTag == SlotTag;
	});
}

bool UDOEquipmentPresentationComponent::GetPublicEntry(const FGameplayTag SlotTag, FDOEquipmentPublicEntry& OutEntry) const
{
	if (const FDOEquipmentPublicEntry* Entry = FindPublicEntry(SlotTag))
	{
		OutEntry = *Entry;
		return true;
	}

	OutEntry = FDOEquipmentPublicEntry();
	return false;
}

bool UDOEquipmentPresentationComponent::ResolveAppearance(const FName AppearanceId, const FGameplayTag VariantTag, FDOEquipmentAppearanceEntry& OutAppearance) const
{
	return AppearanceRegistry && AppearanceRegistry->ResolveAppearance(AppearanceId, VariantTag, OutAppearance);
}

void UDOEquipmentPresentationComponent::HandleFastArrayChanged(const TArray<FGameplayTag>& ChangedSlotTags)
{
	PublicEquipmentList.OwnerComponent = this;
	BroadcastChanged(ChangedSlotTags);
}

void UDOEquipmentPresentationComponent::BroadcastChanged(const TArray<FGameplayTag>& ChangedSlotTags, const bool /*bAdvanceRevision*/)
{
	OnPresentationChanged.Broadcast(ChangedSlotTags, Revision);

	FGameplayTagContainer ChangedTagContainer;
	for (const FGameplayTag& ChangedSlotTag : ChangedSlotTags)
	{
		ChangedTagContainer.AddTag(ChangedSlotTag);
	}
	OnPresentationChangedBP.Broadcast(ChangedTagContainer, Revision);

	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOEquipmentPresentationChangedMessage Message;
	Message.PresentationComponent = this;
	Message.ChangedSlotTags = ChangedSlotTags;
	Message.Revision = Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Equipment::PresentationChanged, Message);
}
