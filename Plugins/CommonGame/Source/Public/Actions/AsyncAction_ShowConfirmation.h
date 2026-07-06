// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "UObject/ObjectPtr.h"
#include "AsyncAction_ShowConfirmation.generated.h"

enum class ECommonMessagingResult : uint8;

class FText;
class UCommonGameDialogDescriptor;
class ULocalPlayer;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonMessagingResultMCDelegate, ECommonMessagingResult, Result);

/**
 * 异步确认对话框——蓝图可调用的异步节点，等待用户选择后返回结果
 */
UCLASS()
class UAsyncAction_ShowConfirmation : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	/** 显示 是/否 确认对话框 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowConfirmation* ShowConfirmationYesNo(
		UObject* InWorldContextObject, FText Title, FText Message
	);

	/** 显示 确定/取消 确认对话框 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowConfirmation* ShowConfirmationOkCancel(
		UObject* InWorldContextObject, FText Title, FText Message
	);

	/** 显示自定义确认对话框 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowConfirmation* ShowConfirmationCustom(
		UObject* InWorldContextObject, UCommonGameDialogDescriptor* Descriptor
	);

	// 激活异步操作
	virtual void Activate() override;

public:
	/** 用户选择结果回调 */
	UPROPERTY(BlueprintAssignable)
	FCommonMessagingResultMCDelegate OnResult;

private:
	/** 处理确认结果 */
	void HandleConfirmationResult(ECommonMessagingResult ConfirmationResult);

	/** 世界上下文对象 */
	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	/** 目标本地玩家 */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	/** 对话框描述符 */
	UPROPERTY(Transient)
	TObjectPtr<UCommonGameDialogDescriptor> Descriptor;
};
