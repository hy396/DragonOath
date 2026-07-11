#pragma once

#include "AbilitySystem/Attributes/DOAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "DOResourceSet.generated.h"

/**
 * 资源属性集：法力、体力及其上限，以及魔法回复。
 *
 * 第二阶段从 DOPlaySet 拆分而来。资源只用于技能消耗与动作消耗，
 * 不直接参与伤害公式；ManaRegen 由回复 Periodic GE 读取后周期性施加到 Mana。
 */
UCLASS()
class DRAGONOATH_API UDOResourceSet : public UDOAttributeSet
{
	GENERATED_BODY()

public:
	UDOResourceSet();

	// 法力值：技能消耗的主要资源。当前值保持在 0 到 MaxMana 之间。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Resource", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UDOResourceSet, Mana)

	// 最大法力值：改变后会在 PostAttributeChange 中回压 Mana，避免当前值超过上限。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Resource", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UDOResourceSet, MaxMana)

	// 体力值：冲刺、闪避、格挡等动作资源。当前值保持在 0 到 MaxStamina 之间。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Resource", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UDOResourceSet, Stamina)

	// 最大体力值：改变后回压 Stamina。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Resource", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UDOResourceSet, MaxStamina)

	// 魔法回复：每秒回复量。仅 Owner 复制，回复 Periodic GE 读取此值后施加到 Mana。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Resource", ReplicatedUsing = OnRep_ManaRegen)
	FGameplayAttributeData ManaRegen;
	ATTRIBUTE_ACCESSORS(UDOResourceSet, ManaRegen)

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
	void OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen);

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
