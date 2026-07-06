// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "UObject/WeakInterfacePtr.h"

#include "LoadingScreenManager.generated.h"

template <typename InterfaceType> class TScriptInterface;

class FSubsystemCollectionBase;
class IInputProcessor;
class ILoadingProcessInterface;
class SWidget;
class UObject;
class UWorld;
struct FFrame;
struct FWorldContext;

/**
 * 加载屏管理器（Loading Screen Manager）
 *
 * GameInstanceSubsystem，负责：
 * - 监听地图加载事件（PreLoadMap / PostLoadMap）
 * - 聚合所有 ILoadingProcessInterface 的加载需求
 * - 在需要时显示/隐藏加载屏 Widget
 * - 阻止游戏输入、调整性能设置
 *
 * 每帧 Tick 检查是否需要显示加载屏，加载完成后自动隐藏。
 */
UCLASS()
class COMMONLOADINGSCREEN_API ULoadingScreenManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	//~FTickableObjectBase interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End of FTickableObjectBase interface

	/** 获取调试信息：当前显示/隐藏加载屏的原因 */
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const
	{
		return DebugReasonForShowingOrHidingLoadingScreen;
	}

	/** 当前是否正在显示加载屏 */
	bool GetLoadingScreenDisplayStatus() const
	{
		return bCurrentlyShowingLoadingScreen;
	}

	/** 加载屏可见性变化委托 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingScreenVisibilityChangedDelegate, bool);
	FORCEINLINE FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChangedDelegate() { return LoadingScreenVisibilityChanged; }

	/** 注册外部加载处理器（实现 ILoadingProcessInterface 的对象） */
	void RegisterLoadingProcessor(TScriptInterface<ILoadingProcessInterface> Interface);
	/** 注销外部加载处理器 */
	void UnregisterLoadingProcessor(TScriptInterface<ILoadingProcessInterface> Interface);

private:
	/** 地图预加载回调 */
	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	/** 地图加载完成回调 */
	void HandlePostLoadMap(UWorld* World);

	/** 每帧更新加载屏显示状态 */
	void UpdateLoadingScreen();

	/** 检查是否有任何理由需要显示加载屏 */
	bool CheckForAnyNeedToShowLoadingScreen();

	/** 检查是否"想要"显示加载屏（包含强制显示等额外条件） */
	bool ShouldShowLoadingScreen();

	/** 是否正在显示启动初始加载屏（此阶段不使用运行时加载屏） */
	bool IsShowingInitialLoadingScreen() const;

	/** 显示加载屏——将 Widget 添加到 Viewport */
	void ShowLoadingScreen();

	/** 隐藏加载屏——销毁 Widget */
	void HideLoadingScreen();

	/** 从 Viewport 移除 Widget */
	void RemoveWidgetFromViewport();

	/** 加载屏显示期间阻止游戏输入 */
	void StartBlockingInput();

	/** 恢复游戏输入 */
	void StopBlockingInput();

	/** 切换性能设置（加载屏期间降低帧率等） */
	void ChangePerformanceSettings(bool bEnabingLoadingScreen);

private:
	/** 加载屏可见性变化委托 */
	FOnLoadingScreenVisibilityChangedDelegate LoadingScreenVisibilityChanged;

	/** 当前显示的加载屏 Widget */
	TSharedPtr<SWidget> LoadingScreenWidget;

	/** 输入预处理器——加载屏显示时吃掉所有输入 */
	TSharedPtr<IInputProcessor> InputPreProcessor;

	/** 外部加载处理器列表（弱引用，避免悬垂） */
	TArray<TWeakInterfacePtr<ILoadingProcessInterface>> ExternalLoadingProcessors;

	/** 调试用：当前显示/隐藏加载屏的原因 */
	FString DebugReasonForShowingOrHidingLoadingScreen;

	/** 加载屏开始显示的时间 */
	double TimeLoadingScreenShown = 0.0;

	/** 加载屏最近一次想要隐藏的时间（可能因最小显示时长而仍在显示） */
	double TimeLoadingScreenLastDismissed = -1.0;

	/** 下一次心跳日志的倒计时（秒） */
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	/** 是否处于 PreLoadMap → PostLoadMap 之间 */
	bool bCurrentlyInLoadMap = false;

	/** 当前是否正在显示加载屏 */
	bool bCurrentlyShowingLoadingScreen = false;
};
