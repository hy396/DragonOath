// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Pipeline/DOExecutionCalculation_Damage.h"

#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystem/Pipeline/DOGameplayEffectContext.h"
#include "Characters/DOCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOExecutionCalculation_Damage)

namespace
{
	// 从 ASC 的 AvatarActor 读取角色等级，作为伤害公式的等级缩放输入。
	int32 GetActorLevel(const UAbilitySystemComponent* AbilitySystemComponent)
	{
		if (AbilitySystemComponent)
		{
			if (const ADOCharacter* Character = Cast<ADOCharacter>(AbilitySystemComponent->GetAvatarActor()))
			{
				return Character->GetCharacterLevel();
			}
		}
		return 1;
	}
}

UDOExecutionCalculation_Damage::UDOExecutionCalculation_Damage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 攻击方属性（Source）
	AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
		UDOCombatSet::GetAttackPowerAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		false /* bSnapshot */);

	CriticalRatingDef = FGameplayEffectAttributeCaptureDefinition(
		UDOCombatSet::GetCriticalRatingAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		false);

	CritDamageRateDef = FGameplayEffectAttributeCaptureDefinition(
		UDOCombatSet::GetCritDamageRateAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		false);

	HitRatingDef = FGameplayEffectAttributeCaptureDefinition(
		UDOCombatSet::GetHitRatingAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		false);

	// 防御方属性（Target）
	DefensePowerDef = FGameplayEffectAttributeCaptureDefinition(
		UDOCombatSet::GetDefensePowerAttribute(),
		EGameplayEffectAttributeCaptureSource::Target,
		false);

	EvasionRatingDef = FGameplayEffectAttributeCaptureDefinition(
		UDOCombatSet::GetEvasionRatingAttribute(),
		EGameplayEffectAttributeCaptureSource::Target,
		false);

	RelevantAttributesToCapture.Add(AttackPowerDef);
	RelevantAttributesToCapture.Add(DefensePowerDef);
	RelevantAttributesToCapture.Add(CriticalRatingDef);
	RelevantAttributesToCapture.Add(CritDamageRateDef);
	RelevantAttributesToCapture.Add(HitRatingDef);
	RelevantAttributesToCapture.Add(EvasionRatingDef);
}

