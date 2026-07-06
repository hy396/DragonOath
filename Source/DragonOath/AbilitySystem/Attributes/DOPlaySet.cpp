#include "AbilitySystem/Attributes/DOPlaySet.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlaySet)

UDOPlaySet::UDOPlaySet()
{
	InitMana(100.0f);
	InitMaxMana(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitAttackPower(10.0f);
	InitDefensePower(0.0f);
}

void UDOPlaySet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDOPlaySet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOPlaySet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOPlaySet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOPlaySet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOPlaySet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOPlaySet, DefensePower, COND_None, REPNOTIFY_Always);
}

void UDOPlaySet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDOPlaySet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDOPlaySet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 最大值降低时，把当前资源值同步压回新上限。
	// 这里使用 ASC 修改属性，保证属性变更通知和复制流程仍然走 GAS。
	if (Attribute == GetMaxManaAttribute() && GetMana() > NewValue)
	{
		if (UDOAbilitySystemComponent* DOASC = GetUDOAbilitySystemComponent())
		{
			DOASC->ApplyModToAttribute(GetManaAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
	else if (Attribute == GetMaxStaminaAttribute() && GetStamina() > NewValue)
	{
		if (UDOAbilitySystemComponent* DOASC = GetUDOAbilitySystemComponent())
		{
			DOASC->ApplyModToAttribute(GetStaminaAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}

void UDOPlaySet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOPlaySet, Mana, OldMana);
}

void UDOPlaySet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOPlaySet, MaxMana, OldMaxMana);
}

void UDOPlaySet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOPlaySet, Stamina, OldStamina);
}

void UDOPlaySet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOPlaySet, MaxStamina, OldMaxStamina);
}

void UDOPlaySet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOPlaySet, AttackPower, OldAttackPower);
}

void UDOPlaySet::OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOPlaySet, DefensePower, OldDefensePower);
}

void UDOPlaySet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// AttributeSet 只负责保证数值合法；复杂战斗公式放到 GameplayEffect / ExecutionCalculation 更好维护。
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetAttackPowerAttribute() || Attribute == GetDefensePowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
