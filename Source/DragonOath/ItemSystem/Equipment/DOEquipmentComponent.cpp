#include "ItemSystem/Equipment/DOEquipmentComponent.h"

#include "AbilitySystem/Attributes/DOAttributeSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/AbilitySystem/DOItemEffectSpecBuilder.h"
#include "ItemSystem/AbilitySystem/DOItemGameplayEffects.h"
#include "Characters/DOCharacter.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "Player/DOPlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOEquipmentComponent)

namespace
{
	bool IsValidEquipmentSlotTag(const FGameplayTag& SlotTag)
	{
		return SlotTag == DragonOathGameplayTags::Equipment::Slot::Head
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Shoulder
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Back
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Chest
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Hands
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Legs
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Feet
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Accessory
			|| SlotTag == DragonOathGameplayTags::Equipment::Slot::Weapon;
	}

	/** 把旧版 Tag Map 转成新结构体，迁移期间只在新字段完全为空时使用。 */
	FDOAttributeModifierValues MakeLegacyAttributeValues(const UDOItemFragment_Equipment& Fragment)
	{
		FDOAttributeModifierValues Values = Fragment.AttributeModifiers;
		if (!Values.IsNearlyZero() || Fragment.BaseAttributeMagnitudes.Num() == 0)
		{
			return Values;
		}

		for (const TPair<FGameplayTag, FScalableFloat>& Pair : Fragment.BaseAttributeMagnitudes)
		{
			const float Value = Pair.Value.GetValueAtLevel(1.0f);
			if (Pair.Key == DragonOathGameplayTags::Data::Equipment::AttackPower)
			{
				Values.AttackPower = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::DefensePower)
			{
				Values.DefensePower = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::MaxHealth)
			{
				Values.MaxHealth = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::MaxMana)
			{
				Values.MaxMana = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::CriticalRating)
			{
				Values.CriticalRating = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::HitRating)
			{
				Values.HitRating = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::EvasionRating)
			{
				Values.EvasionRating = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::AttackSpeed)
			{
				Values.AttackSpeed = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::MoveSpeed)
			{
				Values.MoveSpeed = Value;
			}
			else if (Pair.Key == DragonOathGameplayTags::Data::Equipment::LifeStealRate)
			{
				Values.LifeStealRate = Value;
			}
		}
		return Values;
	}
}

void FDOEquipmentList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 /*FinalSize*/)
{
	if (!OwnerComponent)
	{
		return;
	}

	TArray<FGameplayTag> ChangedSlots;
	for (const int32 Index : AddedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			ChangedSlots.Add(Entries[Index].SlotTag);
		}
	}
	OwnerComponent->HandleFastArrayChanged(ChangedSlots);
}

void FDOEquipmentList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 /*FinalSize*/)
{
	if (!OwnerComponent)
	{
		return;
	}

	TArray<FGameplayTag> ChangedSlots;
	for (const int32 Index : ChangedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			ChangedSlots.Add(Entries[Index].SlotTag);
		}
	}
	OwnerComponent->HandleFastArrayChanged(ChangedSlots);
}

void FDOEquipmentList::PostReplicatedRemove(const TArrayView<int32>& /*RemovedIndices*/, int32 /*FinalSize*/)
{
	if (OwnerComponent)
	{
		OwnerComponent->HandleFastArrayChanged({});
	}
}

UDOEquipmentComponent::UDOEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	EquipmentList.OwnerComponent = this;
	EquipmentAttributeEffect = UDOEquipmentAttributeEffect::StaticClass();
}

void UDOEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	EquipmentList.OwnerComponent = this;
}

void UDOEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UDOEquipmentComponent, Revision, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDOEquipmentComponent, EquipmentList, COND_OwnerOnly);
}

void UDOEquipmentComponent::GetEquippedSnapshot(TArray<FDOEquippedItemEntry>& OutEntries) const
{
	OutEntries = EquipmentList.Entries;
}

bool UDOEquipmentComponent::ValidateEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries) const
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Entries.Num() > 9)
	{
		return false;
	}

	TSet<FGameplayTag> SlotTags;
	TSet<FGuid> InstanceIds;
	for (const FDOEquippedItemEntry& Entry : Entries)
	{
		const UDOItemFragment_Equipment* Fragment = nullptr;
		if (!Entry.SlotTag.IsValid()
			|| !IsValidEquipmentSlotTag(Entry.SlotTag)
			|| SlotTags.Contains(Entry.SlotTag)
			|| InstanceIds.Contains(Entry.Item.InstanceId)
			|| Entry.Item.SlotIndex != INDEX_NONE
			|| !ValidateEquipment(Entry.Item, Fragment)
			|| Entry.SlotTag != Fragment->EquipmentSlotTag)
		{
			return false;
		}

		SlotTags.Add(Entry.SlotTag);
		InstanceIds.Add(Entry.Item.InstanceId);
	}

	return true;
}

