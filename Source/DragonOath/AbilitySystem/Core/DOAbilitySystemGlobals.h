// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemGlobals.h"

#include "DOAbilitySystemGlobals.generated.h"

class UObject;
struct FGameplayEffectContext;

/**
 * DragonOath 的 GAS 全局配置入口。
 *
 * UE 会通过 UAbilitySystemGlobals 创建 GameplayEffectContext。
 * 重写这里后，所有 GE 都会使用 FDOGameplayEffectContext，从而携带暴击、格挡、部位等项目战斗数据。
 */
UCLASS(Config=Game)
class UDOAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_UCLASS_BODY()

	//~UAbilitySystemGlobals interface
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	//~End of UAbilitySystemGlobals interface
};
