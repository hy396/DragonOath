// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：用户、平台和在线会话辅助插件，后续登录、房间、匹配阶段再深入接入。

#pragma once

#include "CommonUserTypes.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"
#include "GameplayTagContainer.h"
#include "CommonUserSubsystem.generated.h"

#if COMMONUSER_OSSV1
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineError.h"
#else
#include "Online/OnlineAsyncOpHandle.h"
#endif

class FNativeGameplayTag;
class IOnlineSubsystem;

/** 通用用户子系统使用的 GameplayTag 集合 */
struct COMMONUSER_API FCommonUserTags
{
	// 通用严重性级别和系统消息
	static FNativeGameplayTag SystemMessage_Error;		// SystemMessage.Error
	static FNativeGameplayTag SystemMessage_Warning;	// SystemMessage.Warning
	static FNativeGameplayTag SystemMessage_Display;	// SystemMessage.Display

	/** 所有初始化尝试均失败，用户需要执行操作后才能重试 */
	static FNativeGameplayTag SystemMessage_Error_InitializeLocalPlayerFailed; // SystemMessage.Error.InitializeLocalPlayerFailed

	// 平台特征标签——由 GameInstance 或其他系统在对应平台设置

	/** 主机平台：控制器 ID 严格映射到不同系统用户 */
	static FNativeGameplayTag Platform_Trait_RequiresStrictControllerMapping; // Platform.Trait.RequiresStrictControllerMapping

	/** 平台只有单一在线用户，所有玩家使用索引 0 */
	static FNativeGameplayTag Platform_Trait_SingleOnlineUser; // Platform.Trait.SingleOnlineUser
};

/**
 * 用户信息（User Info）
 *
 * 逻辑上表示一个独立的用户——每个被初始化的本地玩家都有一个实例。
 * 存储该用户的输入设备、平台用户 ID、登录状态、权限缓存等。
 */
UCLASS(BlueprintType)
class COMMONUSER_API UCommonUserInfo : public UObject
{
	GENERATED_BODY()

public:
	/** 该用户的主控制器输入设备（可能有多个辅助设备） */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** 本地平台的逻辑用户 ID，访客用户指向主用户 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** LocalPlayer 在 GameInstance 数组中的索引（完全创建后生效），-1 表示未分配 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	/** 是否允许以访客身份登录 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	/** 是否为挂靠主用户 0 的访客 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	/** 整体初始化状态 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	ECommonUserInitializationState InitializationState = ECommonUserInitializationState::Invalid;

	/** Returns true if this user has successfully logged in */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	bool IsLoggedIn() const;

	/** Returns true if this user is in the middle of logging in */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	bool IsDoingLogin() const;

	/** Returns the most recently queries result for a specific privilege, will return unknown if never queried */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	ECommonUserPrivilegeResult GetCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Ask about the general availability of a feature, this combines cached results with state */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	ECommonUserAvailability GetPrivilegeAvailability(ECommonUserPrivilege Privilege) const;

	/** Returns the net id for the given context */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	FUniqueNetIdRepl GetNetId(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the user's human readable nickname */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	FString GetNickname() const;

	/** Returns an internal debug string for this player */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	FString GetDebugString() const;

	/** Accessor for platform user id */
	FPlatformUserId GetPlatformUserId() const;

	/** Gets the platform user index for older functions expecting an integer */
	int32 GetPlatformUserIndex() const;

	// Internal data, only intended to be accessed by online subsystems

	/** Cached data for each online system */
	struct FCachedData
	{
		/** Cached net id per system */
		FUniqueNetIdRepl CachedNetId;

		/** Cached values of various user privileges */
		TMap<ECommonUserPrivilege, ECommonUserPrivilegeResult> CachedPrivileges;
	};

	/** Per context cache, game will always exist but others may not */
	TMap<ECommonUserOnlineContext, FCachedData> CachedDataMap;

	/** Looks up cached data using resolution rules */
	FCachedData* GetCachedData(ECommonUserOnlineContext Context);
	const FCachedData* GetCachedData(ECommonUserOnlineContext Context) const;

	/** Updates cached privilege results, will propagate to game if needed */
	void UpdateCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context);

	/** Updates cached privilege results, will propagate to game if needed */
	void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, ECommonUserOnlineContext Context);

	/** Return the subsystem this is owned by */
	class UCommonUserSubsystem* GetSubsystem() const;
};


