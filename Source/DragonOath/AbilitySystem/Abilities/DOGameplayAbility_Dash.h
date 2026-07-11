// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/DOGameplayAbility.h"
#include "DOGameplayAbility_Dash.generated.h"

class UAnimMontage;
class UCurveFloat;

/**
 * 冲刺技能。
 *
 * 功能：
 * - 双击 A/D 触发快速位移
 * - 冲刺期间无敌（通过 ActivationOwnedTags 自动管理）
 * - 消耗体力
 * - 冲刺结束后施加冲刺攻击窗口（Status.DashAttackWindow）
 *
 * 网络策略：
 * - LocalPredicted：客户端预测位移，服务端最终确认
 * - InstancedPerActor：每个 ASC 一份实例
 */
UCLASS(Abstract, Blueprintable)
class DRAGONOATH_API UDOGameplayAbility_Dash : public UDOGameplayAbility
{
	GENERATED_BODY()

public:
	UDOGameplayAbility_Dash(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

protected:
	// RootMotion Task 完成时的回调
	void OnDashFinished();

	// 获取冲刺方向。横版游戏只有左右，角色朝向即为冲刺方向。
	FVector GetDashDirection() const;

	// 获取角色最大移动速度，用于 RootMotion 结束时的速度限制
	float GetMaxSpeed() const;

	// 冲刺力度（RootMotion 恒力大小）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	float DashStrength = 2000.0f;

	// 冲刺持续时间（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	float DashDuration = 0.3f;

	// 冲刺攻击窗口时长（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	float DashAttackWindowDuration = 0.4f;

	// 冲刺期间是否无敌, 默认无敌
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	// bool bInvincibleDuringDash = true;

	// 冲刺动画蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	TObjectPtr<UAnimMontage> DashMontage;

	// 冲刺力度曲线（可选）。X 轴 [0.0, 1.0]，0 是冲刺开始，1 是冲刺结束。
	// 用于实现前段加速后段减速的手感。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	TObjectPtr<UCurveFloat> StrengthOverTimeCurve;

	// 冲刺攻击窗口 GE（授予 Status.DashAttackWindow）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Dash")
	TSubclassOf<UGameplayEffect> DashAttackWindowEffectClass;
};