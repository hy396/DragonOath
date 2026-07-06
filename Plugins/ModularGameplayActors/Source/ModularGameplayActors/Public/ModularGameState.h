// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#pragma once

#include "GameFramework/GameState.h"

#include "ModularGameState.generated.h"

class UObject;

/**
 * 模块化 GameState 基类（Base 版本，配合 AModularGameModeBase 使用）
 *
 * 在 PreInitializeComponents 注册为组件接收器，
 * BeginPlay 发送 GameActorReady 事件，EndPlay 注销接收器，
 * 使得 GameFeature 插件可以通过 UGameFrameworkComponentManager 向其注入组件。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface
};


/**
 * 模块化 GameState（完整版，配合 AModularGameMode 使用）
 *
 * 额外重写 HandleMatchHasStarted，将匹配开始事件转发给所有 GameStateComponent。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameState : public AGameState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

protected:
	//~ Begin AGameState interface
	virtual void HandleMatchHasStarted() override;
	//~ Begin AGameState interface
};
