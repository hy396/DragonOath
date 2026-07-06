// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h" // IWYU pragma: keep

#include "PrimaryGameLayout.generated.h"

class APlayerController;
class UClass;
class UCommonActivatableWidgetContainerBase;
class ULocalPlayer;
class UObject;
struct FFrame;

/**
 * 异步 Widget 层加载状态
 */
enum class EAsyncWidgetLayerState : uint8
{
	Canceled,		// 已取消
	Initialize,		// 初始化中
	AfterPush		// 已推入层栈
};

/**
 * 主游戏布局（Primary Game Layout）
 *
 * 游戏的主 UI 布局 Widget——管理所有 UI 层级的容器。
 * 每个玩家（包括分屏中的每个）都有一个独立的 PrimaryGameLayout 实例。
 * 负责将 ActivatableWidget 推入指定的层栈（Layer Stack）。
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class COMMONGAME_API UPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 获取主玩家（PrimaryPlayer）的主游戏布局 */
	static UPrimaryGameLayout* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	/** 通过 PlayerController 获取主游戏布局 */
	static UPrimaryGameLayout* GetPrimaryGameLayout(APlayerController* PlayerController);
	/** 通过 LocalPlayer 获取主游戏布局 */
	static UPrimaryGameLayout* GetPrimaryGameLayout(ULocalPlayer* LocalPlayer);

public:
	UPrimaryGameLayout(const FObjectInitializer& ObjectInitializer);

	/** 设置布局是否处于休眠状态，休眠时折叠且仅响应持久动作 */
	void SetIsDormant(bool Dormant);
	/** 是否处于休眠状态 */
	bool IsDormant() const { return bIsDormant; }

public:
	/** 异步推送控件到指定 UI 层（简单版，无状态回调） */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EAsyncWidgetLayerState, ActivatableWidgetT*) {});
	}

	/** 异步推送控件到指定 UI 层（带状态回调，支持初始化/完成/取消通知） */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass, TFunction<void(EAsyncWidgetLayerState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UCommonUIExtensions::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
			{
				UCommonUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

				ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit) {
					StateFunc(EAsyncWidgetLayerState::Initialize, &WidgetToInit);
				});

				StateFunc(EAsyncWidgetLayerState::AfterPush, Widget);
			})
		);

		// Setup a cancel delegate so that we can resume input if this handler is canceled.
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
			[this, StateFunc, SuspendInputToken]()
			{
				UCommonUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
				StateFunc(EAsyncWidgetLayerState::Canceled, nullptr);
			})
		);

		return StreamingHandle;
	}

	/** 同步推送控件到指定 UI 层（无初始化回调） */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
	}

	/** 同步推送控件到指定 UI 层（带初始化回调） */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}

		return nullptr;
	}

	// 在所有层中查找并移除指定控件
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// 获取指定层标签对应的层控件
	UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName);

protected:
	/** 注册一个 UI 层，控件可被推送到该层上 */
	UFUNCTION(BlueprintCallable, Category="Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	/** 休眠状态变更回调 */
	virtual void OnIsDormantChanged();

	/** 控件栈过渡动画回调 */
	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

private:
	/** 是否处于休眠状态 */
	bool bIsDormant = false;

	// 挂起输入令牌列表，用于跟踪多个异步 UI 加载期间的输入挂起
	TArray<FName> SuspendInputTokens;

	/** 已注册的 UI 层映射 */
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};
