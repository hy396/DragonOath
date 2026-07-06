// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#pragma once

#include "GameFramework/Pawn.h"

#include "ModularPawn.generated.h"

class UObject;

/**
 * 模块化 Pawn——支持 GameFeature 插件扩展
 *
 * PreInitializeComponents 注册为组件接收器，BeginPlay 发送 GameActorReady，
 * EndPlay 注销接收器，使得外部插件可以注入 PawnComponent。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPawn : public APawn
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

};
