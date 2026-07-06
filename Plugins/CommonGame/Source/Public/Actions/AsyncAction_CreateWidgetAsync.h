// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/SoftObjectPtr.h"

#include "AsyncAction_CreateWidgetAsync.generated.h"

class APlayerController;
class UGameInstance;
class UUserWidget;
class UWorld;
struct FFrame;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateWidgetAsyncDelegate, UUserWidget*, UserWidget);

/**
 * 异步创建 Widget——异步加载 Widget 类，加载完成后实例化并返回
 */
UCLASS(BlueprintType)
class COMMONGAME_API UAsyncAction_CreateWidgetAsync : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	// 取消异步操作
	virtual void Cancel() override;

	/** 异步创建 Widget 的工厂方法 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_CreateWidgetAsync* CreateWidgetAsync(UObject* WorldContextObject, TSoftClassPtr<UUserWidget> UserWidgetSoftClass, APlayerController* OwningPlayer, bool bSuspendInputUntilComplete = true);

	// 激活异步操作
	virtual void Activate() override;

public:

	/** Widget 创建完成回调 */
	UPROPERTY(BlueprintAssignable)
	FCreateWidgetAsyncDelegate OnComplete;

private:

	// Widget 加载完成回调
	void OnWidgetLoaded();

	FName SuspendInputToken;                              // 输入挂起令牌
	TWeakObjectPtr<APlayerController> OwningPlayer;       // 拥有者 PlayerController
	TWeakObjectPtr<UWorld> World;                         // 世界引用
	TWeakObjectPtr<UGameInstance> GameInstance;           // GameInstance 引用
	bool bSuspendInputUntilComplete;                      // 是否在加载完成前挂起输入
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;       // 要加载的 Widget 软类引用
	TSharedPtr<FStreamableHandle> StreamingHandle;        // 流式加载句柄
};