const FDOEquippedItemEntry* UDOEquipmentComponent::FindEquippedBySlot(const FGameplayTag& SlotTag) const
{
	for (const FDOEquippedItemEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.SlotTag == SlotTag)
		{
			return &Entry;
		}
	}
	return nullptr;
}

bool UDOEquipmentComponent::IsSlotEquipped(const FGameplayTag SlotTag) const
{
	return FindEquippedBySlot(SlotTag) != nullptr;
}

const UDOItemDefinition* UDOEquipmentComponent::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
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

bool UDOEquipmentComponent::ValidateEquipment(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment*& OutFragment) const
{
	OutFragment = nullptr;
	const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
	// 装备操作的输入物品来自背包，因此此时允许拥有有效的背包槽位。
	// 只有写入装备栏后，装备快照中的物品才必须将 SlotIndex 设为 INDEX_NONE。
	if (!Definition || !Item.InstanceId.IsValid() || Item.StackCount != 1)
	{
		return false;
	}

	OutFragment = Definition->FindFragment<UDOItemFragment_Equipment>();
	if (!OutFragment || !IsValidEquipmentSlotTag(OutFragment->EquipmentSlotTag))
	{
		return false;
	}

	const ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	const ADOCharacter* Character = PlayerState ? Cast<ADOCharacter>(PlayerState->GetPawn()) : nullptr;
	if (Character && Character->GetCharacterLevel() < OutFragment->RequiredLevel)
	{
		return false;
	}

	if (PlayerState && !OutFragment->RequiredProfessionQuery.IsEmpty())
	{
		FGameplayTagContainer ProfessionTags;
		ProfessionTags.AddTag(PlayerState->GetProfessionTag());
		if (!OutFragment->RequiredProfessionQuery.Matches(ProfessionTags))
		{
			return false;
		}
	}

	return true;
}

bool UDOEquipmentComponent::ApplyEquipmentEffect(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment& Fragment, FActiveGameplayEffectHandle& OutHandle)
{
	OutHandle = FActiveGameplayEffectHandle();

	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (!ASC || !ASC->AbilityActorInfo.IsValid())
	{
		return false;
	}

	const float UpgradeScale = 1.0f + (FMath::Max(0, Item.UpgradeLevel) * 0.05f);
	const FDOAttributeModifierValues AttributeValues = MakeLegacyAttributeValues(Fragment);

	// 新方案统一使用稳定的 C++ 原生 GE 和 Builder；旧蓝图 GE 仅在迁移期间显式覆盖时兼容。
	if (!EquipmentAttributeEffect || EquipmentAttributeEffect == UDOEquipmentAttributeEffect::StaticClass())
	{
		FGameplayEffectSpecHandle SpecHandle;
		if (!FDOItemEffectSpecBuilder::BuildEquipmentSpec(*ASC, *this, AttributeValues, UpgradeScale, SpecHandle))
		{
			return false;
		}

		OutHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		return OutHandle.WasSuccessfullyApplied();
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle LegacySpec = ASC->MakeOutgoingSpec(EquipmentAttributeEffect, 1.0f, EffectContext);
	if (!LegacySpec.IsValid() || !LegacySpec.Data.IsValid())
	{
		return false;
	}

	// 旧自定义 GE 仍由旧 Map 提供数值，保证迁移期间已有资产可运行。
	for (const TPair<FGameplayTag, FScalableFloat>& AttributeMagnitude : Fragment.BaseAttributeMagnitudes)
	{
		const float Value = AttributeMagnitude.Value.GetValueAtLevel(1.0f) * UpgradeScale;
		LegacySpec.Data->SetSetByCallerMagnitude(AttributeMagnitude.Key, Value);
	}

	OutHandle = ASC->ApplyGameplayEffectSpecToSelf(*LegacySpec.Data.Get());
	// 持续型装备 GE 通常有有效句柄，但统一使用 GAS 的成功状态判断更稳妥。
	return OutHandle.WasSuccessfullyApplied();
}

void UDOEquipmentComponent::RemoveEquipmentEffect(const FGameplayTag& SlotTag)
{
	if (const FActiveGameplayEffectHandle* Handle = ActiveEquipmentEffects.Find(SlotTag))
	{
		if (ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner()))
		{
			if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(*Handle);
			}
		}
		ActiveEquipmentEffects.Remove(SlotTag);
	}
}

