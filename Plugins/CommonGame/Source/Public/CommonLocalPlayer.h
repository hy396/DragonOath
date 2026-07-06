// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Engine/LocalPlayer.h"

#include "CommonLocalPlayer.generated.h"

class APawn;
class APlayerController;
class APlayerState;
class FViewport;
class UObject;
class UPrimaryGameLayout;
struct FSceneViewProjectionData;

/** 通用 LocalPlayer 扩展，提供 PlayerController/PlayerState/Pawn 设置事件及玩家视图控制 */
UCLASS(config=Engine, transient)
class COMMONGAME_API UCommonLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UCommonLocalPlayer();

	/** 当本地玩家被分配 PlayerController 时触发 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	/** 当本地玩家被分配 PlayerState 时触发 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UCommonLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate OnPlayerStateSet;

	/** 当本地玩家被分配 Pawn 时触发 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UCommonLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	// 立即调用并注册 PlayerController 设置回调
	FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);
	// 立即调用并注册 PlayerState 设置回调
	FDelegateHandle CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate);
	// 立即调用并注册 PlayerPawn 设置回调
	FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);

public:
	virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	/** 是否启用玩家视图渲染 */
	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	/** 设置玩家视图是否启用 */
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

	/** 获取根 UI 布局 */
	UPrimaryGameLayout* GetRootUILayout() const;

private:
	/** 玩家视图是否启用 */
	bool bIsPlayerViewEnabled = true;
};
