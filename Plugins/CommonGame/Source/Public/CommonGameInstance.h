// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#pragma once

#include "Engine/GameInstance.h"

#include "CommonGameInstance.generated.h"

enum class ECommonUserAvailability : uint8;
enum class ECommonUserPrivilege : uint8;

class FText;
class UCommonUserInfo;
class UCommonSession_SearchResult;
struct FOnlineResultInformation;
class ULocalPlayer;
class USocialManager;
class UObject;
struct FFrame;
struct FGameplayTag;

/** 通用 GameInstance 基类，处理用户登录/错误/会话等 */
UCLASS(Abstract, Config = Game)
class COMMONGAME_API UCommonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCommonGameInstance(const FObjectInitializer& ObjectInitializer);

	/** 处理来自 CommonUser 的错误/警告，可按游戏重写 */
	UFUNCTION()
	virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);

	/** 处理用户权限变更回调 */
	UFUNCTION()
	virtual void HandlePrivilegeChanged(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability);

	/** 处理用户初始化完成回调 */
	UFUNCTION()
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	/** 重置用户和会话状态，通常在玩家断开连接时调用 */
	virtual void ResetUserAndSessionState();

	/**
	 * 请求加入会话的流程：
	 *   外部请求用户加入特定会话（例如平台覆盖层通过 OnUserRequestedSession），
	 *   该请求在 SetRequestedSession 中处理。
	 *   检查是否可以立即加入请求的会话（CanJoinRequestedSession），若可以则直接加入（JoinRequestedSession），
	 *   若不可以则缓存请求的会话，并指示游戏进入可加入会话的状态（ResetGameAndJoinRequestedSession）。
	 */
	/** 处理用户从外部来源（如平台覆盖层）接受会话邀请，可按游戏重写 */
	virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/** 获取请求加入的会话 */
	UCommonSession_SearchResult* GetRequestedSession() const { return RequestedSession; }
	/** 设置（或清除）请求加入的会话，设置后开始请求会话流程 */
	virtual void SetRequestedSession(UCommonSession_SearchResult* InRequestedSession);
	/** 检查是否可以加入请求的会话，可按游戏重写 */
	virtual bool CanJoinRequestedSession() const;
	/** 加入请求的会话 */
	virtual void JoinRequestedSession();
	/** 将游戏重置到可加入请求会话的状态 */
	virtual void ResetGameAndJoinRequestedSession();

	// 添加本地玩家
	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	// 移除本地玩家
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	// 初始化 GameInstance
	virtual void Init() override;
	// 返回主菜单
	virtual void ReturnToMainMenu() override;

private:
	/** 主玩家引用 */
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
	/** 玩家请求加入的会话 */
	UPROPERTY()
	TObjectPtr<UCommonSession_SearchResult> RequestedSession;
};