/** 初始化完成委托（成功或失败） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FCommonUserOnInitializeCompleteMulticast, const UCommonUserInfo*, UserInfo, bool, bSuccess, FText, Error, ECommonUserPrivilege, RequestedPrivilege, ECommonUserOnlineContext, OnlineContext);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FCommonUserOnInitializeComplete, const UCommonUserInfo*, UserInfo, bool, bSuccess, FText, Error, ECommonUserPrivilege, RequestedPrivilege, ECommonUserOnlineContext, OnlineContext);

/** 系统错误消息委托——游戏可据此向用户展示，MessageType 为严重性标签 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCommonUserHandleSystemMessageDelegate, FGameplayTag, MessageType, FText, TitleText, FText, BodyText);

/** 权限可用性变化委托——可用来监听在线状态等变化 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCommonUserAvailabilityChangedDelegate, const UCommonUserInfo*, UserInfo, ECommonUserPrivilege, Privilege, ECommonUserAvailability, OldAvailability, ECommonUserAvailability, NewAvailability);


/** 用户初始化参数——通常由异步节点等包装函数填写 */
USTRUCT(BlueprintType)
struct COMMONUSER_API FCommonUserInitializeParams
{
	GENERATED_BODY()

	/** LocalPlayer 索引（0 = 主玩家，可指定超出当前数量的索引以创建新玩家） */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 LocalPlayerIndex = 0;

	/** 已弃用——请使用 PrimaryInputDevice / PlatformUser 替代 ControllerId */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 ControllerId = -1;

	/** 该用户的主输入设备 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** Specifies the logical user on the local platform */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** Generally either CanPlay or CanPlayOnline, specifies what level of privilege is required */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	ECommonUserPrivilege RequestedPrivilege = ECommonUserPrivilege::CanPlay;

	/** What specific online context to log in to, game means to login to all relevant ones */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	ECommonUserOnlineContext OnlineContext = ECommonUserOnlineContext::Game;

	/** True if this is allowed to create a new local player for initial login */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanCreateNewLocalPlayer = false;

	/** True if this player can be a guest user without an actual online presence */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanUseGuestLogin = false;

	/** True if we should not show login errors, the game will be responsible for displaying them */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bSuppressLoginErrors = false;

	/** If bound, call this dynamic delegate at completion of login */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	FCommonUserOnInitializeComplete OnUserInitializeComplete;
};

/**
 * Game subsystem that handles queries and changes to user identity and login status.
 * One subsystem is created for each game instance and can be accessed from blueprints or C++ code.
 * If a game-specific subclass exists, this base subsystem will not be created.
 */