bool UDOEquipmentComponent::RestoreEquipmentEffect(const FDOItemInstanceRecord& Item, const FGameplayTag& SlotTag)
{
	const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
	const UDOItemFragment_Equipment* Fragment = Definition ? Definition->FindFragment<UDOItemFragment_Equipment>() : nullptr;
	if (!Fragment)
	{
		return false;
	}

	FActiveGameplayEffectHandle EffectHandle;
	if (!ApplyEquipmentEffect(Item, *Fragment, EffectHandle))
	{
		return false;
	}

	if (EffectHandle.IsValid())
	{
		ActiveEquipmentEffects.Add(SlotTag, EffectHandle);
	}
	return true;
}

bool UDOEquipmentComponent::RestoreEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries)
{
	if (!ValidateEquippedSnapshot(Entries))
	{
		return false;
	}

	const TArray<FDOEquippedItemEntry> PreviousEntries = EquipmentList.Entries;
	for (const FDOEquippedItemEntry& Entry : PreviousEntries)
	{
		RemoveEquipmentEffect(Entry.SlotTag);
	}

	EquipmentList.Entries.Reset();
	ActiveEquipmentEffects.Reset();

	TArray<FGameplayTag> AppliedSlots;
	for (const FDOEquippedItemEntry& Entry : Entries)
	{
		if (!RestoreEquipmentEffect(Entry.Item, Entry.SlotTag))
		{
			for (const FGameplayTag& AppliedSlot : AppliedSlots)
			{
				RemoveEquipmentEffect(AppliedSlot);
			}

			EquipmentList.Entries = PreviousEntries;
			EquipmentList.MarkArrayDirty();
			for (const FDOEquippedItemEntry& PreviousEntry : PreviousEntries)
			{
				RestoreEquipmentEffect(PreviousEntry.Item, PreviousEntry.SlotTag);
			}
			return false;
		}

		FDOEquippedItemEntry& NewEntry = EquipmentList.Entries.AddDefaulted_GetRef();
		NewEntry = Entry;
		EquipmentList.MarkItemDirty(NewEntry);
		AppliedSlots.Add(Entry.SlotTag);
	}

	EquipmentList.MarkArrayDirty();
	BroadcastChanged({});
	return true;
}

void UDOEquipmentComponent::RequestEquipItem(const FGuid& InstanceId, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestEquipItem_Implementation(InstanceId, ClientOperationId);
	}
	else
	{
		Server_RequestEquipItem(InstanceId, ClientOperationId);
	}
}

void UDOEquipmentComponent::RequestUnequipItem(const FGameplayTag SlotTag, const int32 ClientOperationId)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_RequestUnequipItem_Implementation(SlotTag, ClientOperationId);
	}
	else
	{
		Server_RequestUnequipItem(SlotTag, ClientOperationId);
	}
}

