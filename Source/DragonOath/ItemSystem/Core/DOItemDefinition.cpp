#include "ItemSystem/Core/DOItemDefinition.h"

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemDefinition)

namespace
{
	bool IsValidItemTypeTag(const FGameplayTag& Tag)
	{
		return Tag == DragonOathGameplayTags::Item::Type::Equipment
			|| Tag == DragonOathGameplayTags::Item::Type::Consumable
			|| Tag == DragonOathGameplayTags::Item::Type::Material
			|| Tag == DragonOathGameplayTags::Item::Type::Quest;
	}

	bool IsValidRarityTag(const FGameplayTag& Tag)
	{
		return Tag == DragonOathGameplayTags::Item::Rarity::Common
			|| Tag == DragonOathGameplayTags::Item::Rarity::Uncommon
			|| Tag == DragonOathGameplayTags::Item::Rarity::Rare
			|| Tag == DragonOathGameplayTags::Item::Rarity::Epic
			|| Tag == DragonOathGameplayTags::Item::Rarity::Legendary;
	}

	bool IsValidEquipmentAttributeTag(const FGameplayTag& Tag)
	{
		return Tag == DragonOathGameplayTags::Data::Equipment::AttackPower
			|| Tag == DragonOathGameplayTags::Data::Equipment::DefensePower
			|| Tag == DragonOathGameplayTags::Data::Equipment::MaxHealth
			|| Tag == DragonOathGameplayTags::Data::Equipment::MaxMana
			|| Tag == DragonOathGameplayTags::Data::Equipment::CriticalRating
			|| Tag == DragonOathGameplayTags::Data::Equipment::HitRating
			|| Tag == DragonOathGameplayTags::Data::Equipment::EvasionRating
			|| Tag == DragonOathGameplayTags::Data::Equipment::AttackSpeed
			|| Tag == DragonOathGameplayTags::Data::Equipment::MoveSpeed
			|| Tag == DragonOathGameplayTags::Data::Equipment::LifeStealRate;
	}

	bool AreFiniteNonNegative(const FDOAttributeModifierValues& Values)
	{
		return FMath::IsFinite(Values.AttackPower) && Values.AttackPower >= 0.0f
			&& FMath::IsFinite(Values.DefensePower) && Values.DefensePower >= 0.0f
			&& FMath::IsFinite(Values.MaxHealth) && Values.MaxHealth >= 0.0f
			&& FMath::IsFinite(Values.MaxMana) && Values.MaxMana >= 0.0f
			&& FMath::IsFinite(Values.CriticalRating) && Values.CriticalRating >= 0.0f
			&& FMath::IsFinite(Values.HitRating) && Values.HitRating >= 0.0f
			&& FMath::IsFinite(Values.EvasionRating) && Values.EvasionRating >= 0.0f
			&& FMath::IsFinite(Values.AttackSpeed) && Values.AttackSpeed >= 0.0f
			&& FMath::IsFinite(Values.MoveSpeed) && Values.MoveSpeed >= 0.0f
			&& FMath::IsFinite(Values.LifeStealRate) && Values.LifeStealRate >= 0.0f;
	}

	bool AreFiniteNonNegative(const FDOResourceRestoreValues& Values)
	{
		return FMath::IsFinite(Values.Healing) && Values.Healing >= 0.0f
			&& FMath::IsFinite(Values.ManaRestore) && Values.ManaRestore >= 0.0f
			&& FMath::IsFinite(Values.StaminaRestore) && Values.StaminaRestore >= 0.0f;
	}
}

FPrimaryAssetId UDOItemDefinition::GetPrimaryAssetId() const
{
	static const FPrimaryAssetType ItemDefinitionType(TEXT("ItemDefinition"));
	return FPrimaryAssetId(ItemDefinitionType, GetFName());
}

