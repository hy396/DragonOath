#include "UI/Inventory/DOItemTooltipViewModel.h"

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "UI/Inventory/DOEquipmentSlotViewModel.h"
#include "UI/Inventory/DOInventorySlotViewModel.h"
#include "UI/Inventory/DOInventoryViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemTooltipViewModel)

namespace
{
	FString TagLeaf(const FGameplayTag& Tag)
	{
		const FString Text = Tag.ToString();
		int32 DotIndex = INDEX_NONE;
		return Text.FindLastChar('.', DotIndex) ? Text.Mid(DotIndex + 1) : Text;
	}

	void AddAttribute(TMap<FGameplayTag, float>& OutValues, const FGameplayTag& Tag, float Value)
	{
		if (Tag.IsValid() && FMath::IsFinite(Value) && !FMath::IsNearlyZero(Value))
		{
			OutValues.Add(Tag, Value);
		}
	}

	void BuildEquipmentAttributes(const UDOItemFragment_Equipment& Fragment, TMap<FGameplayTag, float>& OutValues)
	{
		const FDOAttributeModifierValues& Values = Fragment.AttributeModifiers;
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::AttackPower, Values.AttackPower);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::DefensePower, Values.DefensePower);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::MaxHealth, Values.MaxHealth);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::MaxMana, Values.MaxMana);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::CriticalRating, Values.CriticalRating);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::HitRating, Values.HitRating);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::EvasionRating, Values.EvasionRating);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::AttackSpeed, Values.AttackSpeed);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::MoveSpeed, Values.MoveSpeed);
		AddAttribute(OutValues, DragonOathGameplayTags::Data::Equipment::LifeStealRate, Values.LifeStealRate);

		if (OutValues.IsEmpty())
		{
			for (const TPair<FGameplayTag, FScalableFloat>& Pair : Fragment.BaseAttributeMagnitudes)
			{
				AddAttribute(OutValues, Pair.Key, Pair.Value.GetValueAtLevel(1.0f));
			}
		}
	}

	void AppendAttributeLines(FString& Tooltip, const UDOItemFragment_Equipment& Fragment, int32 UpgradeLevel)
	{
		TMap<FGameplayTag, float> Values;
		BuildEquipmentAttributes(Fragment, Values);
		if (Values.IsEmpty())
		{
			return;
		}

		Tooltip += TEXT("\n基础属性：");
		const float Scale = 1.0f + FMath::Max(0, UpgradeLevel) * 0.05f;
		for (const TPair<FGameplayTag, float>& Pair : Values)
		{
			Tooltip += FString::Printf(TEXT("\n%s  %+0.1f"), *TagLeaf(Pair.Key), Pair.Value * Scale);
		}
	}
}