UCLASS(BlueprintType, Config=Engine)
class COMMONUSER_API UCommonUserSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonUserSubsystem() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;


	/** BP delegate called when any requested initialization request completes */
	UPROPERTY(BlueprintAssignable, Category = CommonUser)
	FCommonUserOnInitializeCompleteMulticast OnUserInitializeComplete;

	/** BP delegate called when the system sends an error/warning message */
	UPROPERTY(BlueprintAssignable, Category = CommonUser)
	FCommonUserHandleSystemMessageDelegate OnHandleSystemMessage;

	/** BP delegate called when privilege availability changes for a user  */
	UPROPERTY(BlueprintAssignable, Category = CommonUser)
	FCommonUserAvailabilityChangedDelegate OnUserPrivilegeChanged;

	/** Send a system message via OnHandleSystemMessage */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual void SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText);

	/** Sets the maximum number of local players, will not destroy existing ones */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual void SetMaxLocalPlayers(int32 InMaxLocalPLayers);

	/** Gets the maximum number of local players */
	UFUNCTION(BlueprintPure, Category = CommonUser)
	int32 GetMaxLocalPlayers() const;

	/** Gets the current number of local players, will always be at least 1 */
	UFUNCTION(BlueprintPure, Category = CommonUser)
	int32 GetNumLocalPlayers() const;

	/** Returns the state of initializing the specified local player */
	UFUNCTION(BlueprintPure, Category = CommonUser)
	ECommonUserInitializationState GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const;

	/** Returns the user info for a given local player index in game instance, 0 is always valid in a running game */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	const UCommonUserInfo* GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const;

	/** Deprecated, use PlatformUserId when available */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	const UCommonUserInfo* GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const;

	/** Returns the primary user info for a given platform user index. Can return null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	const UCommonUserInfo* GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const;

	/** Returns the user info for a unique net id. Can return null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	const UCommonUserInfo* GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const;

	/** Deprecated, use InputDeviceId when available */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	const UCommonUserInfo* GetUserInfoForControllerId(int32 ControllerId) const;

	/** Returns the user info for a given input device. Can return null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = CommonUser)
	const UCommonUserInfo* GetUserInfoForInputDevice(FInputDeviceId InputDevice) const;

	/**
	 * Tries to start the process of creating or updating a local player, including logging in and creating a player controller.
	 * When the process has succeeded or failed, it will broadcast the OnUserInitializeComplete delegate.
	 *
	 * @param LocalPlayerIndex	Desired index of LocalPlayer in Game Instance, 0 will be primary player and 1+ for local multiplayer
	 * @param PrimaryInputDevice The physical controller that should be mapped to this user, will use the default device if invalid
	 * @param bCanUseGuestLogin	If true, this player can be a guest without a real Unique Net Id
	 *
	 * @returns true if the process was started, false if it failed before properly starting
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual bool TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * Starts the process of taking a locally logged in user and doing a full online login including account permission checks.
	 * When the process has succeeded or failed, it will broadcast the OnUserInitializeComplete delegate.
	 *
	 * @param LocalPlayerIndex	Index of existing LocalPlayer in Game Instance
	 *
	 * @returns true if the process was started, false if it failed before properly starting
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual bool TryToLoginForOnlinePlay(int32 LocalPlayerIndex);

	/**
	 * Starts a general user login and initialization process, using the params structure to determine what to log in to.
	 * When the process has succeeded or failed, it will broadcast the OnUserInitializeComplete delegate.
	 * AsyncAction_CommonUserInitialize provides several wrapper functions for using this in an Event graph.
	 *
	 * @returns true if the process was started, false if it failed before properly starting
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual bool TryToInitializeUser(FCommonUserInitializeParams Params);

	/**
	 * Starts the process of listening for user input for new and existing controllers and logging them.
	 * This will insert a key input handler on the active GameViewportClient and is turned off by calling again with empty key arrays.
	 *
	 * @param AnyUserKeys		Listen for these keys for any user, even the default user. Set this for an initial press start screen or empty to disable
	 * @param NewUserKeys		Listen for these keys for a new user without a player controller. Set this for splitscreen/local multiplayer or empty to disable
	 * @param Params			Params passed to TryToInitializeUser after detecting key input
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual void ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FCommonUserInitializeParams Params);

	/** Attempts to cancel an in-progress initialization attempt, this may not work on all platforms but will disable callbacks */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual bool CancelUserInitialization(int32 LocalPlayerIndex);

	/** Logs a player out of any online systems, and optionally destroys the player entirely if it's not the first one */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual bool TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer = false);

	/** Resets the login and initialization state when returning to the main menu after an error */
	UFUNCTION(BlueprintCallable, Category = CommonUser)
	virtual void ResetUserState();

	/** Returns true if this this could be a real platform user with a valid identity (even if not currently logged in)  */
	virtual bool IsRealPlatformUserIndex(int32 PlatformUserIndex) const;

	/** Returns true if this this could be a real platform user with a valid identity (even if not currently logged in) */
	virtual bool IsRealPlatformUser(FPlatformUserId PlatformUser) const;

	/** Converts index to id */
	virtual FPlatformUserId GetPlatformUserIdForIndex(int32 PlatformUserIndex) const;

	/** Converts id to index */
	virtual int32 GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const;

	/** Gets the user for an input device */
	virtual FPlatformUserId GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const;

	/** Gets a user's primary input device id */
	virtual FInputDeviceId GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const;

	/** Call from game code to set the cached trait tags when platform state or options changes */
	virtual void SetTraitTags(const FGameplayTagContainer& InTags);

	/** Gets the current tags that affect feature avialability */
	const FGameplayTagContainer& GetTraitTags() const { return CachedTraitTags; }

	/** Checks if a specific platform/feature tag is enabled */
	UFUNCTION(BlueprintPure, Category=CommonUser)
	bool HasTraitTag(const FGameplayTag TraitTag) const { return CachedTraitTags.HasTag(TraitTag); }

	/** Checks to see if we should display a press start/input confirmation screen at startup. Games can call this or check the trait tags directly */
	UFUNCTION(BlueprintPure, BlueprintPure, Category=CommonUser)
	virtual bool ShouldWaitForStartInput() const;


	// Functions for accessing low-level online system information