EDataValidationResult UDOItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (DisplayName.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("ItemDefinition 的 DisplayName 不能为空。")));
		Result = EDataValidationResult::Invalid;
	}
	if (!Icon.ToSoftObjectPath().IsValid())
	{
		Context.AddError(FText::FromString(TEXT("ItemDefinition 必须配置 Icon。")));
		Result = EDataValidationResult::Invalid;
	}
	if (!ItemType.IsValid() || !IsValidItemTypeTag(ItemType))
	{
		Context.AddError(FText::FromString(TEXT("ItemDefinition 必须配置有效的 Item.Type.* 标签。")));
		Result = EDataValidationResult::Invalid;
	}
	if (!Rarity.IsValid() || !IsValidRarityTag(Rarity))
	{
		Context.AddError(FText::FromString(TEXT("ItemDefinition 必须配置有效的 Item.Rarity.* 标签。")));
		Result = EDataValidationResult::Invalid;
	}
	if (MaxStackSize <= 0 || SellPrice < 0)
	{
		Context.AddError(FText::FromString(TEXT("MaxStackSize 必须大于 0，SellPrice 不能为负数。")));
		Result = EDataValidationResult::Invalid;
	}

	bool bHasEquipmentFragment = false;
	bool bHasConsumableFragment = false;
	bool bHasInventoryFragment = false;
	for (int32 Index = 0; Index < Fragments.Num(); ++Index)
	{
		const UDOItemFragment* Fragment = Fragments[Index];
		if (!Fragment)
		{
			Context.AddError(FText::Format(FText::FromString(TEXT("Fragments[{0}] 为空。")), FText::AsNumber(Index)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (const UDOItemFragment_Equipment* EquipmentFragment = Cast<UDOItemFragment_Equipment>(Fragment))
		{
			const bool bIsValidEquipmentSlot =
				EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Head
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Shoulder
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Back
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Chest
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Hands
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Legs
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Feet
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Accessory
				|| EquipmentFragment->EquipmentSlotTag == DragonOathGameplayTags::Equipment::Slot::Weapon;

			if (bHasEquipmentFragment
				|| ItemType != DragonOathGameplayTags::Item::Type::Equipment
				|| !bIsValidEquipmentSlot
				|| EquipmentFragment->RequiredLevel <= 0
				|| EquipmentFragment->MaxDurability < 0)
			{
				Context.AddError(FText::FromString(TEXT("装备 Fragment 重复，或装备部位、需求等级、耐久配置无效。")));
				Result = EDataValidationResult::Invalid;
			}
			bHasEquipmentFragment = true;
			if (!AreFiniteNonNegative(EquipmentFragment->AttributeModifiers)
				|| (EquipmentFragment->AttributeModifiers.IsNearlyZero() && EquipmentFragment->BaseAttributeMagnitudes.Num() == 0))
			{
				Context.AddError(FText::FromString(TEXT("装备必须配置至少一个非零属性，且属性值不能为负数或非法浮点数。")));
				Result = EDataValidationResult::Invalid;
			}
			for (const TPair<FGameplayTag, FScalableFloat>& Attribute : EquipmentFragment->BaseAttributeMagnitudes)
			{
				if (!IsValidEquipmentAttributeTag(Attribute.Key))
				{
					Context.AddError(FText::FromString(TEXT("装备属性必须使用 Data.Equipment.* 标签。")));
					Result = EDataValidationResult::Invalid;
				}
			}
		}

		if (const UDOItemFragment_Consumable* ConsumableFragment = Cast<UDOItemFragment_Consumable>(Fragment))
		{
			const bool bHasLegacyEffect = ConsumableFragment->UseGameplayEffect != nullptr;
			const bool bHasLegacyAbility = ConsumableFragment->UseGameplayAbility != nullptr;
			const bool bHasLegacyEvent = ConsumableFragment->UseEventTag.IsValid();
			const int32 LegacyActionCount = (bHasLegacyEffect ? 1 : 0) + (bHasLegacyAbility ? 1 : 0) + (bHasLegacyEvent ? 1 : 0);
			bool bValidNewAction = false;

			switch (ConsumableFragment->EffectKind)
			{
			case EDOConsumableEffectKind::InstantRestore:
				bValidNewAction = AreFiniteNonNegative(ConsumableFragment->InstantRestore)
					&& !ConsumableFragment->InstantRestore.IsNearlyZero();
				break;
			case EDOConsumableEffectKind::TimedAttributeModifier:
				bValidNewAction = FMath::IsFinite(ConsumableFragment->TimedModifier.DurationSeconds)
					&& ConsumableFragment->TimedModifier.DurationSeconds > 0.0f
					&& AreFiniteNonNegative(ConsumableFragment->TimedModifier.Modifiers)
					&& (!ConsumableFragment->TimedModifier.Modifiers.IsNearlyZero()
						|| !ConsumableFragment->TimedModifier.GrantedTags.IsEmpty());
				break;
			case EDOConsumableEffectKind::GameplayAbility:
				bValidNewAction = ConsumableFragment->UseGameplayAbility != nullptr;
				break;
			case EDOConsumableEffectKind::GameplayEvent:
				bValidNewAction = ConsumableFragment->UseEventTag.IsValid();
				break;
			case EDOConsumableEffectKind::None:
			default:
				break;
			}

			const bool bValidLegacyAction = ConsumableFragment->EffectKind == EDOConsumableEffectKind::None && LegacyActionCount == 1;
			if (bHasConsumableFragment || ItemType != DragonOathGameplayTags::Item::Type::Consumable || (!bValidNewAction && !bValidLegacyAction))
			{
				Context.AddError(FText::FromString(TEXT("消耗品 Fragment 重复、类型不匹配，或必须且只能配置一种使用方式。")));
				Result = EDataValidationResult::Invalid;
			}
			if (ConsumableFragment->EffectKind != EDOConsumableEffectKind::None && bHasLegacyEffect)
			{
				Context.AddError(FText::FromString(TEXT("新消耗品配置不能同时保留旧 UseGameplayEffect。")));
				Result = EDataValidationResult::Invalid;
			}
			const bool bHasCooldownTag = ConsumableFragment->Cooldown.CooldownTag.IsValid();
			const bool bHasCooldownDuration = FMath::IsFinite(ConsumableFragment->Cooldown.DurationSeconds)
				&& ConsumableFragment->Cooldown.DurationSeconds > 0.0f;
			if (bHasCooldownTag != bHasCooldownDuration)
			{
				Context.AddError(FText::FromString(TEXT("消耗品 CooldownTag 和冷却时长必须同时有效。")));
				Result = EDataValidationResult::Invalid;
			}
			bHasConsumableFragment = true;
		}

		if (Cast<UDOItemFragment_Inventory>(Fragment))
		{
			if (bHasInventoryFragment)
			{
				Context.AddError(FText::FromString(TEXT("Inventory Fragment 只能配置一个。")));
				Result = EDataValidationResult::Invalid;
			}
			bHasInventoryFragment = true;
		}
	}

	// 物品类型与专用 Fragment 必须成对出现，避免资产在编辑器中看似合法，
	// 运行时却因为缺少装备或消耗品行为而只能进入背包、无法使用。
	if (ItemType == DragonOathGameplayTags::Item::Type::Equipment && !bHasEquipmentFragment)
	{
		Context.AddError(FText::FromString(TEXT("装备类型物品必须配置一个 Equipment Fragment。")));
		Result = EDataValidationResult::Invalid;
	}
	if (ItemType == DragonOathGameplayTags::Item::Type::Consumable && !bHasConsumableFragment)
	{
		Context.AddError(FText::FromString(TEXT("消耗品类型物品必须配置一个 Consumable Fragment。")));
		Result = EDataValidationResult::Invalid;
	}

	if (bHasEquipmentFragment && MaxStackSize != 1)
	{
		Context.AddError(FText::FromString(TEXT("装备物品的 MaxStackSize 必须为 1。")));
		Result = EDataValidationResult::Invalid;
	}
	if (bHasInventoryFragment)
	{
		const UDOItemFragment_Inventory* InventoryFragment = FindFragment<UDOItemFragment_Inventory>();
		if (InventoryFragment && InventoryFragment->bUnique && MaxStackSize != 1)
		{
			Context.AddError(FText::FromString(TEXT("唯一物品的 MaxStackSize 必须为 1。")));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
