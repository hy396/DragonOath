// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"

#include "DOExecutionCalculation_Damage.generated.h"

/**
 * 基础伤害执行计算。
 *
 * 已实现（第二阶段）：
 * - 从 SetByCaller 读取 SkillBaseDamage 和 SkillDamageMultiplier
 * - 从攻击者读取 AttackPower / CriticalRating / CritDamageRate / HitRating
 * - 从目标读取 DefensePower / EvasionRating
 * - 基础伤害：max(1, (SkillBaseDamage + AttackPower) - DefensePower) * SkillDamageMultiplier
 * - 命中判定（仅 Damage.Type.Monster / Damage.Type.Pet）
 * - 暴击判定（CriticalRating 换算暴击率，命中则乘 CritDamageRate）
 * - 输出到目标的 Damage Meta Attribute（由 UDOHealthSet::PostGameplayEffectExecute 处理）
 * - 暴击/格挡等判定结果写回 FDOGameplayEffectContext，随 GE 网络同步到客户端供 GameplayCue 读取
 *
 * 吸血（LifeSteal）：不在本类处理，也不在 ApplyDamageToTarget 估算。改为在
 * UDOHealthSet::PostGameplayEffectExecute 用真实伤害 LocalDamage 计算（LifeStealRate * LocalDamage），
 * 仅服务端执行，数据正确且天然避免客户端双重回血。
 *
 * 后续 Phase 4 扩展：元素伤害
 */
UCLASS()
class DRAGONOATH_API UDOExecutionCalculation_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDOExecutionCalculation_Damage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	// 缓存的属性捕获定义，避免每次 Execute 重新创建
	FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
	FGameplayEffectAttributeCaptureDefinition DefensePowerDef;
	FGameplayEffectAttributeCaptureDefinition CriticalRatingDef;
	FGameplayEffectAttributeCaptureDefinition CritDamageRateDef;
	FGameplayEffectAttributeCaptureDefinition HitRatingDef;
	FGameplayEffectAttributeCaptureDefinition EvasionRatingDef;
};