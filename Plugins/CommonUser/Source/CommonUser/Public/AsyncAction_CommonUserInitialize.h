// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：用户、平台和在线会话辅助插件，后续登录、房间、匹配阶段再深入接入。

#pragma once

#include "CommonUserSubsystem.h"
#include "Engine/CancellableAsyncAction.h"

#include "AsyncAction_CommonUserInitialize.generated.h"

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;
struct FInputDeviceId;

class FText;
class UObject;
struct FFrame;

/**
 * 用户初始化异步动作（Blueprint Async Action）
 *
 * 提供蓝图可调用的异步节点，用于：
 * - InitializeForLocalPlay：初始化本地玩家（平台登录 + 权限检查）
 * - LoginForOnlinePlay：将已登录的本地用户登录到在线后端
 *
 * 完成或失败时广播 OnInitializationComplete 委托。
 */
UCLASS()
class COMMONUSER_API UAsyncAction_CommonUserInitialize : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 初始化本地玩家——执行平台登录和权限检查
	 *
	 * @param LocalPlayerIndex  LocalPlayer 在 GameInstance 中的索引（0 = 主玩家，1+ = 本地多人）
	 * @param PrimaryInputDevice 主输入设备，无效则使用系统默认
	 * @param bCanUseGuestLogin  是否允许以访客身份登录
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_CommonUserInitialize* InitializeForLocalPlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * 将已有本地用户登录到在线后端（启用完整在线功能）
	 *
	 * @param LocalPlayerIndex 已存在的 LocalPlayer 索引
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_CommonUserInitialize* LoginForOnlinePlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex);

	/** 初始化完成（成功或失败）时广播 */
	UPROPERTY(BlueprintAssignable)
	FCommonUserOnInitializeCompleteMulticast OnInitializationComplete;

	/** 失败时触发回调 */
	void HandleFailure();

	/** 内部委托包装——转发到 OnInitializationComplete */
	UFUNCTION()
	virtual void HandleInitializationComplete(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

protected:
	/** 异步动作激活时实际开始初始化 */
	virtual void Activate() override;

	TWeakObjectPtr<UCommonUserSubsystem> Subsystem;	// 目标 Subsystem 弱引用
	FCommonUserInitializeParams Params;				// 初始化参数
};
