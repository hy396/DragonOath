// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Engine/World.h"

#include "GameUIPolicy.generated.h"

class UCommonLocalPlayer;
class UGameUIManagerSubsystem;
class ULocalPlayer;
class UPrimaryGameLayout;

/** 本地多人交互模式 */
UENUM()
enum class ELocalMultiplayerInteractionMode : uint8
{
	PrimaryOnly,    // 仅主玩家全屏视口，忽略其他玩家
	SingleToggle,   // 单人全屏视口，玩家可切换控制权
	Simultaneous    // 多玩家视口同时显示
};

/** 根视口布局信息，关联本地玩家与其主布局 */
USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()
public:
	/** 关联的本地玩家 */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	/** 关联的根布局控件 */
	UPROPERTY(Transient)
	TObjectPtr<UPrimaryGameLayout> RootLayout = nullptr;

	/** 是否已添加到视口 */
	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo() {}
	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UPrimaryGameLayout* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		, RootLayout(InRootLayout)
		, bAddedToViewport(bIsInViewport)
	{}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};

/**
 * UI 策略基类，控制 UI 层级和布局规则，管理本地多人的视口分配和主布局生命周期。
 * 游戏可子类化以实现自定义的 UI 管理策略。
 * 必须挂在 UGameUIManagerSubsystem 下使用。
 */
UCLASS(Abstract, Blueprintable, Within = GameUIManagerSubsystem)
class COMMONGAME_API UGameUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	template <typename GameUIPolicyClass = UGameUIPolicy>
	static GameUIPolicyClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetGameUIPolicy(WorldContextObject));
	}

	static UGameUIPolicy* GetGameUIPolicy(const UObject* WorldContextObject);

public:
	virtual UWorld* GetWorld() const override;
	/** 获取所属的 UI 管理器 */
	UGameUIManagerSubsystem* GetOwningUIManager() const;
	/** 获取指定本地玩家的根布局 */
	UPrimaryGameLayout* GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const;

	/** 获取本地多人交互模式 */
	ELocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	/** 请求主控制权 */
	void RequestPrimaryControl(UPrimaryGameLayout* Layout);

protected:
	/** 将布局添加到视口 */
	void AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	/** 从视口移除布局 */
	void RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);

	/** 根布局添加到视口时的回调 */
	virtual void OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	/** 根布局从视口移除时的回调 */
	virtual void OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	/** 根布局释放时的回调 */
	virtual void OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);

	/** 为本地玩家创建布局控件 */
	void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer);
	/** 获取布局控件类 */
	TSubclassOf<UPrimaryGameLayout> GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer);

private:
	/** 本地多人交互模式 */
	ELocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = ELocalMultiplayerInteractionMode::PrimaryOnly;

	/** 布局控件的软类引用 */
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UPrimaryGameLayout> LayoutClass;

	/** 根视口布局信息数组 */
	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;

private:
	/** 通知玩家添加（由 UGameUIManagerSubsystem 调用） */
	void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	/** 通知玩家移除 */
	void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	/** 通知玩家销毁 */
	void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

	friend class UGameUIManagerSubsystem;
};