void UDOEquipmentComponent::Server_RequestEquipItem_Implementation(const FGuid& InstanceId, const int32 ClientOperationId)
{
	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		Client_EquipmentOperationResult(ClientOperationId, false, EDOInventoryFailureReason::NotOwner);
		return;
	}

	const FDOItemInstanceRecord* ItemPtr = Inventory->FindItemByInstanceId(InstanceId);
	if (!ItemPtr)
	{
		Client_EquipmentOperationResult(ClientOperationId, false, EDOInventoryFailureReason::ItemNotFound);
		return;
	}
	const FDOItemInstanceRecord Item = *ItemPtr;
	const UDOItemFragment_Equipment* Fragment = nullptr;
	if (!ValidateEquipment(Item, Fragment))
	{
		Client_EquipmentOperationResult(ClientOperationId, false, EDOInventoryFailureReason::RequirementFailed);
		return;
	}

	const FGameplayTag SlotTag = Fragment->EquipmentSlotTag;
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	const FDOEquippedItemEntry* OldEntry = FindEquippedBySlot(SlotTag);
	const FDOItemInstanceRecord OldItem = OldEntry ? OldEntry->Item : FDOItemInstanceRecord();
	const int32 OldEntryIndex = OldEntry ? EquipmentList.Entries.IndexOfByPredicate([&SlotTag](const FDOEquippedItemEntry& Entry)
	{
		return Entry.SlotTag == SlotTag;
	}) : INDEX_NONE;

	// 替换装备必须在修改任何数组前确认旧装备当前就能放回背包。
	// 第一版按设计约定：背包已满时不进行替换，避免客户端看到跨组件的中间状态。
	if (OldEntry && !Inventory->CanInsertExistingItem(OldItem, FailureReason))
	{
		Client_EquipmentOperationResult(ClientOperationId, false, FailureReason);
		return;
	}

	// 先准备新装备 GE。此时尚未改动背包和装备数组，失败可以直接退出。
	if (OldEntry)
	{
		RemoveEquipmentEffect(SlotTag);
	}

	FActiveGameplayEffectHandle NewEffectHandle;
	if (!ApplyEquipmentEffect(Item, *Fragment, NewEffectHandle))
	{
		if (OldEntry)
		{
			RestoreEquipmentEffect(OldItem, SlotTag);
		}
		Client_EquipmentOperationResult(ClientOperationId, false, EDOInventoryFailureReason::Unknown);
		return;
	}

	FDOItemInstanceRecord RemovedItem;
	if (!Inventory->TryRemoveItemByInstanceId(InstanceId, RemovedItem, FailureReason))
	{
		if (NewEffectHandle.IsValid())
		{
			if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(NewEffectHandle);
			}
		}
		if (OldEntry)
		{
			RestoreEquipmentEffect(OldItem, SlotTag);
		}
		Client_EquipmentOperationResult(ClientOperationId, false, FailureReason);
		return;
	}

	if (OldEntry && !Inventory->TryInsertExistingItem(OldItem, FailureReason))
	{
		// 新装备已经从背包移出，理论上必然有一个空位；失败时完整恢复旧状态。
		Inventory->TryInsertExistingItem(RemovedItem, FailureReason);
		if (NewEffectHandle.IsValid())
		{
			if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(NewEffectHandle);
			}
		}
		RestoreEquipmentEffect(OldItem, SlotTag);
		Client_EquipmentOperationResult(ClientOperationId, false, FailureReason);
		return;
	}

	if (OldEntryIndex != INDEX_NONE)
	{
		EquipmentList.Entries.RemoveAt(OldEntryIndex);
		EquipmentList.MarkArrayDirty();
	}

	FDOEquippedItemEntry& NewEntry = EquipmentList.Entries.AddDefaulted_GetRef();
	NewEntry.SlotTag = SlotTag;
	NewEntry.Item = RemovedItem;
	NewEntry.Item.SlotIndex = INDEX_NONE;
	EquipmentList.MarkItemDirty(NewEntry);
	if (NewEffectHandle.IsValid())
	{
		ActiveEquipmentEffects.Add(SlotTag, NewEffectHandle);
	}

	BroadcastChanged({ SlotTag });
}

void UDOEquipmentComponent::Server_RequestUnequipItem_Implementation(const FGameplayTag SlotTag, const int32 ClientOperationId)
{
	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		Client_EquipmentOperationResult(ClientOperationId, false, EDOInventoryFailureReason::NotOwner);
		return;
	}

	const int32 EntryIndex = EquipmentList.Entries.IndexOfByPredicate([&SlotTag](const FDOEquippedItemEntry& Entry)
	{
		return Entry.SlotTag == SlotTag;
	});
	if (!EquipmentList.Entries.IsValidIndex(EntryIndex))
	{
		Client_EquipmentOperationResult(ClientOperationId, false, EDOInventoryFailureReason::ItemNotFound);
		return;
	}

	const FDOItemInstanceRecord Item = EquipmentList.Entries[EntryIndex].Item;
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	if (!Inventory->TryInsertExistingItem(Item, FailureReason))
	{
		Client_EquipmentOperationResult(ClientOperationId, false, FailureReason);
		return;
	}

	RemoveEquipmentEffect(SlotTag);
	EquipmentList.Entries.RemoveAt(EntryIndex);
	EquipmentList.MarkArrayDirty();
	BroadcastChanged({ SlotTag });
}

void UDOEquipmentComponent::Client_EquipmentOperationResult_Implementation(const int32 ClientOperationId, const bool bSuccess, const EDOInventoryFailureReason FailureReason)
{
	if (!bSuccess)
	{
		BroadcastOperationFailure(ClientOperationId, FailureReason);
	}
}

void UDOEquipmentComponent::BroadcastOperationFailure(const int32 ClientOperationId, const EDOInventoryFailureReason FailureReason)
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOEquipmentOperationFailedMessage Message;
	Message.EquipmentComponent = this;
	Message.ClientOperationId = ClientOperationId;
	Message.FailureReason = FailureReason;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Equipment::OperationFailed, Message);
}

void UDOEquipmentComponent::HandleFastArrayChanged(const TArray<FGameplayTag>& ChangedSlotTags)
{
	EquipmentList.OwnerComponent = this;
	BroadcastChanged(ChangedSlotTags);
}

void UDOEquipmentComponent::BroadcastChanged(const TArray<FGameplayTag>& ChangedSlotTags)
{
	++Revision;
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOEquipmentChangedMessage Message;
	Message.EquipmentComponent = this;
	Message.ChangedSlotTags = ChangedSlotTags;
	Message.Revision = Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Equipment::Changed, Message);
}
