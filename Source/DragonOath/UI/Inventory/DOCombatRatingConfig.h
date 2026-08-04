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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power")
	float CombatAttackPowerWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power")
	float CombatCriticalRatingWeight = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power")
	float CombatHitRatingWeight = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power")
	float CombatAttackSpeedWeight = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Power")
	float CombatLifeStealRateWeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power")
	float GuardDefensePowerWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power")
	float GuardMaxHealthWeight = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power")
	float GuardMaxManaWeight = 0.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power")
	float GuardEvasionRatingWeight = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard Power")
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
