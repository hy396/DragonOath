// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "DOAbilitySet.generated.h"

class UDOGameplayAbility;

/**
 * 技能授予时的绑定方式。
 *
 * 与 EDOAbilityActivationPolicy（配置在 Ability 蓝图中）职责不同：
 * - TriggerType 负责"授予时怎么绑定"（绑输入、绑事件、自动激活、不绑定）
 * - ActivationPolicy 负责"激活后输入行为策略"（点按、持续、授予即激活）
 */
UENUM(BlueprintType)
enum class EDOAbilityGrantTriggerType : uint8
{
	// 不自动绑定，用于纯 UI 展示或外部手动激活
	None,

	// 玩家按键触发，需要填写 InputTag
	Input,

	// 事件触发，需要填写 EventTag，同时 Ability 蓝图的 AbilityTriggers 也要配相同的 EventTag
	GameplayEvent,

	// 授予后自动激活，映射到 Ability 蓝图的 ActivationPolicy = OnSpawn
	OnGranted,
};

/**
 * 单个技能授予项：表示"这个职业拥有这个技能，以及它的初始状态"。
 *
 * 只负责静态配置（职业定义），技能树前置条件/消耗由 SkillTreeComponent 管理。
 */
USTRUCT(BlueprintType)
struct FDOAbilityGrant
{
	GENERATED_BODY()

	// 技能唯一标识，用于 UI、存档、升级查找。不建议用技能类名当唯一标识。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityId;

	// 技能蓝图类，必须继承 UDOGameplayAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDOGameplayAbility> AbilityClass;

	// 初始等级。0 = 尚未学习，1+ = 已学习可激活
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InitialLevel = 0;

	// 最大等级
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxLevel = 1;

	// 授予时的绑定方式
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EDOAbilityGrantTriggerType TriggerType = EDOAbilityGrantTriggerType::None;

	// 输入标签，TriggerType = Input 时必须填写
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag;

	// 事件标签，TriggerType = GameplayEvent 时必须填写，同时 Ability 蓝图的 AbilityTriggers 也要配相同标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag EventTag;
};

/**
 * 职业技能组。
 *
 * 每个职业对应一个 AbilitySet，包含该职业的所有技能授予项。
 * 初始化时一次性授予全部技能到 ASC，0 级技能保留但不可激活。
 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "AbilityId"))
	TArray<FDOAbilityGrant> GrantedAbilities;
};