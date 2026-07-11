#include "AbilitySystem/Attributes/DOResourceSet.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOResourceSet)

UDOResourceSet::UDOResourceSet()
{
	InitMana(100.0f);
	InitMaxMana(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitManaRegen(0.0f);
}

void UDOResourceSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDOResourceSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOResourceSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOResourceSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDOResourceSet, MaxStamina, COND_None, REPNOTIFY_Always);
	// 魔法回复只影响 Owner 的 Periodic GE 计算，仅同步给 Owner 即可。
	DOREPLIFETIME_CONDITION_NOTIFY(UDOResourceSet, ManaRegen, COND_OwnerOnly, REPNOTIFY_Always);
}

void UDOResourceSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDOResourceSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UDOResourceSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 最大值降低时，把当前资源值同步压回新上限。
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

void UDOResourceSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOResourceSet, Mana, OldMana);
}

void UDOResourceSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOResourceSet, MaxMana, OldMaxMana);
}

void UDOResourceSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOResourceSet, Stamina, OldStamina);
}

void UDOResourceSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOResourceSet, MaxStamina, OldMaxStamina);
}

void UDOResourceSet::OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDOResourceSet, ManaRegen, OldManaRegen);
}

void UDOResourceSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
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
	else if (Attribute == GetManaRegenAttribute())
	{
		// 回复速率不允许为负
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
