#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/Equipment/DOEquipmentPresentationComponent.h"
#include "ItemSystem/Equipment/DOEquipmentLayout.h"

#include "AbilitySystem/Attributes/DOAttributeSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/AbilitySystem/DOItemEffectSpecBuilder.h"
#include "ItemSystem/AbilitySystem/DOItemGameplayEffects.h"
#include "Characters/DOCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Inventory/DOInventoryMessages.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "Player/DOPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Misc/ScopeExit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOEquipmentComponent)

namespace
{
	bool IsDefaultEquipmentSlotTag(const FGameplayTag& SlotTag)
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

	void ApplyAffixRolls(FDOAttributeModifierValues& Values, const TArray<FDOItemAffixRoll>& Affixes)
	{
		for (const FDOItemAffixRoll& Affix : Affixes)
		{
			if (!Affix.AffixTag.IsValid() || !FMath::IsFinite(Affix.Magnitude))
			{
				continue;
			}

			if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::AttackPower) Values.AttackPower += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::DefensePower) Values.DefensePower += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::MaxHealth) Values.MaxHealth += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::MaxMana) Values.MaxMana += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::CriticalRating) Values.CriticalRating += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::HitRating) Values.HitRating += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::EvasionRating) Values.EvasionRating += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::AttackSpeed) Values.AttackSpeed += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::MoveSpeed) Values.MoveSpeed += Affix.Magnitude;
			else if (Affix.AffixTag == DragonOathGameplayTags::Data::Equipment::LifeStealRate) Values.LifeStealRate += Affix.Magnitude;
		}
	}
}

void FDOEquipmentList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : AddedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedSlotTags.AddUnique(Entries[Index].SlotTag);
		}
	}
}

void FDOEquipmentList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : ChangedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedSlotTags.AddUnique(Entries[Index].SlotTag);
		}
	}
}

void FDOEquipmentList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 /*FinalSize*/)
{
	for (const int32 Index : RemovedIndices)
	{
		if (Entries.IsValidIndex(Index))
		{
			PendingChangedSlotTags.AddUnique(Entries[Index].SlotTag);
		}
	}
}

