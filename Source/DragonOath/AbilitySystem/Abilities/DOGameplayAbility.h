// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DOGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EDOAbilityActivationPolicy : uint8
{
	// 按下输入时触发一次，适合普攻、闪避、瞬发技能。
	OnInputTriggered,

	// 按键保持按下时尝试激活，适合格挡、蓄力、引导、持续瞄准。
	WhileInputActive,

	// 技能被授予或 Avatar 切换完成后自动尝试激活。
	OnSpawn,
};

/**
 * DragonOath 的技能基类。
 *
 * 这层基类只保留项目真正需要统一的规则，不重复封装 UGameplayAbility 已有的能力：
 * - 默认使用 InstancedPerActor + LocalPredicted，适合玩家技能。
 * - 通过 ActivationPolicy 统一输入激活策略，ASC 依赖它决定何时 TryActivate。
 * - CanActivateAbility 拦截 Level <= 0 的技能，配合职业技能配置的 0 级未学习机制。
 * - OnGiveAbility / OnAvatarSet 处理 OnSpawn 被动的自动激活（带 Level 检查）。
 *
 * 子类直接重写 GAS 原生的 ActivateAbility / EndAbility / InputPressed / InputReleased 即可，
 * 不需要额外的 DO* 间接层。
 */
UCLASS(Abstract, Blueprintable)
class DRAGONOATH_API UDOGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDOGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//~End of UGameplayAbility interface

	UFUNCTION(BlueprintPure, Category="DO|Ability")
	EDOAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DO|Ability")
	EDOAbilityActivationPolicy ActivationPolicy = EDOAbilityActivationPolicy::OnInputTriggered;
};
