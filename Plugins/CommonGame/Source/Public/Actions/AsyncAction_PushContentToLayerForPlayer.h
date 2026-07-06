// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

#include "AsyncAction_PushContentToLayerForPlayer.generated.h"

class APlayerController;
class UCommonActivatableWidget;
class UObject;
struct FFrame;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushContentToLayerForPlayerAsyncDelegate, UCommonActivatableWidget*, UserWidget);

/**
 * 异步推送内容到 UI 层——异步加载 Widget 并推入指定玩家的 UI 层栈
 */
UCLASS(BlueprintType)
class COMMONGAME_API UAsyncAction_PushContentToLayerForPlayer : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	// 取消异步操作
	virtual void Cancel() override;

	/** 异步推送内容到层的工厂方法 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_PushContentToLayerForPlayer* PushContentToLayerForPlayer(APlayerController* OwningPlayer, UPARAM(meta = (AllowAbstract=false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, bool bSuspendInputUntilComplete = true);

	// 激活异步操作
	virtual void Activate() override;

public:

	/** 推入前回调 */
	UPROPERTY(BlueprintAssignable)
	FPushContentToLayerForPlayerAsyncDelegate BeforePush;

	/** 推入后回调 */
	UPROPERTY(BlueprintAssignable)
	FPushContentToLayerForPlayerAsyncDelegate AfterPush;

private:

	FGameplayTag LayerName;                                    // 目标 UI 层标签
	bool bSuspendInputUntilComplete = false;                   // 是否在加载完成前挂起输入
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;         // 拥有者 PlayerController
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;       // 要加载的 Widget 软类引用

	TSharedPtr<FStreamableHandle> StreamingHandle;             // 流式加载句柄
};