void FDOEquipmentList::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& /*Parameters*/)
{
	if (OwnerComponent && PendingChangedSlotTags.Num() > 0)
	{
		OwnerComponent->HandleFastArrayChanged(PendingChangedSlotTags);
	}
	PendingChangedSlotTags.Reset();
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

void UDOEquipmentComponent::GetSupportedSlotTags(TArray<FGameplayTag>& OutSlotTags) const
{
	if (EquipmentLayout)
	{
		OutSlotTags = EquipmentLayout->SlotTags;
		return;
	}

	OutSlotTags = {
		DragonOathGameplayTags::Equipment::Slot::Head,
		DragonOathGameplayTags::Equipment::Slot::Shoulder,
		DragonOathGameplayTags::Equipment::Slot::Back,
		DragonOathGameplayTags::Equipment::Slot::Chest,
		DragonOathGameplayTags::Equipment::Slot::Hands,
		DragonOathGameplayTags::Equipment::Slot::Legs,
		DragonOathGameplayTags::Equipment::Slot::Feet,
		DragonOathGameplayTags::Equipment::Slot::Accessory,
		DragonOathGameplayTags::Equipment::Slot::Weapon
	};
}

bool UDOEquipmentComponent::ValidateEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries) const
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || (EquipmentLayout ? Entries.Num() > EquipmentLayout->GetSlotCount() : Entries.Num() > 9))
	{
		return false;
	}

	TSet<FGameplayTag> SlotTags;
	TSet<FGuid> InstanceIds;
	for (const FDOEquippedItemEntry& Entry : Entries)
	{
		const UDOItemFragment_Equipment* Fragment = nullptr;
		if (!Entry.SlotTag.IsValid()
			|| !IsSlotConfigured(Entry.SlotTag)
			|| SlotTags.Contains(Entry.SlotTag)
			|| InstanceIds.Contains(Entry.Item.InstanceId)
			|| Entry.Item.SlotIndex != INDEX_NONE
			|| !ValidateEquipment(Entry.Item, Fragment)
			|| Entry.SlotTag != Fragment->EquipmentSlotTag
			|| Entry.Item.CurrentDurability < 0
			|| Entry.Item.CurrentDurability > Fragment->MaxDurability)
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

const UDOEquipmentInstance* UDOEquipmentComponent::FindEquipmentInstance(const FGameplayTag& SlotTag) const
{
	const TObjectPtr<UDOEquipmentInstance>* Instance = EquipmentInstances.Find(SlotTag);
	return Instance ? Instance->Get() : nullptr;
}

bool UDOEquipmentComponent::IsSlotEquipped(const FGameplayTag SlotTag) const
{
	return FindEquippedBySlot(SlotTag) != nullptr;
}

bool UDOEquipmentComponent::SetEquippedDurability(const FGameplayTag SlotTag, const int32 NewDurability)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	const int32 EntryIndex = EquipmentList.Entries.IndexOfByPredicate([&SlotTag](const FDOEquippedItemEntry& Entry)
	{
		return Entry.SlotTag == SlotTag;
	});
	if (!EquipmentList.Entries.IsValidIndex(EntryIndex))
	{
		return false;
	}

	FDOItemInstanceRecord& Item = EquipmentList.Entries[EntryIndex].Item;
	const FDOItemInstanceRecord PreviousItem = Item;
	const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
	const UDOItemFragment_Equipment* Fragment = Definition ? Definition->FindFragment<UDOItemFragment_Equipment>() : nullptr;
	if (!Fragment || NewDurability < 0 || NewDurability > Fragment->MaxDurability)
	{
		return false;
	}

	const bool bWasRuntimeActive = IsEquipmentRuntimeActive(PreviousItem, *Fragment);
	FDOItemInstanceRecord UpdatedItem = PreviousItem;
	UpdatedItem.CurrentDurability = NewDurability;
	const bool bWillBeRuntimeActive = IsEquipmentRuntimeActive(UpdatedItem, *Fragment);

	if (TObjectPtr<UDOEquipmentInstance>* InstancePtr = EquipmentInstances.Find(SlotTag))
	{
		UDOEquipmentInstance* Instance = InstancePtr->Get();
		if (!Instance)
		{
			return false;
		}
		if (bWasRuntimeActive != bWillBeRuntimeActive)
		{
			RemoveEquipmentInstanceFromAbilitySystem(*Instance);
			Item = UpdatedItem;
			EquipmentList.MarkItemDirty(EquipmentList.Entries[EntryIndex]);
			Instance->Initialize(Item, SlotTag);

			if (bWillBeRuntimeActive)
			{
				FActiveGameplayEffectHandle EffectHandle;
				const bool bEffectReady = ApplyEquipmentEffect(Item, *Fragment, Instance, EffectHandle);
				const bool bAbilitiesReady = bEffectReady && GrantEquipmentAbilities(*Instance, *Fragment);
				if (!bEffectReady || !bAbilitiesReady)
				{
					Instance->SetAttributeEffectHandle(EffectHandle);
					RemoveEquipmentInstanceFromAbilitySystem(*Instance);
					Item = PreviousItem;
					EquipmentList.MarkItemDirty(EquipmentList.Entries[EntryIndex]);
					Instance->Initialize(Item, SlotTag);
					return false;
				}
				Instance->SetAttributeEffectHandle(EffectHandle);
			}
		}
		else
		{
			Item = UpdatedItem;
			EquipmentList.MarkItemDirty(EquipmentList.Entries[EntryIndex]);
			Instance->UpdateItemRuntimeState(Item);
		}
	}
	else
	{
		// 快照正常情况下总有对应的运行时实例；拒绝异常状态，避免只改复制数据而不改 GAS。
		return false;
	}

	BroadcastChanged({ SlotTag });
	RebuildPublicPresentation();
	return true;
}

