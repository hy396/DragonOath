#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"

#include "DOPlayerState.generated.h"

class UDOAbilitySystemComponent;
class UDOHealthSet;
class UDOPlaySet;

/**
 * 玩家状态。
 *
 * PlayerState 生命周期比 Pawn 更稳定，适合拥有 ASC 和持久属性集。
 * 玩家死亡、换 Pawn、重生时，能力和属性不会因为 Pawn 被销毁而丢失。
 */
UCLASS()
class DRAGONOATH_API ADOPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADOPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "DO|PlayerState")
	UDOAbilitySystemComponent* GetDOAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "DO|PlayerState")
	UDOHealthSet* GetHealthSet() const;

	UFUNCTION(BlueprintPure, Category = "DO|PlayerState")
	UDOPlaySet* GetPlaySet() const;

private:
	// 玩家真正的 ASC。Mixed 复制模式下，Owner 能拿到完整 GE 信息，其它客户端只需要标签和 Cue。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DO|Ability", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;

	// 生命相关属性集：Health、MaxHealth、Damage 等。
	UPROPERTY()
	TObjectPtr<UDOHealthSet> HealthSet;

	// 战斗资源和基础数值属性集：Mana、Stamina、AttackPower、DefensePower 等。
	UPROPERTY()
	TObjectPtr<UDOPlaySet> PlaySet;
};
