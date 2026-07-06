// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"

#include "GameUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonLocalPlayer;
class UGameUIPolicy;
class UObject;

/**
 * 游戏 UI 管理子系统，管理 UI 策略和主布局。
 * 此管理器设计为可被子类替换，标记为 Abstract 防止直接实例化。
 * 若只需基本功能，可在自己的游戏中子类化此子系统。
 */
UCLASS(Abstract, config = Game)
class COMMONGAME_API UGameUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGameUIManagerSubsystem() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 获取当前 UI 策略（只读） */
	const UGameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	/** 获取当前 UI 策略（可写） */
	UGameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	/** 通知：LocalPlayer 被添加 */
	virtual void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	/** 通知：LocalPlayer 被移除 */
	virtual void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	/** 通知：LocalPlayer 被销毁 */
	virtual void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

protected:
	/** 切换到新的 UI 策略 */
	void SwitchToPolicy(UGameUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UGameUIPolicy> CurrentPolicy = nullptr;		// 当前生效的 UI 策略

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;		// 默认 UI 策略类的软引用
};