void UDOEquipmentComponent::RebuildPublicPresentation()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (const ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner()))
	{
		if (APawn* Pawn = PlayerState->GetPawn())
		{
			if (UDOEquipmentPresentationComponent* Presentation = Pawn->FindComponentByClass<UDOEquipmentPresentationComponent>())
			{
				Presentation->RebuildFromEquipment(*this);
			}
		}
	}
}

const UDOItemDefinition* UDOEquipmentComponent::ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const
{
	return UDOItemDefinitionSubsystem::ResolveItemDefinition(this, DefinitionId);
}

bool UDOEquipmentComponent::IsSlotConfigured(const FGameplayTag& SlotTag) const
{
	return EquipmentLayout ? EquipmentLayout->ContainsSlot(SlotTag) : IsDefaultEquipmentSlotTag(SlotTag);
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
	if (!OutFragment || !IsSlotConfigured(OutFragment->EquipmentSlotTag))
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

	if (!OutFragment->CompatibleSlotQuery.IsEmpty())
	{
		FGameplayTagContainer SlotTags;
		SlotTags.AddTag(OutFragment->EquipmentSlotTag);
		if (!OutFragment->CompatibleSlotQuery.Matches(SlotTags))
		{
			return false;
		}
	}

	return true;
}

bool UDOEquipmentComponent::IsEquipmentRuntimeActive(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment& Fragment) const
{
	return Fragment.MaxDurability <= 0 || Item.CurrentDurability > 0;
}

bool UDOEquipmentComponent::ApplyEquipmentEffect(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment& Fragment, UObject* SourceObject, FActiveGameplayEffectHandle& OutHandle)
{
	OutHandle = FActiveGameplayEffectHandle();
	if (!IsEquipmentRuntimeActive(Item, Fragment))
	{
		return true;
	}

	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	if (!ASC || !ASC->AbilityActorInfo.IsValid())
	{
		return false;
	}

	const float UpgradeScale = 1.0f + (FMath::Max(0, Item.UpgradeLevel) * 0.05f);
	FDOAttributeModifierValues AttributeValues = MakeLegacyAttributeValues(Fragment);
	ApplyAffixRolls(AttributeValues, Item.Affixes);

	// 新方案统一使用稳定的 C++ 原生 GE 和 Builder；旧蓝图 GE 仅在迁移期间显式覆盖时兼容。
	if (!EquipmentAttributeEffect || EquipmentAttributeEffect == UDOEquipmentAttributeEffect::StaticClass())
	{
		FGameplayEffectSpecHandle SpecHandle;
		if (!FDOItemEffectSpecBuilder::BuildEquipmentSpec(*ASC, SourceObject ? *SourceObject : *this, AttributeValues, Item.Affixes, UpgradeScale, SpecHandle))
		{
			return false;
		}

		OutHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		return OutHandle.WasSuccessfullyApplied();
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(SourceObject ? SourceObject : this);
	FGameplayEffectSpecHandle LegacySpec = ASC->MakeOutgoingSpec(EquipmentAttributeEffect, 1.0f, EffectContext);
	if (!LegacySpec.IsValid() || !LegacySpec.Data.IsValid())
	{
		return false;
	}

	// 旧自定义 GE 仍由旧 Map 提供数值，保证迁移期间已有资产可运行。
	for (const TPair<FGameplayTag, FScalableFloat>& AttributeMagnitude : Fragment.BaseAttributeMagnitudes)
	{
		float Value = AttributeMagnitude.Value.GetValueAtLevel(1.0f) * UpgradeScale;
		if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::AttackPower) Value = AttributeValues.AttackPower * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::DefensePower) Value = AttributeValues.DefensePower * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::MaxHealth) Value = AttributeValues.MaxHealth * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::MaxMana) Value = AttributeValues.MaxMana * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::CriticalRating) Value = AttributeValues.CriticalRating * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::HitRating) Value = AttributeValues.HitRating * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::EvasionRating) Value = AttributeValues.EvasionRating * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::AttackSpeed) Value = AttributeValues.AttackSpeed * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::MoveSpeed) Value = AttributeValues.MoveSpeed * UpgradeScale;
		else if (AttributeMagnitude.Key == DragonOathGameplayTags::Data::Equipment::LifeStealRate) Value = AttributeValues.LifeStealRate * UpgradeScale;
		LegacySpec.Data->SetSetByCallerMagnitude(AttributeMagnitude.Key, Value);
	}
	for (const FDOItemAffixRoll& Affix : Item.Affixes)
	{
		if (Affix.AffixTag.IsValid() && !Fragment.BaseAttributeMagnitudes.Contains(Affix.AffixTag))
		{
			LegacySpec.Data->SetSetByCallerMagnitude(Affix.AffixTag, Affix.Magnitude * UpgradeScale);
		}
	}

	OutHandle = ASC->ApplyGameplayEffectSpecToSelf(*LegacySpec.Data.Get());
	// 持续型装备 GE 通常有有效句柄，但统一使用 GAS 的成功状态判断更稳妥。
	return OutHandle.WasSuccessfullyApplied();
}

