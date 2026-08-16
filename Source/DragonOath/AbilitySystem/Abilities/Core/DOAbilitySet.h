// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "Engine/DataAsset.h"
#include "UObject/Object.h"

#include "DOAbilitySet.generated.h"

class UDOGameplayAbility;
class UDOAbilitySystemComponent;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOGameplayEffectGrant
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "GameplayEffect 类"))
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.01", DisplayName = "效果等级"))
	float Level = 1.0f;
};

/** 一个来源获得的所有 GAS 句柄，装备卸下时只撤销这一组。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOAbilitySetGrantedHandles
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer GrantedTags;

	void Reset()
	{
		AbilitySpecHandles.Reset();
		GameplayEffectHandles.Reset();
		GrantedTags.Reset();
	}
};

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
	None UMETA(DisplayName = "不自动绑定"),

	// 玩家按键触发，需要填写 InputTag
	Input UMETA(DisplayName = "输入触发"),

	// 事件触发，需要填写 EventTag，同时 Ability 蓝图的 AbilityTriggers 也要配相同的 EventTag
	GameplayEvent UMETA(DisplayName = "Gameplay 事件触发"),

	// 授予后自动激活，映射到 Ability 蓝图的 ActivationPolicy = OnSpawn
	OnGranted UMETA(DisplayName = "授予后自动激活"),
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
	// 仅允许从 Ability.Id.* 命名空间选择，避免误填 InputTag / Status 等其它语义标签。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Ability.Id", DisplayName = "技能标识"))
	FGameplayTag AbilityId;

	// 技能蓝图类，必须继承 UDOGameplayAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (DisplayName = "技能类"))
	TSubclassOf<UDOGameplayAbility> AbilityClass;

	// 初始等级。0 = 尚未学习，1+ = 已学习可激活
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (DisplayName = "初始等级"))
	int32 InitialLevel = 0;

	// 最大等级
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (DisplayName = "最大等级"))
	int32 MaxLevel = 1;

	// 授予时的绑定方式
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (DisplayName = "授予触发方式"))
	EDOAbilityGrantTriggerType TriggerType = EDOAbilityGrantTriggerType::None;

	// 输入标签，TriggerType = Input 时必须填写。仅允许 InputTag.* 命名空间（与 Setly 输入配置一致）。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag", DisplayName = "输入标签"))
	FGameplayTag InputTag;

	// 事件标签，TriggerType = GameplayEvent 时必须填写，同时 Ability 蓝图的 AbilityTriggers 也要配相同标签。仅允许 Event.* 命名空间。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Event", DisplayName = "事件标签"))
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "AbilityId", DisplayName = "授予技能"))
	TArray<FDOAbilityGrant> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "GameplayEffectClass", DisplayName = "授予 GameplayEffect"))
	TArray<FDOGameplayEffectGrant> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Gameplay", DisplayName = "授予标签"))
	FGameplayTagContainer GrantedTags;

	// 编辑期数据校验：检查每个授予项的必填 Tag 与 TriggerType 是否匹配，避免漏填导致技能不可用。
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
};
