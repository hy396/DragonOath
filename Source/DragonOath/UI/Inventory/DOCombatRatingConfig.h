#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DOCombatRatingConfig.generated.h"

/** 用于战力和守护力展示的最终属性输入快照。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOCombatRatingInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float AttackPower = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float DefensePower = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float MaxHealth = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float MaxMana = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float CriticalRating = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float HitRating = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float EvasionRating = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float AttackSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float MoveSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float LifeStealRate = 0.0f;
};

/** 战力和守护力权重配置，避免把展示公式散落在 Widget 或 ViewModel 中。 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOCombatRatingConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power", meta = (DisplayName = "战力：攻击力权重"))
	float CombatAttackPowerWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power", meta = (DisplayName = "战力：暴击率权重"))
	float CombatCriticalRatingWeight = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power", meta = (DisplayName = "战力：命中率权重"))
	float CombatHitRatingWeight = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power", meta = (DisplayName = "战力：攻击速度权重"))
	float CombatAttackSpeedWeight = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power", meta = (DisplayName = "战力：吸血率权重"))
	float CombatLifeStealRateWeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power", meta = (DisplayName = "守护力：防御力权重"))
	float GuardDefensePowerWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power", meta = (DisplayName = "守护力：最大生命值权重"))
	float GuardMaxHealthWeight = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power", meta = (DisplayName = "守护力：最大法力值权重"))
	float GuardMaxManaWeight = 0.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power", meta = (DisplayName = "守护力：闪避率权重"))
	float GuardEvasionRatingWeight = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power", meta = (DisplayName = "守护力：移动速度权重"))
	float GuardMoveSpeedWeight = 0.10f;
};

/** 战力和守护力的纯计算库，只用于展示和推荐，不参与战斗结算。 */
UCLASS()
class DRAGONOATH_API UDOCombatRatingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DO|Combat Rating")
	static int32 CalculateCombatPower(const FDOCombatRatingInput& Input, const UDOCombatRatingConfig* Config = nullptr);

	UFUNCTION(BlueprintPure, Category = "DO|Combat Rating")
	static int32 CalculateGuardPower(const FDOCombatRatingInput& Input, const UDOCombatRatingConfig* Config = nullptr);
};
