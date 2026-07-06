// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "ModularPlayerController.h"

#include "CommonPlayerController.generated.h"

class APawn;
class UObject;

/** 通用 PlayerController 基类，支持 ModularGameplay 组件注入 */
UCLASS(config=Game)
class COMMONGAME_API ACommonPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	ACommonPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 玩家连接后接收玩家回调
	virtual void ReceivedPlayer() override;
	// 设置控制的 Pawn
	virtual void SetPawn(APawn* InPawn) override;
	// 附身 Pawn
	virtual void OnPossess(class APawn* APawn) override;
	// 解除附身 Pawn
	virtual void OnUnPossess() override;

protected:
	// PlayerState 属性同步回调
	virtual void OnRep_PlayerState() override;
};