void UDOEquipmentComponent::RemoveEquipmentEffect(const FGameplayTag& SlotTag)
{
	if (TObjectPtr<UDOEquipmentInstance>* InstancePtr = EquipmentInstances.Find(SlotTag))
	{
		if (InstancePtr->Get())
		{
			RemoveEquipmentInstanceFromAbilitySystem(*InstancePtr->Get());
		}
		EquipmentInstances.Remove(SlotTag);
	}
}

bool UDOEquipmentComponent::RestoreEquipmentEffect(const FDOItemInstanceRecord& Item, const FGameplayTag& SlotTag)
{
	const UDOItemDefinition* Definition = ResolveItemDefinition(Item.DefinitionId);
	const UDOItemFragment_Equipment* Fragment = Definition ? Definition->FindFragment<UDOItemFragment_Equipment>() : nullptr;
	if (!Fragment || !IsSlotConfigured(SlotTag))
	{
		return false;
	}

	UDOEquipmentInstance* Instance = NewObject<UDOEquipmentInstance>(this);
	Instance->Initialize(Item, SlotTag);
	FActiveGameplayEffectHandle EffectHandle;
	const bool bEffectReady = ApplyEquipmentEffect(Item, *Fragment, Instance, EffectHandle);
	const bool bAbilitiesReady = !bEffectReady || !IsEquipmentRuntimeActive(Item, *Fragment) || GrantEquipmentAbilities(*Instance, *Fragment);
	if (!bEffectReady || !bAbilitiesReady)
	{
		Instance->SetAttributeEffectHandle(EffectHandle);
		RemoveEquipmentInstanceFromAbilitySystem(*Instance);
		return false;
	}

	Instance->SetAttributeEffectHandle(EffectHandle);
	EquipmentInstances.Add(SlotTag, Instance);
	return true;
}

bool UDOEquipmentComponent::GrantEquipmentAbilities(UDOEquipmentInstance& Instance, const UDOItemFragment_Equipment& Fragment)
{
	const UDOItemDefinition* Definition = ResolveItemDefinition(Instance.GetItem().DefinitionId);
	const UDOItemFragment_Equipment* RuntimeFragment = Definition ? Definition->FindFragment<UDOItemFragment_Equipment>() : nullptr;
	if (RuntimeFragment && !IsEquipmentRuntimeActive(Instance.GetItem(), *RuntimeFragment))
	{
		return true;
	}

	if (!Fragment.EquipmentAbilitySet)
	{
		return true;
	}

	ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner());
	UDOAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetDOAbilitySystemComponent() : nullptr;
	return ASC && ASC->GiveDOAbilitySetForSource(Fragment.EquipmentAbilitySet, &Instance, Instance.GetGrantedHandles());
}

