// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#pragma once

#include "AIController.h"

#include "ModularAIController.generated.h"

class UObject;

/**
 * 模块化 AIController——支持 GameFeature 插件扩展
 *
 * PreInitializeComponents 注册为组件接收器，BeginPlay 发送 GameActorReady，
 * EndPlay 注销接收器，使得外部插件可以注入 AIControllerComponent。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularAIController : public AAIController
{
	GENERATED_BODY()

public:
	//~ Begin AActor Interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
