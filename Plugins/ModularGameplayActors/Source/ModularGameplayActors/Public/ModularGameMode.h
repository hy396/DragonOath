// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#pragma once

#include "GameFramework/GameMode.h"

#include "ModularGameMode.generated.h"

class UObject;

/**
 * 模块化 GameMode 基类（Base 版本，配合 AModularGameStateBase 使用）
 *
 * 构造时自动将 GameState / PlayerController / PlayerState / DefaultPawn
 * 设置为对应的 Modular 版本，使得这些 Actor 也支持组件注入。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AModularGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

/**
 * 模块化 GameMode（完整版，配合 AModularGameState 使用）
 *
 * 继承自 AGameMode（非 Base），支持匹配流程等完整 GameMode 功能。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AModularGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