void UDOExecutionCalculation_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// ==================== 属性快照 ====================
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters CaptureParams;

	float AttackerAttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(AttackPowerDef, CaptureParams, AttackerAttackPower);
	AttackerAttackPower = FMath::Max(0.0f, AttackerAttackPower);

	float TargetDefensePower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DefensePowerDef, CaptureParams, TargetDefensePower);
	TargetDefensePower = FMath::Max(0.0f, TargetDefensePower);

	float AttackerCriticalRating = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalRatingDef, CaptureParams, AttackerCriticalRating);
	AttackerCriticalRating = FMath::Max(0.0f, AttackerCriticalRating);

	float AttackerCritDamageRate = 1.5f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CritDamageRateDef, CaptureParams, AttackerCritDamageRate);
	AttackerCritDamageRate = FMath::Max(1.0f, AttackerCritDamageRate);

	float AttackerHitRating = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HitRatingDef, CaptureParams, AttackerHitRating);
	AttackerHitRating = FMath::Max(0.0f, AttackerHitRating);

	float TargetEvasionRating = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(EvasionRatingDef, CaptureParams, TargetEvasionRating);
	TargetEvasionRating = FMath::Max(0.0f, TargetEvasionRating);

	// ==================== 等级读取 ====================
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	const int32 AttackerLevel = GetActorLevel(SourceASC);
	const int32 DefenderLevel = GetActorLevel(TargetASC);

	// ==================== 伤害类型标记 ====================
	// 这些 Tag 配置在伤害 GE 的 Source Tags 上，这里通过捕获的源标签读取。
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const bool bIsPlayerDamage = SourceTags && SourceTags->HasTag(DragonOathGameplayTags::Damage::TypePlayer);

	// ==================== SetByCaller 读取 ====================
	// 技能通过 SetSetByCallerMagnitude 设置技能基础伤害和倍率
	const float SkillBaseDamage = Spec.GetSetByCallerMagnitude(DragonOathGameplayTags::Data::Damage, false, 0.0f);
	const float SkillDamageMultiplier = Spec.GetSetByCallerMagnitude(DragonOathGameplayTags::Data::DamageMultiplier, false, 1.0f);

	// ==================== 基础伤害公式 ====================
	// RawDamage = SkillBaseDamage + AttackPower
	const float RawDamage = SkillBaseDamage + AttackerAttackPower;

	// MitigatedDamage = max(1, RawDamage - DefensePower)
	const float MitigatedDamage = FMath::Max(1.0f, RawDamage - TargetDefensePower);

	// FinalDamage = MitigatedDamage * SkillDamageMultiplier
	float FinalDamage = MitigatedDamage * SkillDamageMultiplier;

	// ==================== 命中判定（仅怪物/召唤物攻击玩家）====================
	// 玩家攻击必中；怪物/召唤物走命中/闪避判定。
	if (!bIsPlayerDamage)
	{
		const float BaseHitChance = 0.90f;
		const float HitScale = 200.0f + AttackerLevel * 10.0f;
		const float EvasionScale = 200.0f + DefenderLevel * 10.0f;
		const float FinalHitChance = FMath::Clamp(
			BaseHitChance + AttackerHitRating / HitScale - TargetEvasionRating / EvasionScale,
			0.05f, 0.98f);

		// 注意：LocalPredicted 下客户端预测与服务端权威的随机结果可能短暂不一致，由 GAS 预测回滚修正。
		if (FMath::FRand() > FinalHitChance)
		{
			FinalDamage = 0.0f;
		}
	}

	// ==================== 暴击判定 ====================
	bool bCriticalHit = false;
	if (FinalDamage > 0.0f)
	{
		const float CritScale = 200.0f + AttackerLevel * 10.0f;
		const float CritChance = AttackerCriticalRating / (AttackerCriticalRating + CritScale);

		// 同上，随机判定在 LocalPredicted 下存在预测回滚的可能。
		if (FMath::FRand() < CritChance)
		{
			FinalDamage *= AttackerCritDamageRate;
			bCriticalHit = true;
		}
	}

	// ==================== 写回自定义 EffectContext（供客户端 GameplayCue 读取）====================
	// 伤害计算的随机判定只在服务端一次性完成，结果写回 Context 后随 GE 自动网络复制到客户端，
	// 客户端无需重算，避免 LocalPredicted 预测回滚导致的不一致。
	// 说明：DragonOath 当前无格挡属性/判定（bBlocked 恒 false），格挡字段接口保留待 Phase 4 补齐；
	// 命中/部位/方向/元素/倍率等字段若后续在 ExecCalc 中算出，可在此一并写回。
	{
		FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
		checkf(EffectContextHandle.Get() &&
			EffectContextHandle.Get()->GetScriptStruct()->IsChildOf(FDOGameplayEffectContext::StaticStruct()),
			TEXT("EffectContext 不是 FDOGameplayEffectContext，请确认 UDOAbilitySystemGlobals 已注册自定义 Context"));
		if (FDOGameplayEffectContext* DOCtx = static_cast<FDOGameplayEffectContext*>(EffectContextHandle.Get()))
		{
			DOCtx->SetIsCriticalHit(bCriticalHit);
			// DOCtx->SetIsBlockedHit(bBlocked);        // 待 Phase 4 格挡属性补齐
			// DOCtx->SetHitBoneName(...);              // 待部位伤害系统
			// DOCtx->SetDamageDirection(...);
			// DOCtx->SetDamageElementTag(...);
			// DOCtx->SetDamageMultiplier(...);
		}
	}

	// ==================== 输出到 Damage Meta Attribute ====================
	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UDOHealthSet::GetDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage));
	}
}
