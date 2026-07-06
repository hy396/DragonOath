// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UObject/SoftObjectPath.h"

#include "CommonLoadingScreenSettings.generated.h"

class UObject;

/**
 * 加载屏系统设置（Developer Settings）
 *
 * 可在项目设置中配置加载屏 Widget、Z-Order、额外保持时间等。
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Common Loading Screen"))
class UCommonLoadingScreenSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UCommonLoadingScreenSettings();

public:

	// 加载屏使用的 UMG Widget 类
	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	// 加载屏 Widget 在 Viewport 栈中的 Z-Order
	UPROPERTY(config, EditAnywhere, Category=Display)
	int32 LoadingScreenZOrder = 10000;

	// 加载完成后额外保持加载屏的秒数——给贴图流式加载留时间，避免模糊
	// 注意：编辑器中默认不生效，可通过 HoldLoadingScreenAdditionalSecsEvenInEditor 启用
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s, ConsoleVariable="CommonLoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	// 加载屏心跳超时阈值（秒）——超过此时间视为卡死（0 = 不检测）
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	// 心跳日志间隔（秒）——定期输出加载屏仍在显示的原因
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	// 每帧打印加载屏显示/隐藏原因（调试用）
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="CommonLoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = 0;

	// 强制显示加载屏（调试用）
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="CommonLoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	// 编辑器中也应用额外保持时间（便于迭代加载屏）
	UPROPERTY(Transient, EditAnywhere, Category=Debugging)
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

	// 编辑器中也强制 Tick 加载屏
	UPROPERTY(config, EditAnywhere, Category=Configuration)
	bool ForceTickLoadingScreenEvenInEditor = true;
};
