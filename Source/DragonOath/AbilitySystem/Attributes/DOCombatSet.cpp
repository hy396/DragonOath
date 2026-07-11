#include "AbilitySystem/Attributes/DOCombatSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOCombatSet)

UDOCombatSet::UDOCombatSet()
{
	InitAttackPower(10.0f);
	InitDefensePower(0.0f);
	InitMoveSpeed(500.0f);
	InitCriticalRating(0.0f);
	InitCritDamageRate(1.5f);
	InitHitRating(0.0f);
	InitEvasionRating(0.0f);
	InitAttackSpeed(1.0f);
	InitLifeStealRate(0.0f);
}

void UDOCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, DefensePower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, CriticalRating, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, CritDamageRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, HitRating, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, EvasionRating, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOCombatSet, LifeStealRate, COND_None, REPNOTIFY_Always);
}

void UDOCombatSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDOCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDOCombatSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, AttackPower, OldAttackPower);
}

void UDOCombatSet::OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, DefensePower, OldDefensePower);
}

void UDOCombatSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, MoveSpeed, OldMoveSpeed);
}

void UDOCombatSet::OnRep_CriticalRating(const FGameplayAttributeData& OldCriticalRating)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, CriticalRating, OldCriticalRating);
}

void UDOCombatSet::OnRep_CritDamageRate(const FGameplayAttributeData& OldCritDamageRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, CritDamageRate, OldCritDamageRate);
}

void UDOCombatSet::OnRep_HitRating(const FGameplayAttributeData& OldHitRating)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, HitRating, OldHitRating);
}

void UDOCombatSet::OnRep_EvasionRating(const FGameplayAttributeData& OldEvasionRating)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, EvasionRating, OldEvasionRating);
}

void UDOCombatSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, AttackSpeed, OldAttackSpeed);
}

void UDOCombatSet::OnRep_LifeStealRate(const FGameplayAttributeData& OldLifeStealRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOCombatSet, LifeStealRate, OldLifeStealRate);
}

void UDOCombatSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// 属性集只做 Clamp，复杂战斗逻辑留给 ExecutionCalculation / GE。
	if (Attribute == GetMoveSpeedAttribute())
	{
		// 移动速度下限 1.0，避免角色完全无法移动
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetCritDamageRateAttribute())
	{
		// 暴击倍率至少 1.0，避免低于基础伤害
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else
	{
		// 其余战斗属性不允许为负：攻击/防御/致命/命中/闪避/攻速/吸血
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
