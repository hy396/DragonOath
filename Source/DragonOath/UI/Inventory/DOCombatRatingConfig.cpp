#include "UI/Inventory/DOCombatRatingConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOCombatRatingConfig)

namespace
{
	const UDOCombatRatingConfig* ResolveConfig(const UDOCombatRatingConfig* Config)
	{
		return Config ? Config : GetDefault<UDOCombatRatingConfig>();
	}
}

int32 UDOCombatRatingLibrary::CalculateCombatPower(const FDOCombatRatingInput& Input, const UDOCombatRatingConfig* Config)
{
	const UDOCombatRatingConfig* EffectiveConfig = ResolveConfig(Config);
	const float Value =
		Input.AttackPower * EffectiveConfig->CombatAttackPowerWeight
		+ Input.CriticalRating * EffectiveConfig->CombatCriticalRatingWeight
		+ Input.HitRating * EffectiveConfig->CombatHitRatingWeight
		+ Input.AttackSpeed * EffectiveConfig->CombatAttackSpeedWeight
		+ Input.LifeStealRate * EffectiveConfig->CombatLifeStealRateWeight;
	return FMath::Max(0, FMath::RoundToInt(Value));
}

int32 UDOCombatRatingLibrary::CalculateGuardPower(const FDOCombatRatingInput& Input, const UDOCombatRatingConfig* Config)
{
	const UDOCombatRatingConfig* EffectiveConfig = ResolveConfig(Config);
	const float Value =
		Input.DefensePower * EffectiveConfig->GuardDefensePowerWeight
		+ Input.MaxHealth * EffectiveConfig->GuardMaxHealthWeight
		+ Input.MaxMana * EffectiveConfig->GuardMaxManaWeight
		+ Input.EvasionRating * EffectiveConfig->GuardEvasionRatingWeight
		+ Input.MoveSpeed * EffectiveConfig->GuardMoveSpeedWeight;
	return FMath::Max(0, FMath::RoundToInt(Value));
}
