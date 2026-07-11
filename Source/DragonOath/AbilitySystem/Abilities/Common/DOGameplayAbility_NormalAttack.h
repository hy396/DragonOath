// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/Core/DOGameplayAbility.h"
#include "DOGameplayAbility_NormalAttack.generated.h"

class UAnimMontage;

/**
 * 普通攻击技能基类。
 *
 * 功能：
 * - 基础近战攻击
 * - 冲刺攻击窗口期间被阻止（让冲刺攻击优先）
 * - 可配置动画、伤害等参数
 *
 * 网络策略：
 * - LocalPredicted：客户端预测攻击
 * - InstancedPerActor：每个 ASC 一份实例
 */
UCLASS(Abstract, Blueprintable)
class DRAGONOATH_API UDOGameplayAbility_NormalAttack : public UDOGameplayAbility
{
	GENERATED_BODY()

public:
	UDOGameplayAbility_NormalAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~End of UGameplayAbility interface

protected:
	// 动画完成时的回调
	void OnMontageCompleted();
	void OnMontageCancelled();

	// 普攻动画蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|NormalAttack")
	TObjectPtr<UAnimMontage> AttackMontage;
};