#include "ItemSystem/AbilitySystem/DOItemGameplayEffects.h"

#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"
#include "AbilitySystem/Core/DOGameplayTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemGameplayEffects)

namespace
{
	/** 给原生 GE 增加一个由 SetByCaller 提供数值的属性 Modifier。 */
	void AddSetByCallerModifier(UGameplayEffect& Effect, const FGameplayAttribute& Attribute, const FGameplayTag& DataTag)
	{
		FGameplayModifierInfo& Modifier = Effect.Modifiers.AddDefaulted_GetRef();
		Modifier.Attribute = Attribute;
		Modifier.ModifierOp = EGameplayModOp::Additive;

		FSetByCallerFloat SetByCaller;
		SetByCaller.DataTag = DataTag;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	}

	/** 给持续型 GE 设置由 SetByCaller 提供的持续时间。 */
	void SetDurationByCaller(UGameplayEffect& Effect, const FGameplayTag& DataTag)
	{
		FSetByCallerFloat SetByCaller;
		SetByCaller.DataTag = DataTag;
		Effect.DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	}
}

UDOEquipmentAttributeEffect::UDOEquipmentAttributeEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	AddSetByCallerModifier(*this, UDOCombatSet::GetAttackPowerAttribute(), DragonOathGameplayTags::Data::Equipment::AttackPower);
	AddSetByCallerModifier(*this, UDOCombatSet::GetDefensePowerAttribute(), DragonOathGameplayTags::Data::Equipment::DefensePower);
	AddSetByCallerModifier(*this, UDOHealthSet::GetMaxHealthAttribute(), DragonOathGameplayTags::Data::Equipment::MaxHealth);
	AddSetByCallerModifier(*this, UDOResourceSet::GetMaxManaAttribute(), DragonOathGameplayTags::Data::Equipment::MaxMana);
	AddSetByCallerModifier(*this, UDOCombatSet::GetCriticalRatingAttribute(), DragonOathGameplayTags::Data::Equipment::CriticalRating);
	AddSetByCallerModifier(*this, UDOCombatSet::GetHitRatingAttribute(), DragonOathGameplayTags::Data::Equipment::HitRating);
	AddSetByCallerModifier(*this, UDOCombatSet::GetEvasionRatingAttribute(), DragonOathGameplayTags::Data::Equipment::EvasionRating);
	AddSetByCallerModifier(*this, UDOCombatSet::GetAttackSpeedAttribute(), DragonOathGameplayTags::Data::Equipment::AttackSpeed);
	AddSetByCallerModifier(*this, UDOCombatSet::GetMoveSpeedAttribute(), DragonOathGameplayTags::Data::Equipment::MoveSpeed);
	AddSetByCallerModifier(*this, UDOCombatSet::GetLifeStealRateAttribute(), DragonOathGameplayTags::Data::Equipment::LifeStealRate);
}

UDOItemInstantRestoreEffect::UDOItemInstantRestoreEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	AddSetByCallerModifier(*this, UDOHealthSet::GetHealingAttribute(), DragonOathGameplayTags::Data::ItemUse::Healing);
	AddSetByCallerModifier(*this, UDOResourceSet::GetManaAttribute(), DragonOathGameplayTags::Data::ItemUse::ManaRestore);
	AddSetByCallerModifier(*this, UDOResourceSet::GetStaminaAttribute(), DragonOathGameplayTags::Data::ItemUse::StaminaRestore);
}

UDOItemTimedAttributeEffect::UDOItemTimedAttributeEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	SetDurationByCaller(*this, DragonOathGameplayTags::Data::ItemUse::Duration);

	AddSetByCallerModifier(*this, UDOCombatSet::GetAttackPowerAttribute(), DragonOathGameplayTags::Data::Equipment::AttackPower);
	AddSetByCallerModifier(*this, UDOCombatSet::GetDefensePowerAttribute(), DragonOathGameplayTags::Data::Equipment::DefensePower);
	AddSetByCallerModifier(*this, UDOHealthSet::GetMaxHealthAttribute(), DragonOathGameplayTags::Data::Equipment::MaxHealth);
	AddSetByCallerModifier(*this, UDOResourceSet::GetMaxManaAttribute(), DragonOathGameplayTags::Data::Equipment::MaxMana);
	AddSetByCallerModifier(*this, UDOCombatSet::GetCriticalRatingAttribute(), DragonOathGameplayTags::Data::Equipment::CriticalRating);
	AddSetByCallerModifier(*this, UDOCombatSet::GetHitRatingAttribute(), DragonOathGameplayTags::Data::Equipment::HitRating);
	AddSetByCallerModifier(*this, UDOCombatSet::GetEvasionRatingAttribute(), DragonOathGameplayTags::Data::Equipment::EvasionRating);
	AddSetByCallerModifier(*this, UDOCombatSet::GetAttackSpeedAttribute(), DragonOathGameplayTags::Data::Equipment::AttackSpeed);
	AddSetByCallerModifier(*this, UDOCombatSet::GetMoveSpeedAttribute(), DragonOathGameplayTags::Data::Equipment::MoveSpeed);
	AddSetByCallerModifier(*this, UDOCombatSet::GetLifeStealRateAttribute(), DragonOathGameplayTags::Data::Equipment::LifeStealRate);
}

UDOItemCooldownEffect::UDOItemCooldownEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	SetDurationByCaller(*this, DragonOathGameplayTags::Data::ItemUse::CooldownDuration);
}
