// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"

#include "CommonMessagingSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonGameDialogDescriptor;
class UObject;

/** 对话框可能的结果 */
UENUM(BlueprintType)
enum class ECommonMessagingResult : uint8
{
	Confirmed,   // 按下"是"
	Declined,    // 按下"否"
	Cancelled,   // 按下"忽略/取消"
	Killed,      // 对话框被强制关闭（无用户输入）
	Unknown UMETA(Hidden)
};

/** 对话框结果回调委托 */
DECLARE_DELEGATE_OneParam(FCommonMessagingResultDelegate, ECommonMessagingResult /* Result */);

/**
 * 消息子系统，管理确认/错误对话框的显示。
 * 挂载在 LocalPlayer 下，提供 ShowConfirmation 和 ShowError 接口。
 */
UCLASS(config = Game)
class COMMONGAME_API UCommonMessagingSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UCommonMessagingSubsystem() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 显示确认对话框 */
	virtual void ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate());
	/** 显示错误对话框 */
	virtual void ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate());

private:

};
