#pragma once

#include "AbilitySystem/Attributes/DOAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "DOCombatSet.generated.h"

/**
 * 战斗属性集：攻击/防御、移动速度，以及进阶战斗属性（暴击、命中、闪避、攻速、吸血）。
 *
 * 第二阶段从 DOPlaySet 拆分而来。所有伤害公式的输入属性集中放在这里，
 * 复杂计算交给 ExecutionCalculation 或专用 GE，属性集本身只做 Clamp。
 */
UCLASS()
class DRAGONOATH_API UDOCombatSet : public UDOAttributeSet
{
	GENERATED_BODY()

public:
	UDOCombatSet();

	// 攻击力：伤害计算输入属性，所有技能共用。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, AttackPower)

	// 防御力：减伤计算输入属性。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, DefensePower)

	// 移动速度：属性值直接映射到 CharacterMovementComponent::MaxWalkSpeed，通过 delegate 桥接。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, MoveSpeed)

	// 致命（暴击值）：数值型属性，通过公式换算为暴击率，不直接显示百分比。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_CriticalRating)
	FGameplayAttributeData CriticalRating;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, CriticalRating)

	// 暴击伤害倍率：暴击时伤害乘以此值，默认 1.5。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_CritDamageRate)
	FGameplayAttributeData CritDamageRate;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, CritDamageRate)

	// 命中值：换算为命中率，用于怪物/召唤物攻击玩家的命中判定。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_HitRating)
	FGameplayAttributeData HitRating;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, HitRating)

	// 闪避值：换算为闪避率，用于被怪物/召唤物攻击时的闪避判定。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_EvasionRating)
	FGameplayAttributeData EvasionRating;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, EvasionRating)

	// 攻击速度：影响攻击动画速度和技能前摇，子类 PlayMontage 时作为播放速率。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, AttackSpeed)

	// 吸血：按最终伤害比例回复生命的系数（0~1）。
	UPROPERTY(BlueprintReadOnly, Category = "DO|Combat", ReplicatedUsing = OnRep_LifeStealRate)
	FGameplayAttributeData LifeStealRate;
	ATTRIBUTE_ACCESSORS(UDOCombatSet, LifeStealRate)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

protected:
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	void OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	void OnRep_CriticalRating(const FGameplayAttributeData& OldCriticalRating);

	UFUNCTION()
	void OnRep_CritDamageRate(const FGameplayAttributeData& OldCritDamageRate);

	UFUNCTION()
	void OnRep_HitRating(const FGameplayAttributeData& OldHitRating);

	UFUNCTION()
	void OnRep_EvasionRating(const FGameplayAttributeData& OldEvasionRating);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);

	UFUNCTION()
	void OnRep_LifeStealRate(const FGameplayAttributeData& OldLifeStealRate);

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
