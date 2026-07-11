// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/DOGameplayAbility.h"
#include "DOGameplayAbility_DashAttack.generated.h"

class UAnimMontage;

/**
 * 冲刺攻击技能。
 *
 * 功能：
 * - 只在冲刺攻击窗口期间（Status.DashAttackWindow）可以激活
 * - 和普攻共用 InputTag.Ability.Primary
 * - 激活时消耗 DashAttackWindow 标签，避免重复触发
 * - 播放冲刺攻击动画并应用伤害
 *
 * 网络策略：
 * - LocalPredicted：客户端预测激活，立即播放动画
 * - InstancedPerActor：每个 ASC 一份实例
 */
UCLASS(Abstract, Blueprintable)
class DRAGONOATH_API UDOGameplayAbility_DashAttack : public UDOGameplayAbility
{
	GENERATED_BODY()

public:
	UDOGameplayAbility_DashAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~End of UGameplayAbility interface

protected:
	// 动画完成时的回调
	void OnMontageCompleted();
	void OnMontageCancelled();

	// 冲刺攻击动画蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|DashAttack")
	TObjectPtr<UAnimMontage> DashAttackMontage;
};