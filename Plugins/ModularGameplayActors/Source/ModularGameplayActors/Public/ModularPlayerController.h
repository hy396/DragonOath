// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#pragma once

#include "GameFramework/PlayerController.h"

#include "ModularPlayerController.generated.h"

class UObject;

/**
 * 模块化 PlayerController——支持 GameFeature 插件扩展
 *
 * 在 PreInitializeComponents 注册为组件接收器，EndPlay 注销；
 * ReceivedPlayer 时发送 GameActorReady 事件并转发给所有 ControllerComponent；
 * PlayerTick 转发给所有 ControllerComponent。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

	//~ Begin APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~ End APlayerController interface
};
