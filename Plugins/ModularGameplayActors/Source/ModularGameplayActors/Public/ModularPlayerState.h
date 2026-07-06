// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#pragma once

#include "GameFramework/PlayerState.h"

#include "ModularPlayerState.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UObject;

/**
 * 模块化 PlayerState——支持 GameFeature 插件扩展
 *
 * PreInitializeComponents 注册接收器，BeginPlay 发送 GameActorReady，EndPlay 注销；
 * Reset 和 CopyProperties 转发给所有 PlayerStateComponent。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	//~ End AActor interface

protected:
	//~ Begin APlayerState interface
	virtual void CopyProperties(APlayerState* PlayerState);
	//~ End APlayerState interface
};
