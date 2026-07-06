// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：用户、平台和在线会话辅助插件，后续登录、房间、匹配阶段再深入接入。

#pragma once


#if COMMONUSER_OSSV1

// Online Subsystem (OSS v1) includes and forward declares
#include "OnlineSubsystemTypes.h"
class IOnlineSubsystem;
struct FOnlineError;
using FOnlineErrorType = FOnlineError;
using ELoginStatusType = ELoginStatus::Type;

#else

// Online Services (OSS v2) includes and forward declares
#include "Online/Connectivity.h"
#include "Online/OnlineError.h"
namespace UE::Online
{
	enum class ELoginStatus : uint8;
	enum class EPrivilegeResults : uint32;
	enum class EUserPrivileges : uint8;
	using IAuthPtr = TSharedPtr<class IAuth>;
	using IOnlineServicesPtr = TSharedPtr<class IOnlineServices>;
	template <typename OpType>
	class TOnlineResult;
	struct FAuthLogin;
	struct FConnectionStatusChanged;
	struct FExternalUIShowLoginUI;
	struct FAuthLoginStatusChanged;
	struct FQueryUserPrivilege;
	struct FAccountInfo;
}
using FOnlineErrorType = UE::Online::FOnlineError;
using ELoginStatusType = UE::Online::ELoginStatus;

#endif

#include "CommonUserTypes.generated.h"


/** 在线上下文（Online Context）——指定在线查询的目标系统 */
UENUM(BlueprintType)
enum class ECommonUserOnlineContext : uint8
{
	Game,				// 游戏代码调用，使用默认系统（可能合并多个上下文的结果）
	Default,			// 引擎默认在线系统（等同于 Service 或 Platform）
	Service,			// 外部服务（如 EOS），可能不存在
	ServiceOrDefault,	// 优先外部服务，回退到默认
	Platform,			// 平台系统（如 Xbox Live / PSN），可能不存在
	PlatformOrDefault,	// 优先平台系统，回退到默认
	Invalid				// 无效
};

/** 用户初始化状态 */
UENUM(BlueprintType)
enum class ECommonUserInitializationState : uint8
{
	Unknown,			// 尚未开始登录流程
	DoingInitialLogin,	// 正在获取本地用户 ID
	DoingNetworkLogin,	// 已完成本地登录，正在执行网络登录
	FailedtoLogin,		// 登录完全失败
	LoggedInOnline,		// 已登录，可使用在线功能
	LoggedInLocalOnly,	// 仅本地登录（访客或真实用户），无法执行在线操作
	Invalid,			// 无效状态或无效用户
};

/** 用户权限/能力（Privilege） */
UENUM(BlueprintType)
enum class ECommonUserPrivilege : uint8
{
	CanPlay,						// 能否游玩（不论在线/离线）
	CanPlayOnline,					// 能否在线游玩
	CanCommunicateViaTextOnline,	// 能否使用文字聊天
	CanCommunicateViaVoiceOnline,	// 能否使用语音聊天
	CanUseUserGeneratedContent,		// 能否访问用户生成内容
	CanUseCrossPlay,				// 能否跨平台游玩
	Invalid_Count					UMETA(Hidden)	// 无效/计数
};

/** 功能/权限可用性 */
UENUM(BlueprintType)
enum class ECommonUserAvailability : uint8
{
	Unknown,				// 状态未知，需要查询
	NowAvailable,			// 当前可用
	PossiblyAvailable,		// 完成正常登录流程后可能可用
	CurrentlyUnavailable,	// 当前不可用（如断网），但未来可能恢复
	AlwaysUnavailable,		// 永远不可用（账号/平台硬性限制）
	Invalid,				// 无效
};

/** 权限检查结果——说明用户为何能/不能使用某权限 */
UENUM(BlueprintType)
enum class ECommonUserPrivilegeResult : uint8
{
	Unknown,						// 状态未知
	Available,						// 权限可用
	UserNotLoggedIn,				// 用户未登录
	LicenseInvalid,					// 用户未拥有游戏或内容
	VersionOutdated,				// 游戏需要更新
	NetworkConnectionUnavailable,	// 无网络连接
	AgeRestricted,					// 家长控制限制
	AccountTypeRestricted,			// 账号类型/订阅不足
	AccountUseRestricted,			// 账号被限制（如封禁）
	PlatformFailure,				// 平台特定错误
};

/** 异步任务状态 */
enum class ECommonUserAsyncTaskState : uint8
{
	NotStarted,	// 尚未开始
	InProgress,	// 进行中
	Done,		// 已成功完成
	Failed		// 已失败
};

/** 在线错误详情——对 FOnlineError 的封装 */
USTRUCT(BlueprintType)
struct FOnlineResultInformation
{
	GENERATED_BODY()

	/** 操作是否成功——成功时错误字段为空 */
	UPROPERTY(BlueprintReadOnly)
	bool bWasSuccessful = true;

	/** 唯一错误 ID，可用于与特定错误比对 */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorId;

	/** 显示给用户的错误文本 */
	UPROPERTY(BlueprintReadOnly)
	FText ErrorText;

	/**
	 * 从 FOnlineErrorType 初始化
	 * @param InOnlineError 在线错误对象
	 */
	void COMMONUSER_API FromOnlineError(const FOnlineErrorType& InOnlineError);
};
