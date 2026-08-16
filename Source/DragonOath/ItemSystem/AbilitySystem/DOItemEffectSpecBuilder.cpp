#include "ItemSystem/AbilitySystem/DOItemEffectSpecBuilder.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/AbilitySystem/DOItemGameplayEffects.h"

namespace
{
	/** 统一写入一个数值型 SetByCaller，避免遗漏未配置字段。 */
	void SetMagnitude(FGameplayEffectSpec& Spec, const FGameplayTag& DataTag, const float Value)
	{
		Spec.SetSetByCallerMagnitude(DataTag, FMath::IsFinite(Value) ? Value : 0.0f);
	}

	void AddAffixMagnitude(FDOAttributeModifierValues& Values, const FDOItemAffixRoll& Affix)
	{
		if (!Affix.AffixTag.IsValid() || !FMath::IsFinite(Affix.Magnitude))
		{
			return;
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

bool FDOItemEffectSpecBuilder::InitializeSpec(
	UDOAbilitySystemComponent& ASC,
	UObject& SourceObject,
	const TSubclassOf<UGameplayEffect> EffectClass,
	FGameplayEffectSpecHandle& OutSpec)
{
	OutSpec = FGameplayEffectSpecHandle();
	if (!ASC.IsOwnerActorAuthoritative() || !ASC.AbilityActorInfo.IsValid() || !EffectClass)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = ASC.MakeEffectContext();
	EffectContext.AddSourceObject(&SourceObject);
	OutSpec = ASC.MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
	return OutSpec.IsValid() && OutSpec.Data.IsValid();
}

void FDOItemEffectSpecBuilder::WriteAttributeMagnitudes(
	FGameplayEffectSpec& Spec,
	const FDOAttributeModifierValues& Values,
	const float Scale)
{
	const float SafeScale = FMath::IsFinite(Scale) ? Scale : 1.0f;
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::AttackPower, Values.AttackPower * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::DefensePower, Values.DefensePower * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::MaxHealth, Values.MaxHealth * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::MaxMana, Values.MaxMana * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::CriticalRating, Values.CriticalRating * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::HitRating, Values.HitRating * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::EvasionRating, Values.EvasionRating * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::AttackSpeed, Values.AttackSpeed * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::MoveSpeed, Values.MoveSpeed * SafeScale);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::Equipment::LifeStealRate, Values.LifeStealRate * SafeScale);
}

void FDOItemEffectSpecBuilder::WriteRestoreMagnitudes(FGameplayEffectSpec& Spec, const FDOResourceRestoreValues& Values)
{
	SetMagnitude(Spec, DragonOathGameplayTags::Data::ItemUse::Healing, Values.Healing);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::ItemUse::ManaRestore, Values.ManaRestore);
	SetMagnitude(Spec, DragonOathGameplayTags::Data::ItemUse::StaminaRestore, Values.StaminaRestore);
}

bool FDOItemEffectSpecBuilder::BuildEquipmentSpec(
	UDOAbilitySystemComponent& ASC,
	UObject& SourceObject,
	const FDOAttributeModifierValues& Values,
	const TArray<FDOItemAffixRoll>& Affixes,
	const float UpgradeScale,
	FGameplayEffectSpecHandle& OutSpec)
{
	if (!FMath::IsFinite(UpgradeScale) || UpgradeScale < 0.0f
		|| !InitializeSpec(ASC, SourceObject, UDOEquipmentAttributeEffect::StaticClass(), OutSpec))
	{
		return false;
	}

	FDOAttributeModifierValues CombinedValues = Values;
	for (const FDOItemAffixRoll& Affix : Affixes)
	{
		AddAffixMagnitude(CombinedValues, Affix);
	}
	WriteAttributeMagnitudes(*OutSpec.Data.Get(), CombinedValues, UpgradeScale);
	return true;
}

bool FDOItemEffectSpecBuilder::BuildInstantRestoreSpec(
	UDOAbilitySystemComponent& ASC,
	UObject& SourceObject,
	const FDOResourceRestoreValues& Values,
	FGameplayEffectSpecHandle& OutSpec)
{
	if (Values.IsNearlyZero() || !InitializeSpec(ASC, SourceObject, UDOItemInstantRestoreEffect::StaticClass(), OutSpec))
	{
		return false;
	}

	WriteRestoreMagnitudes(*OutSpec.Data.Get(), Values);
	return true;
}

bool FDOItemEffectSpecBuilder::BuildTimedModifierSpec(
	UDOAbilitySystemComponent& ASC,
	UObject& SourceObject,
	const FDOItemTimedModifierValues& Values,
	FGameplayEffectSpecHandle& OutSpec)
{
	if (!FMath::IsFinite(Values.DurationSeconds) || Values.DurationSeconds <= 0.0f
		|| Values.Modifiers.IsNearlyZero() && Values.GrantedTags.IsEmpty()
		|| !InitializeSpec(ASC, SourceObject, UDOItemTimedAttributeEffect::StaticClass(), OutSpec))
	{
		return false;
	}

	WriteAttributeMagnitudes(*OutSpec.Data.Get(), Values.Modifiers, 1.0f);
	SetMagnitude(*OutSpec.Data.Get(), DragonOathGameplayTags::Data::ItemUse::Duration, Values.DurationSeconds);
	OutSpec.Data->SetDuration(Values.DurationSeconds, true);
	OutSpec.Data->DynamicGrantedTags.AppendTags(Values.GrantedTags);
	return true;
}

bool FDOItemEffectSpecBuilder::BuildCooldownSpec(
	UDOAbilitySystemComponent& ASC,
	UObject& SourceObject,
	const FDOItemCooldownConfig& Cooldown,
	FGameplayEffectSpecHandle& OutSpec)
{
	if (!Cooldown.IsEnabled() || !InitializeSpec(ASC, SourceObject, UDOItemCooldownEffect::StaticClass(), OutSpec))
	{
		return false;
	}

	SetMagnitude(*OutSpec.Data.Get(), DragonOathGameplayTags::Data::ItemUse::CooldownDuration, Cooldown.DurationSeconds);
	OutSpec.Data->SetDuration(Cooldown.DurationSeconds, true);
	OutSpec.Data->DynamicGrantedTags.AddTag(Cooldown.CooldownTag);
	return true;
}
