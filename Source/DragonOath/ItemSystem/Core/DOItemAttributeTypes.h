#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "DOItemAttributeTypes.generated.h"

/** 消耗品的直接效果类型。普通效果由 C++ 原生 GE 处理，复杂流程再交给 Ability/Event。 */
UENUM(BlueprintType)
enum class EDOConsumableEffectKind : uint8
{
	None UMETA(DisplayName = "未配置"),
	InstantRestore UMETA(DisplayName = "即时回复"),
	TimedAttributeModifier UMETA(DisplayName = "限时属性"),
	GameplayAbility UMETA(DisplayName = "Gameplay Ability"),
	GameplayEvent UMETA(DisplayName = "Gameplay Event")
};

/** 装备和限时 Buff 共用的类型化属性数值。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOAttributeModifierValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "攻击力"))
	float AttackPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "防御力"))
	float DefensePower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "最大生命值"))
	float MaxHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "最大法力值"))
	float MaxMana = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "暴击率"))
	float CriticalRating = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "命中率"))
	float HitRating = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "闪避率"))
	float EvasionRating = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "攻击速度"))
	float AttackSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "移动速度"))
	float MoveSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "属性", meta = (ClampMin = "0.0", DisplayName = "吸血率"))
	float LifeStealRate = 0.0f;

	/** 判断所有属性是否都接近零。 */
	bool IsNearlyZero() const
	{
		return FMath::IsNearlyZero(AttackPower)
			&& FMath::IsNearlyZero(DefensePower)
			&& FMath::IsNearlyZero(MaxHealth)
			&& FMath::IsNearlyZero(MaxMana)
			&& FMath::IsNearlyZero(CriticalRating)
			&& FMath::IsNearlyZero(HitRating)
			&& FMath::IsNearlyZero(EvasionRating)
			&& FMath::IsNearlyZero(AttackSpeed)
			&& FMath::IsNearlyZero(MoveSpeed)
			&& FMath::IsNearlyZero(LifeStealRate);
	}
};

/** 即时回复使用的资源数值，不直接暴露 GAS 或 ASC。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOResourceRestoreValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "回复", meta = (ClampMin = "0.0", DisplayName = "生命恢复量"))
	float Healing = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "回复", meta = (ClampMin = "0.0", DisplayName = "法力恢复量"))
	float ManaRestore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "回复", meta = (ClampMin = "0.0", DisplayName = "体力恢复量"))
	float StaminaRestore = 0.0f;

	/** 判断所有回复数值是否都接近零。 */
	bool IsNearlyZero() const
	{
		return FMath::IsNearlyZero(Healing)
			&& FMath::IsNearlyZero(ManaRestore)
			&& FMath::IsNearlyZero(StaminaRestore);
	}
};

/** 限时属性道具的数值、持续时间和激活期间授予的状态标签。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemTimedModifierValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "限时效果", meta = (ClampMin = "0.0", DisplayName = "持续时间（秒）"))
	float DurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "限时效果", meta = (ShowOnlyInnerProperties, DisplayName = "属性修正"))
	FDOAttributeModifierValues Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "限时效果", meta = (DisplayName = "授予标签"))
	FGameplayTagContainer GrantedTags;
};

/** 道具公共冷却的配置。Tag 和时长必须同时有效。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOItemCooldownConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "冷却", meta = (Categories = "Cooldown", DisplayName = "冷却标签"))
	FGameplayTag CooldownTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "冷却", meta = (ClampMin = "0.0", DisplayName = "冷却时间（秒）"))
	float DurationSeconds = 0.0f;

	bool IsEnabled() const
	{
		return CooldownTag.IsValid() && FMath::IsFinite(DurationSeconds) && DurationSeconds > 0.0f;
	}
};
