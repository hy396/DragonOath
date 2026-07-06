// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"

#include "CommonUIExtensions.generated.h"

enum class ECommonInputType : uint8;
template <typename T> class TSubclassOf;

class APlayerController;
class UCommonActivatableWidget;
class ULocalPlayer;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayTag;

/** CommonUI 扩展工具类，提供 UI 层级推送、输入挂起/恢复等蓝图可调用工具函数 */
UCLASS()
class COMMONGAME_API UCommonUIExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UCommonUIExtensions() { }

	/** 获取所属玩家的输入类型 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Extensions", meta = (WorldContext = "WidgetContextObject"))
	static ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	/** 判断所属玩家是否正在使用触控 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Extensions", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	/** 判断所属玩家是否正在使用手柄 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Extensions", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	/** 将控件内容推送到指定 UI 层 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UCommonActivatableWidget* PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/** 将流式加载的控件内容推送到指定 UI 层 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static void PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	/** 从 UI 层中弹出指定控件 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static void PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** 从 PlayerController 获取 LocalPlayer */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	/** 挂起玩家输入，返回挂起令牌 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);

	/** 挂起玩家输入（LocalPlayer 版本），返回挂起令牌 */
	static FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	/** 恢复玩家输入 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);

	/** 恢复玩家输入（LocalPlayer 版本） */
	static void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	/** 输入挂起计数 */
	static int32 InputSuspensions;
};
