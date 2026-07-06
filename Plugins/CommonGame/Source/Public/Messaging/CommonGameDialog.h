// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonMessagingSubsystem.h"

#include "CommonGameDialog.generated.h"

/** 确认对话框按钮动作定义 */
USTRUCT(BlueprintType)
struct FConfirmationDialogAction
{
	GENERATED_BODY()

public:
	/** 必填：对话框选项的结果类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECommonMessagingResult Result = ECommonMessagingResult::Unknown;

	/** 可选：覆盖动作名称的自定义显示文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FConfirmationDialogAction& Other) const
	{
		return Result == Other.Result &&
			OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};

/** 游戏对话框描述符，定义对话框的标题、内容和按钮动作 */
UCLASS()
class COMMONGAME_API UCommonGameDialogDescriptor : public UObject
{
	GENERATED_BODY()

public:
	/** 创建"确定"对话框描述符 */
	static UCommonGameDialogDescriptor* CreateConfirmationOk(const FText& Header, const FText& Body);
	/** 创建"确定/取消"对话框描述符 */
	static UCommonGameDialogDescriptor* CreateConfirmationOkCancel(const FText& Header, const FText& Body);
	/** 创建"是/否"对话框描述符 */
	static UCommonGameDialogDescriptor* CreateConfirmationYesNo(const FText& Header, const FText& Body);
	/** 创建"是/否/取消"对话框描述符 */
	static UCommonGameDialogDescriptor* CreateConfirmationYesNoCancel(const FText& Header, const FText& Body);

public:
	/** 对话框标题 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Header;

	/** 对话框正文 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Body;

	/** 对话框按钮动作列表 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
};


/** 游戏对话框基类，用于显示确认/错误等弹窗 */
UCLASS(Abstract)
class COMMONGAME_API UCommonGameDialog : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UCommonGameDialog();

	/** 设置对话框内容和结果回调 */
	virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback);

	/** 强制关闭对话框（无用户输入） */
	virtual void KillDialog();
};
