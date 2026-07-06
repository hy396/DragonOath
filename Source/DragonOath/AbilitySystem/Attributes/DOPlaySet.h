#pragma once

#include "AbilitySystem/Attributes/DOAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "DOPlaySet.generated.h"

/**
 * 玩家资源和基础战斗数值属性集。
 *
 * Health 独立放在 UDOHealthSet；本类放技能消耗和伤害公式会读取的基础数值。
 * 小怪以后也可以复用本属性集，只要它们的 ASC 持有这个 AttributeSet。
 */
UCLASS()
class DRAGONOATH_API UDOPlaySet : public UDOAttributeSet
{
	GENERATED_BODY()

public:
	UDOPlaySet();

	// 法力值：通常用于技能消耗。当前值应始终保持在 0 到 MaxMana 之间。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Play", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UDOPlaySet, Mana)

	// 最大法力值：改变后会在 PostAttributeChange 中回压 Mana，避免当前值超过上限。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Play", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UDOPlaySet, MaxMana)

	// 体力值：适合冲刺、闪避、格挡等频繁消耗的动作资源。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Play", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UDOPlaySet, Stamina)

	// 最大体力值：改变后会同步限制当前 Stamina。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Play", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UDOPlaySet, MaxStamina)

	// 攻击力：作为伤害计算的输入属性，最终公式建议放在 ExecutionCalculation 或专用伤害逻辑里。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Play", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDOPlaySet, AttackPower)

	// 防御力：作为减伤计算的输入属性，不建议在 AttributeSet 里直接写复杂战斗公式。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Play", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS(UDOPlaySet, DefensePower)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

protected:
	// OnRep 必须调用 GAMEPLAYATTRIBUTE_REPNOTIFY，让 ASC 的属性变更委托和 UI 监听保持同步。
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	void OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower);

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
