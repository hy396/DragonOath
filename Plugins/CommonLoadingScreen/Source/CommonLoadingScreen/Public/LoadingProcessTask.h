// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#pragma once

#include "LoadingProcessInterface.h"
#include "UObject/Object.h"

#include "LoadingProcessTask.generated.h"

struct FFrame;

/**
 * 加载过程任务（Loading Process Task）
 *
 * 一个 UObject + ILoadingProcessInterface 的便捷实现，
 * 可在蓝图中创建并注册到 LoadingScreenManager，
 * 用于手动控制加载屏的显示/隐藏。
 */
UCLASS(BlueprintType)
class COMMONLOADINGSCREEN_API ULoadingProcessTask : public UObject, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	/** 创建一个加载过程任务并注册到 LoadingScreenManager */
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject"))
	static ULoadingProcessTask* CreateLoadingScreenProcessTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

public:
	ULoadingProcessTask() { }

	/** 注销此任务（不再阻止加载屏隐藏） */
	UFUNCTION(BlueprintCallable)
	void Unregister();

	/** 更新显示加载屏的原因 */
	UFUNCTION(BlueprintCallable)
	void SetShowLoadingScreenReason(const FString& InReason);

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;

	FString Reason;		// 加载屏原因描述
};