#if COMMONUSER_OSSV1
	/** Returns OSS interface of specific type, will return null if there is no type */
	IOnlineSubsystem* GetOnlineSubsystem(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns identity interface of specific type, will return null if there is no type */
	IOnlineIdentity* GetOnlineIdentity(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns human readable name of OSS system */
	FName GetOnlineSubsystemName(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the current online connection status */
	EOnlineServerConnectionStatus::Type GetConnectionStatus(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;
#else
	/** Get the services provider type, or None if there isn't one. */
	UE::Online::EOnlineServices GetOnlineServicesProvider(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns auth interface of specific type, will return null if there is no type */
	UE::Online::IAuthPtr GetOnlineAuth(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the current online connection status */
	UE::Online::EOnlineServicesConnectionStatus GetConnectionStatus(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;
#endif

	/** Returns true if we are currently connected to backend servers */
	bool HasOnlineConnection(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the current login status for a player on the specified online system, only works for real platform users */
	ELoginStatusType GetLocalUserLoginStatus(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Returns the unique net id for a local platform user */
	FUniqueNetIdRepl GetLocalUserNetId(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	/** Convert a user id to a debug string */
	FString PlatformUserIdToString(FPlatformUserId UserId);

	/** Convert a context to a debug string */
	FString ECommonUserOnlineContextToString(ECommonUserOnlineContext Context);

	/** Returns human readable string for privilege checks */
	virtual FText GetPrivilegeDescription(ECommonUserPrivilege Privilege) const;
	virtual FText GetPrivilegeResultDescription(ECommonUserPrivilegeResult Result) const;

	/**
	 * Starts the process of login for an existing local user, will return false if callback was not scheduled
	 * This activates the low level state machine and does not modify the initialization state on user info
	 */
	DECLARE_DELEGATE_FiveParams(FOnLocalUserLoginCompleteDelegate, const UCommonUserInfo* /*UserInfo*/, ELoginStatusType /*NewStatus*/, FUniqueNetIdRepl /*NetId*/, const TOptional<FOnlineErrorType>& /*Error*/, ECommonUserOnlineContext /*Type*/);
	virtual bool LoginLocalUser(const UCommonUserInfo* UserInfo, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext Context, FOnLocalUserLoginCompleteDelegate OnComplete);

	/** Assign a local player to a specific local user and call callbacks as needed */
	virtual void SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UCommonUserInfo* UserInfo);

	/** Resolves a context that has default behavior into a specific context */
	ECommonUserOnlineContext ResolveOnlineContext(ECommonUserOnlineContext Context) const;

	/** True if there is a separate platform and service interface */
	bool HasSeparatePlatformContext() const;

protected:
	/** Internal structure that caches status and pointers for each online context */
	struct FOnlineContextCache
	{
#if COMMONUSER_OSSV1
		/** Pointer to base subsystem, will stay valid as long as game instance does */
		IOnlineSubsystem* OnlineSubsystem = nullptr;

		/** Cached identity system, this will always be valid */
		IOnlineIdentityPtr IdentityInterface;

		/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander */
		EOnlineServerConnectionStatus::Type	CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#else
		/** Online services, accessor to specific services */
		UE::Online::IOnlineServicesPtr OnlineServices;
		/** Cached auth service */
		UE::Online::IAuthPtr AuthService;
		/** Login status changed event handle */
		UE::Online::FOnlineEventDelegateHandle LoginStatusChangedHandle;
		/** Connection status changed event handle */
		UE::Online::FOnlineEventDelegateHandle ConnectionStatusChangedHandle;
		/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander */
		UE::Online::EOnlineServicesConnectionStatus CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif

		/** Resets state, important to clear all shared ptrs */
		void Reset()
		{
#if COMMONUSER_OSSV1
			OnlineSubsystem = nullptr;
			IdentityInterface.Reset();
			CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
#else
			OnlineServices.Reset();
			AuthService.Reset();
			CurrentConnectionStatus = UE::Online::EOnlineServicesConnectionStatus::NotConnected;
#endif
		}
	};

	/** Internal structure to represent an in-progress login request */
	struct FUserLoginRequest : public TSharedFromThis<FUserLoginRequest>
	{
		FUserLoginRequest(UCommonUserInfo* InUserInfo, ECommonUserPrivilege InPrivilege, ECommonUserOnlineContext InContext, FOnLocalUserLoginCompleteDelegate&& InDelegate)
			: UserInfo(TWeakObjectPtr<UCommonUserInfo>(InUserInfo))
			, DesiredPrivilege(InPrivilege)
			, DesiredContext(InContext)
			, Delegate(MoveTemp(InDelegate))
			{}

		/** Which local user is trying to log on */
		TWeakObjectPtr<UCommonUserInfo> UserInfo;

		/** Overall state of login request, could come from many sources */
		ECommonUserAsyncTaskState OverallLoginState = ECommonUserAsyncTaskState::NotStarted;

		/** State of attempt to use platform auth. When started, this immediately transitions to Failed for OSSv1, as we do not support platform auth there. */
		ECommonUserAsyncTaskState TransferPlatformAuthState = ECommonUserAsyncTaskState::NotStarted;

		/** State of attempt to use AutoLogin */
		ECommonUserAsyncTaskState AutoLoginState = ECommonUserAsyncTaskState::NotStarted;

		/** State of attempt to use external login UI */
		ECommonUserAsyncTaskState LoginUIState = ECommonUserAsyncTaskState::NotStarted;

		/** Final privilege to that is requested */
		ECommonUserPrivilege DesiredPrivilege = ECommonUserPrivilege::Invalid_Count;

		/** State of attempt to request the relevant privilege */
		ECommonUserAsyncTaskState PrivilegeCheckState = ECommonUserAsyncTaskState::NotStarted;

		/** The final context to log into */
		ECommonUserOnlineContext DesiredContext = ECommonUserOnlineContext::Invalid;

		/** What online system we are currently logging into */
		ECommonUserOnlineContext CurrentContext = ECommonUserOnlineContext::Invalid;

		/** User callback for completion */
		FOnLocalUserLoginCompleteDelegate Delegate;

		/** Most recent/relevant error to display to user */
		TOptional<FOnlineErrorType> Error;
	};


	/** Create a new user info object */
	virtual UCommonUserInfo* CreateLocalUserInfo(int32 LocalPlayerIndex);

	/** Deconst wrapper for const getters */
	FORCEINLINE UCommonUserInfo* ModifyInfo(const UCommonUserInfo* Info) { return const_cast<UCommonUserInfo*>(Info); }

	/** Refresh user info from OSS */
	virtual void RefreshLocalUserInfo(UCommonUserInfo* UserInfo);

	/** Possibly send privilege availability notification, compares current value to cached old value */
	virtual void HandleChangedAvailability(UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability);

	/** Updates the cached privilege on a user and notifies delegate */
	virtual void UpdateUserPrivilegeResult(UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context);

	/** Gets internal data for a type of online system, can return null for service */
	const FOnlineContextCache* GetContextCache(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;
	FOnlineContextCache* GetContextCache(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game);

	/** Create and set up system objects before delegates are bound */
	virtual void CreateOnlineContexts();
	virtual void DestroyOnlineContexts();

	/** Bind online delegates */
	virtual void BindOnlineDelegates();

	/** Forcibly logs out and deinitializes a single user */
	virtual void LogOutLocalUser(FPlatformUserId PlatformUser);

	/** Performs the next step of a login request, which could include completing it. Returns true if it's done */
	virtual void ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request);

	/** Call login on OSS, with platform auth from the platform OSS. Return true if AutoLogin started */
	virtual bool TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** Call AutoLogin on OSS. Return true if AutoLogin started. */
	virtual bool AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** Call ShowLoginUI on OSS. Return true if ShowLoginUI started. */
	virtual bool ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** Call QueryUserPrivilege on OSS. Return true if QueryUserPrivilege started. */
	virtual bool QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** OSS-specific functions */
#if COMMONUSER_OSSV1
	virtual ECommonUserPrivilege ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const;
	virtual EUserPrivileges::Type ConvertOSSPrivilege(ECommonUserPrivilege Privilege) const;
	virtual ECommonUserPrivilegeResult ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const;

	void BindOnlineDelegatesOSSv1();
	bool AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	bool ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	bool QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
#else
	virtual ECommonUserPrivilege ConvertOnlineServicesPrivilege(UE::Online::EUserPrivileges Privilege) const;
	virtual UE::Online::EUserPrivileges ConvertOnlineServicesPrivilege(ECommonUserPrivilege Privilege) const;
	virtual ECommonUserPrivilegeResult ConvertOnlineServicesPrivilegeResult(UE::Online::EUserPrivileges Privilege, UE::Online::EPrivilegeResults Results) const;

	void BindOnlineDelegatesOSSv2();
	void CacheConnectionStatus(ECommonUserOnlineContext Context);
	bool TransferPlatformAuthOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	bool AutoLoginOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	bool ShowLoginUIOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	bool QueryUserPrivilegeOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	TSharedPtr<UE::Online::FAccountInfo> GetOnlineServiceAccountInfo(UE::Online::IAuthPtr AuthService, FPlatformUserId InUserId) const;
#endif

	/** Callbacks for OSS functions */
#if COMMONUSER_OSSV1
	virtual void HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId, ECommonUserOnlineContext Context);
	virtual void HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& Error, ECommonUserOnlineContext Context);
	virtual void HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser);
	virtual void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus, ECommonUserOnlineContext Context);
	virtual void HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error, ECommonUserOnlineContext Context);
	virtual void HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, ECommonUserPrivilege RequestedPrivilege, TWeakObjectPtr<UCommonUserInfo> CommonUserInfo, ECommonUserOnlineContext Context);
#else
	virtual void HandleAuthLoginStatusChanged(const UE::Online::FAuthLoginStatusChanged& EventParameters, ECommonUserOnlineContext Context);
	virtual void HandleUserLoginCompletedV2(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, FPlatformUserId PlatformUser, ECommonUserOnlineContext Context);
	virtual void HandleOnLoginUIClosedV2(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result, FPlatformUserId PlatformUser, ECommonUserOnlineContext Context);
	virtual void HandleNetworkConnectionStatusChanged(const UE::Online::FConnectionStatusChanged& EventParameters, ECommonUserOnlineContext Context);
	virtual void HandleCheckPrivilegesComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserPrivilege>& Result, TWeakObjectPtr<UCommonUserInfo> CommonUserInfo, UE::Online::EUserPrivileges DesiredPrivilege, ECommonUserOnlineContext Context);
#endif

	/**
	 * Callback for when an input device (i.e. a gamepad) has been connected or disconnected.
	 */
	virtual void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	virtual void HandleLoginForUserInitialize(const UCommonUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& Error, ECommonUserOnlineContext Context, FCommonUserInitializeParams Params);
	virtual void HandleUserInitializeFailed(FCommonUserInitializeParams Params, FText Error);
	virtual void HandleUserInitializeSucceeded(FCommonUserInitializeParams Params);

	/** Callback for handling press start/login logic */
	virtual bool OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs);


	/** Previous override handler, will restore on cancel */
	FOverrideInputKeyHandler WrappedInputKeyHandler;

	/** List of keys to listen for from any user */
	TArray<FKey> LoginKeysForAnyUser;

	/** List of keys to listen for a new unmapped user */
	TArray<FKey> LoginKeysForNewUser;

	/** Params to use for a key-triggered login */
	FCommonUserInitializeParams ParamsForLoginKey;

	/** Maximum number of local players */
	int32 MaxNumberOfLocalPlayers = 0;

	/** True if this is a dedicated server, which doesn't require a LocalPlayer */
	bool bIsDedicatedServer = false;

	/** List of current in progress login requests */
	TArray<TSharedRef<FUserLoginRequest>> ActiveLoginRequests;

	/** Information about each local user, from local player index to user */
	UPROPERTY()
	TMap<int32, TObjectPtr<UCommonUserInfo>> LocalUserInfos;

	/** Cached platform/mode trait tags */
	FGameplayTagContainer CachedTraitTags;

	/** Do not access this outside of initialization */
	FOnlineContextCache* DefaultContextInternal = nullptr;
	FOnlineContextCache* ServiceContextInternal = nullptr;
	FOnlineContextCache* PlatformContextInternal = nullptr;

	friend UCommonUserInfo;
};