FText UDOItemTooltipViewModel::BuildForInventorySlot(const FDOInventorySlotViewModel& Slot, const UDOInventoryViewModel* InventoryViewModel)
{
	if (Slot.bIsEmpty)
	{
		return FText::FromString(TEXT("空槽"));
	}

	const UDOItemDefinition* Definition = InventoryViewModel && Slot.Item.DefinitionId.IsValid()
		? UDOItemDefinitionSubsystem::ResolveItemDefinition(InventoryViewModel, Slot.Item.DefinitionId)
		: nullptr;

	FString Tooltip = Slot.DisplayName.ToString();
	Tooltip += FString::Printf(TEXT("\n品质：%s"), *TagLeaf(Slot.Rarity));
	Tooltip += FString::Printf(TEXT("\n类型：%s"), *TagLeaf(Slot.ItemType));
	Tooltip += FString::Printf(TEXT("\n数量：%d"), Slot.Item.StackCount);
	Tooltip += FString::Printf(TEXT("\n背包槽位：%d"), Slot.Item.SlotIndex + 1);
	if (Definition && !Definition->Description.IsEmpty())
	{
		Tooltip += FString::Printf(TEXT("\n\n%s"), *Definition->Description.ToString());
	}

	if (const UDOItemFragment_Inventory* InventoryFragment = Definition ? Definition->FindFragment<UDOItemFragment_Inventory>() : nullptr)
	{
		if (InventoryFragment->bBindOnPickup) Tooltip += TEXT("\n拾取绑定");
		if (!InventoryFragment->bCanDiscard) Tooltip += TEXT("\n不可丢弃");
		if (!InventoryFragment->bCanSell) Tooltip += TEXT("\n不可出售");
	}

	if (const UDOItemFragment_Equipment* EquipmentFragment = Definition ? Definition->FindFragment<UDOItemFragment_Equipment>() : nullptr)
	{
		Tooltip += FString::Printf(TEXT("\n部位：%s"), *TagLeaf(EquipmentFragment->EquipmentSlotTag));
		Tooltip += FString::Printf(TEXT("\n需求等级：%d"), EquipmentFragment->RequiredLevel);
		if (!EquipmentFragment->RequiredProfessionQuery.IsEmpty()) Tooltip += TEXT("\n职业限制：有");
		if (Slot.Item.UpgradeLevel > 0) Tooltip += FString::Printf(TEXT("\n强化：+%d"), Slot.Item.UpgradeLevel);
		if (EquipmentFragment->MaxDurability > 0)
		{
			Tooltip += FString::Printf(TEXT("\n耐久：%d / %d"), Slot.Item.CurrentDurability, EquipmentFragment->MaxDurability);
		}
		AppendAttributeLines(Tooltip, *EquipmentFragment, Slot.Item.UpgradeLevel);

		if (InventoryViewModel && InventoryViewModel->GetEquipmentComponent())
		{
			if (const FDOEquippedItemEntry* Equipped = InventoryViewModel->GetEquipmentComponent()->FindEquippedBySlot(EquipmentFragment->EquipmentSlotTag))
			{
				TMap<FGameplayTag, float> NewValues;
				TMap<FGameplayTag, float> CurrentValues;
				BuildEquipmentAttributes(*EquipmentFragment, NewValues);
				if (const UDOItemDefinition* CurrentDefinition = UDOItemDefinitionSubsystem::ResolveItemDefinition(InventoryViewModel, Equipped->Item.DefinitionId))
				{
					if (const UDOItemFragment_Equipment* CurrentEquipment = CurrentDefinition->FindFragment<UDOItemFragment_Equipment>())
					{
						BuildEquipmentAttributes(*CurrentEquipment, CurrentValues);
					}
				}
				TSet<FGameplayTag> AttributeTags;
				for (const TPair<FGameplayTag, float>& Pair : NewValues) AttributeTags.Add(Pair.Key);
				for (const TPair<FGameplayTag, float>& Pair : CurrentValues) AttributeTags.Add(Pair.Key);
				Tooltip += TEXT("\n\n对比当前装备：");
				for (const FGameplayTag& AttributeTag : AttributeTags)
				{
					const float NewValue = NewValues.FindRef(AttributeTag) * (1.0f + FMath::Max(0, Slot.Item.UpgradeLevel) * 0.05f);
					const float CurrentValue = CurrentValues.FindRef(AttributeTag) * (1.0f + FMath::Max(0, Equipped->Item.UpgradeLevel) * 0.05f);
					Tooltip += FString::Printf(TEXT("\n%s  %+0.1f"), *TagLeaf(AttributeTag), NewValue - CurrentValue);
				}
			}
		}
	}

	if (const UDOItemFragment_Consumable* ConsumableFragment = Definition ? Definition->FindFragment<UDOItemFragment_Consumable>() : nullptr)
	{
		Tooltip += TEXT("\n使用效果：使用后生效");
		if (ConsumableFragment->Cooldown.CooldownTag.IsValid())
		{
			Tooltip += FString::Printf(TEXT("\n公共冷却：%s"), *TagLeaf(ConsumableFragment->Cooldown.CooldownTag));
		}
	}

	for (const FDOItemAffixRoll& Affix : Slot.Item.Affixes)
	{
		Tooltip += FString::Printf(TEXT("\n%s  %+0.1f"), *TagLeaf(Affix.AffixTag), Affix.Magnitude);
	}
	return FText::FromString(Tooltip);
}

FText UDOItemTooltipViewModel::BuildForEquipmentSlot(const FGameplayTag& SlotTag, const FText& DisplayName, const UDOInventoryViewModel* InventoryViewModel)
{
	if (InventoryViewModel)
	{
		if (const FDOEquipmentSlotViewModel* Slot = InventoryViewModel->FindEquipmentSlot(SlotTag))
		{
			return FText::Format(FText::FromString(TEXT("{0}\n{1}")), DisplayName, FText::FromName(Slot->Item.DefinitionId.PrimaryAssetName));
		}
	}
	return FText::Format(FText::FromString(TEXT("{0}\n空槽\n拖入对应部位装备")), DisplayName);
}
