// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "CommonUserWidget.h"
#include "Fonts/SlateFontInfo.h"

#include "CommonPlayerInputKey.generated.h"

enum class ECommonInputType : uint8;

class APlayerController;
class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UCommonLocalPlayer;
class UMaterialInstanceDynamic;
class UObject;
struct FFrame;
struct FGeometry;

/** 按键长按强制状态 */
UENUM(BlueprintType)
enum class ECommonKeybindForcedHoldStatus : uint8
{
	NoForcedHold,    // 不强制
	ForcedHold,      // 强制显示为长按
	NeverShowHold    // 从不显示长按
};

/** 带缓存的测量文本结构体 */
USTRUCT()
struct FMeasuredText
{
	GENERATED_BODY()

public:
	FText GetText() const { return CachedText; }       // 获取缓存的文本
	void SetText(const FText& InText);                 // 设置文本

	FVector2D GetTextSize() const { return CachedTextSize; }           // 获取缓存的文本尺寸
	FVector2D UpdateTextSize(const FSlateFontInfo &InFontInfo, float FontScale = 1.0f) const;  // 更新文本尺寸

private:

	FText CachedText;                           // 缓存的文本内容
	mutable FVector2D CachedTextSize;           // 缓存的文本尺寸
	mutable bool bTextDirty = true;             // 文本是否需要重新测量
};

/** 按键输入显示控件，用于在 UI 上展示当前绑定的按键/动作，并支持长按进度显示 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class COMMONGAME_API UCommonPlayerInputKey : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonPlayerInputKey(const FObjectInitializer& ObjectInitializer);

	/** 根据当前绑定动作更新按键显示 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void UpdateKeybindWidget();

	/** 设置绑定的按键 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetBoundKey(FKey NewBoundAction);

	/** 设置绑定的动作名称 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetBoundAction(FName NewBoundAction);

	/** 强制设置长按状态（已弃用，请使用 SetForcedHoldKeybindStatus） */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget", meta=(DeprecatedFunction, DeprecationMessage = "Use SetForcedHoldKeybindStatus instead"))
	void SetForcedHoldKeybind(bool InForcedHoldKeybind);

	/** 设置长按强制状态 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetForcedHoldKeybindStatus(ECommonKeybindForcedHoldStatus InForcedHoldKeybindStatus);

	/** 设置是否显示长按倒计时 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetShowProgressCountDown(bool bShow);

	/** 设置轴缩放值 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetAxisScale(const float NewValue) { AxisScale = NewValue; }

	/** 设置预设名称覆盖值 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetPresetNameOverride(const FName NewValue) { PresetNameOverride = NewValue; }

	/** 当前绑定的动作名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FName BoundAction;

	/** 轴映射的缩放值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	float AxisScale;

	/** 直接绑定的按键（用于引用特定按键而非动作时的回退） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKeyFallback;

	/** 输入类型覆盖（显式指定按键类型） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	ECommonInputType InputTypeOverride;

	/** 预设名称覆盖（显式指定预设名称） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	FName PresetNameOverride;

	/** 长按强制状态：可显示为长按或从不显示长按 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	ECommonKeybindForcedHoldStatus ForcedHoldKeybindStatus;

	/** 长按进度开始时通过委托调用 */
	UFUNCTION()
	void StartHoldProgress(FName HoldActionName, float HoldDuration);

	/** 长按进度停止时通过委托调用 */
	UFUNCTION()
	void StopHoldProgress(FName HoldActionName, bool bCompletedSuccessfully);

	/** 获取当前按键绑定是否为长按动作 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	bool IsHoldKeybind() const { return bIsHoldKeybind; }

	/** 绑定按键是否有效 */
	UFUNCTION()
	bool IsBoundKeyValid() const { return BoundKey.IsValid(); }

protected:
	virtual void NativePreConstruct() override;      // 预构造
	virtual void NativeConstruct() override;          // 构造
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	void RecalculateDesiredSize();                    // 重新计算期望尺寸

	/** 销毁时清理 MID */
	virtual void NativeDestruct() override;

	/** 是否为长按绑定 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Keybind Widget", meta=(ScriptName = "IsHoldKeybindValue"))
	bool bIsHoldKeybind;

	/** 是否显示按键边框 */
	UPROPERTY(Transient)
	bool bShowKeybindBorder;

	/** 帧尺寸 */
	UPROPERTY(Transient)
	FVector2D FrameSize;

	/** 是否显示倒计时 */
	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	bool bShowTimeCountDown;

	/** 推导出的绑定按键 */
	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKey;

	/** 长按进度画刷 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush HoldProgressBrush;

	/** 按键文本边框画刷 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush KeyBindTextBorder;

	/** 是否显示未绑定状态 */
	UPROPERTY(EditAnywhere, Category = "Keybind Widget")
	bool bShowUnboundStatus = false;

	/** 按键文本字体 */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo KeyBindTextFont;

	/** 倒计时文本字体 */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo CountdownTextFont;

	/** 倒计时文本 */
	UPROPERTY(Transient)
	FMeasuredText CountdownText;

	/** 按键绑定文本 */
	UPROPERTY(Transient)
	FMeasuredText KeybindText;

	/** 按键文本内边距 */
	UPROPERTY(Transient)
	FMargin KeybindTextPadding;

	/** 按键帧最小尺寸 */
	UPROPERTY(Transient)
	FVector2D KeybindFrameMinimumSize;

	/** 长按百分比材质参数名 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FName PercentageMaterialParameterName;

	/** 长按进度的动态材质实例 */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressPercentageMID;

	virtual void NativeOnInitialized() override;

private:
	/** 同步长按进度到当前 PlayerController 的状态 */
	void SyncHoldProgress();

	/** 长按期间更新进度显示 */
	void UpdateHoldProgress();

	/** 设置长按绑定的初始化 */
	void SetupHoldKeybind();

	/** 显示长按背板 */
	void ShowHoldBackPlate();

	/** PlayerController 设置回调 */
	void HandlePlayerControllerSet(UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController);

	/** 长按开始时间 */
	float HoldKeybindStartTime = 0;

	/** 长按持续时间（秒） */
	float HoldKeybindDuration = 0;

	bool bDrawProgress = false;                // 是否绘制进度
	bool bDrawBrushForKey = false;             // 是否绘制按键画刷
	bool bDrawCountdownText = false;           // 是否绘制倒计时文本
	bool bWaitingForPlayerController = false;  // 是否等待 PlayerController

	/** 缓存的按键画刷 */
	UPROPERTY(Transient)
	FSlateBrush CachedKeyBrush;
};
