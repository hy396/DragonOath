// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：用户、平台和在线会话辅助插件，后续登录、房间、匹配阶段再深入接入。
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonUserBasicPresence.generated.h"

class UCommonSessionSubsystem;
enum class ECommonSessionInformationState : uint8;

//////////////////////////////////////////////////////////////////////
// UCommonUserBasicPresence

/**
 * 基础在线状态（Rich Presence）子系统
 *
 * 监听会话子系统的状态变化，将其推送到在线 Presence 接口。
 * 这不是一个完整的 Rich Presence 实现，仅作为概念验证，
 * 演示如何从会话子系统向 Presence 系统推送信息。
 */
UCLASS(BlueprintType, Config = Engine)
class COMMONUSER_API UCommonUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonUserBasicPresence();


	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	/** 总开关——设为 false 可阻止推送 Presence */
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** "游戏中" 状态对应的后端 Key */
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** "主菜单" 状态对应的后端 Key */
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** "匹配中" 状态对应的后端 Key */
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** "游戏模式" Rich Presence 条目对应的后端 Key */
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** "地图名称" Rich Presence 条目对应的后端 Key */
	UPROPERTY(Config)
	FString PresenceKeyMapName;

	void OnNotifySessionInformationChanged(ECommonSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	FString SessionStateToBackendKey(ECommonSessionInformationState SessionStatus);
};