void UDOEquipmentComponent::RemoveEquipmentInstanceFromAbilitySystem(UDOEquipmentInstance& Instance)
{
	if (ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwner()))
	{
		if (UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent())
		{
			if (Instance.GetAttributeEffectHandle().IsValid())
			{
				ASC->RemoveActiveGameplayEffect(Instance.GetAttributeEffectHandle());
			}
			ASC->RemoveDOAbilitySet(Instance.GetGrantedHandles());
		}
	}
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
	EquipmentInstances.Reset();

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
	RebuildPublicPresentation();
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

	// Inventory 与 Equipment 是两个聚合根，但一次装备替换必须只产生一个
	// Inventory Revision/Changed 通知，避免 UI 看到跨组件中间状态。
	Inventory->BeginMutation();
	ON_SCOPE_EXIT
	{
		Inventory->EndMutation();
	};

	// 先准备新装备 GE。此时尚未改动背包和装备数组，失败可以直接退出。
	if (OldEntry)
	{
		RemoveEquipmentEffect(SlotTag);
	}

	UDOEquipmentInstance* NewInstance = NewObject<UDOEquipmentInstance>(this);
	NewInstance->Initialize(Item, SlotTag);
	FActiveGameplayEffectHandle NewEffectHandle;
	if (!ApplyEquipmentEffect(Item, *Fragment, NewInstance, NewEffectHandle)
		|| !GrantEquipmentAbilities(*NewInstance, *Fragment))
	{
		NewInstance->SetAttributeEffectHandle(NewEffectHandle);
		RemoveEquipmentInstanceFromAbilitySystem(*NewInstance);
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
		NewInstance->SetAttributeEffectHandle(NewEffectHandle);
		RemoveEquipmentInstanceFromAbilitySystem(*NewInstance);
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
		NewInstance->SetAttributeEffectHandle(NewEffectHandle);
		RemoveEquipmentInstanceFromAbilitySystem(*NewInstance);
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
	NewInstance->SetAttributeEffectHandle(NewEffectHandle);
	EquipmentInstances.Add(SlotTag, NewInstance);

	BroadcastChanged({ SlotTag });
	RebuildPublicPresentation();
	Client_EquipmentOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
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
	RebuildPublicPresentation();
	Client_EquipmentOperationResultEx(ClientOperationId, EDOItemOperationOutcome::Success, EDOInventoryFailureReason::None, Revision);
}

void UDOEquipmentComponent::Client_EquipmentOperationResult_Implementation(const int32 ClientOperationId, const bool bSuccess, const EDOInventoryFailureReason FailureReason)
{
	if (!bSuccess)
	{
		BroadcastOperationFailure(ClientOperationId, FailureReason);
	}
	BroadcastOperationResult(ClientOperationId, bSuccess ? EDOItemOperationOutcome::Success : EDOItemOperationOutcome::Failure, FailureReason, Revision);
}

void UDOEquipmentComponent::Client_EquipmentOperationResultEx_Implementation(const int32 ClientOperationId, const EDOItemOperationOutcome Outcome, const EDOInventoryFailureReason FailureReason, const int32 AuthoritativeRevision)
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

void UDOEquipmentComponent::BroadcastOperationResult(const int32 ClientOperationId, const EDOItemOperationOutcome Outcome, const EDOInventoryFailureReason FailureReason, const int32 AuthoritativeRevision)
{
	if (!GetWorld() || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FDOEquipmentOperationResultMessage Message;
	Message.EquipmentComponent = this;
	Message.Result.Domain = EDOItemOperationDomain::Equipment;
	Message.Result.Outcome = Outcome;
	Message.Result.ClientOperationId = ClientOperationId;
	Message.Result.FailureReason = FailureReason;
	Message.Result.AuthoritativeRevision = AuthoritativeRevision >= 0 ? AuthoritativeRevision : Revision;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(DragonOathGameplayTags::Message::UI::Equipment::OperationResult, Message);
}

void UDOEquipmentComponent::HandleFastArrayChanged(const TArray<FGameplayTag>& ChangedSlotTags)
{
	EquipmentList.OwnerComponent = this;
	BroadcastChanged(ChangedSlotTags, false);
}

void UDOEquipmentComponent::BroadcastChanged(const TArray<FGameplayTag>& ChangedSlotTags, const bool bAdvanceRevision)
{
	if (bAdvanceRevision && GetOwner() && GetOwner()->HasAuthority())
	{
		++Revision;
	}
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
