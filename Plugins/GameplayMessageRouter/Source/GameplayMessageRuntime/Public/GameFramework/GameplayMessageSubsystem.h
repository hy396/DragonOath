// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayMessageTypes2.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"

#include "GameplayMessageSubsystem.generated.h"

#define UE_API GAMEPLAYMESSAGERUNTIME_API

class UGameplayMessageSubsystem;
struct FFrame;

GAMEPLAYMESSAGERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplayMessageSubsystem, Log, All);

class UAsyncAction_ListenForGameplayMessage;

/**
 * 消息监听句柄（Listener Handle）
 *
 * 每次调用 `RegisterListener` 都会返回一个句柄——把它保存好，
 * 销毁时调用 `Unregister()` 取消注册，避免悬垂回调。
 *
 * 该结构体对外不透明：内部字段（Subsystem / Channel / ID）都是 private，
 * 只能通过 `IsValid()` 判断是否有效，或通过 `Unregister()` 反注册。
 *
 * @see UGameplayMessageSubsystem::RegisterListener
 * @see UGameplayMessageSubsystem::UnregisterListener
 */
USTRUCT(BlueprintType)
struct FGameplayMessageListenerHandle
{
public:
	GENERATED_BODY()

	FGameplayMessageListenerHandle() {}

	/** 反注册该监听器。反复调用安全（内部会置空） */
	UE_API void Unregister();

	/** 是否是有效句柄（注册成功后才为 true） */
	bool IsValid() const { return ID != 0; }

private:
	// 所属的 Subsystem 弱引用（GC 后自动失效，避免悬垂）
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameplayMessageSubsystem> Subsystem;

	// 注册时指定的频道 Tag
	UPROPERTY(Transient)
	FGameplayTag Channel;

	// Subsystem 分配的递增 ID（0 = 无效）
	UPROPERTY(Transient)
	int32 ID = 0;

	FDelegateHandle StateClearedHandle;

	friend UGameplayMessageSubsystem;

	FGameplayMessageListenerHandle(UGameplayMessageSubsystem* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID) {}
};

/**
 * 单个监听器的内部数据（Subsystem 管理用）
 *
 * 外部代码不需要关心这个结构——它只负责记录：
 *   - 回调函数（类型擦除为 void* 形式的 Payload）
 *   - 期望的 USTRUCT 类型（用于运行时校验）
 *   - 匹配规则
 */
USTRUCT()
struct FGameplayMessageListenerData
{
	GENERATED_BODY()

	// 收到消息时要调用的函数（Payload 以类型擦除 void* 形式传入）
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID;
	EGameplayMessageMatch MatchType;

	// 监听器期望的消息结构体类型（用于类型校验；若发送方和监听方的 StructType 不兼容会报错）
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;
	bool bHadValidType = false;
};

/**
 * Gameplay 消息子系统（广播-订阅消息总线）
 *
 * ## 职责
 * 让"发送方"和"监听方"**互不认识**就能通信。双方只需约定：
 *   1. 使用同一个 GameplayTag 作为频道标识
 *   2. 使用同一个 USTRUCT 作为消息 Payload 类型
 *
 * ## 获取方式
 * ```cpp
 * // 方式一：通过 GameInstance
 * UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(GameInstance);
 *
 * // 方式二：通过任何有 World 上下文的对象
 * UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
 * ```
 *
 * ## 广播消息
 * ```cpp
 * FMyMessagePayload Payload { ... };
 * Router.BroadcastMessage(MyTag, Payload);
 * ```
 *
 * ## 监听消息
 * ```cpp
 * FGameplayMessageListenerHandle Handle = Router.RegisterListener<FMyMessagePayload>(
 *     MyTag,
 *     [](FGameplayTag Channel, const FMyMessagePayload& Payload) {
 *         // 处理消息
 *     });
 *
 * // 不再需要时
 * Handle.Unregister();
 * ```
 *
 * ## 调用顺序
 * 多个监听器监听同一频道时，**调用顺序不保证**，且可能随时间变化。
 * 不要依赖"哪个监听器先收到"这种假设。
 *
 * ## 日志
 * 通过控制台变量 `GameplayMessageSubsystem.LogMessages 1` 打开消息广播日志。
 */
UCLASS(MinimalAPI)
class UGameplayMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend UAsyncAction_ListenForGameplayMessage;

public:

	/**
	 * 拿到当前 World 对应 GameInstance 上的消息路由 Subsystem
	 *
	 * @param WorldContextObject  任意带 World 上下文的对象
	 * @return 消息路由 Subsystem 引用（断言确保非空）
	 */
	static UE_API UGameplayMessageSubsystem& Get(const UObject* WorldContextObject);

	/**
	 * 判断当前 World 是否有可用的消息路由 Subsystem
	 * （用于安全地在不确定上下文中调用，例如编辑器预览世界可能没有 GameInstance）
	 */
	static UE_API bool HasInstance(const UObject* WorldContextObject);

	//~USubsystem interface
	UE_API virtual void Deinitialize() override;
	//~End of USubsystem interface

	/**
	 * 向指定频道广播消息
	 *
	 * @param Channel  广播到的频道 Tag
	 * @param Message  消息体（类型必须和监听器注册时指定的 USTRUCT 一致，否则会日志报错）
	 *
	 * 内部会沿着 Tag 层级向父节点传播（例如广播 "A.B.C" 时，
	 * 监听 "A.B"（PartialMatch）和 "A"（PartialMatch）的监听器都会收到；
	 * 监听 "A.B"（ExactMatch）则不会收到）。
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * 注册一个监听器到指定频道（Lambda / 函数对象形式）
	 *
	 * @param Channel    要监听的频道 Tag
	 * @param Callback   消息到达时调用的回调（Payload 类型必须和发送方一致）
	 * @param MatchType  匹配规则（精确匹配 or 部分匹配，默认精确匹配）
	 *
	 * @return 监听句柄——请保存到成员变量，对象销毁时调用 Unregister() 防止悬垂
	 */
	template <typename FMessageStructType>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TFunction<void(FGameplayTag, const FMessageStructType&)>&& Callback, EGameplayMessageMatch MatchType = EGameplayMessageMatch::ExactMatch)
	{
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
		};

		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		return RegisterListenerInternal(Channel, ThunkCallback, StructType, MatchType);
	}

	/**
	 * 注册一个监听器到指定频道（UObject 成员函数形式，自动弱引用检查）
	 *
	 * 内部会用 WeakObjectPtr 持有对象——对象被 GC 后回调会自动跳过，
	 * **不会崩**，但你仍然应该在对象销毁前主动 Unregister。
	 *
	 * @param Channel   要监听的频道 Tag
	 * @param Object    目标 UObject 实例
	 * @param Function  目标类的成员函数指针（签名必须是 void(FGameplayTag, const FMessageStructType&)）
	 *
	 * @return 监听句柄
	 */
	template <typename FMessageStructType, typename TOwner = UObject>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object, void(TOwner::* Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return RegisterListener<FMessageStructType>(Channel,
			[WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
			{
				if (TOwner* StrongObject = WeakObject.Get())
				{
					(StrongObject->*Function)(Channel, Payload);
				}
			});
	}

	/**
	 * 注册一个监听器（参数结构体形式，便于一次性配置多个高级参数）
	 *
	 * 当你要同时设置"匹配规则 + 成员函数回调"时，用这个重载最清晰：
	 * ```cpp
	 * FGameplayMessageListenerParams<FMyMessage> Params;
	 * Params.MatchType = EGameplayMessageMatch::PartialMatch;
	 * Params.SetMessageReceivedCallback(this, &UMyClass::OnMessage);
	 * Handle = Router.RegisterListener(MyTag, Params);
	 * ```
	 */
	template <typename FMessageStructType>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, FGameplayMessageListenerParams<FMessageStructType>& Params)
	{
		FGameplayMessageListenerHandle Handle;

		// Register to receive any future messages broadcast on this channel
		if (Params.OnMessageReceivedCallback)
		{
			auto ThunkCallback = [InnerCallback = Params.OnMessageReceivedCallback](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
			{
				InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
			};

			const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
			Handle = RegisterListenerInternal(Channel, ThunkCallback, StructType, Params.MatchType);
		}

		return Handle;
	}

	/**
	 * 反注册一个监听器
	 *
	 * 两种方式是等价的：
	 *   - `Handle.Unregister()` （推荐，更简洁）
	 *   - `Router.UnregisterListener(Handle)` （兼容写法）
	 *
	 * @param Handle  之前 RegisterListener 返回的句柄
	 */
	UE_API void UnregisterListener(FGameplayMessageListenerHandle Handle);

protected:
	/**
	 * 蓝图节点版的广播（通过 CustomThunk 处理通配符 Message 参数）
	 *
	 * 不要直接调用这个 C++ 函数——它永远不会执行（会 checkNoEntry），
	 * 实际执行路径在下面的 execK2_BroadcastMessage 里。
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category=Messaging, meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="Broadcast Message"))
	UE_API void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);

	DECLARE_FUNCTION(execK2_BroadcastMessage);

private:
	// 广播消息的内部实现（类型擦除为 UScriptStruct + void*）
	UE_API void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

	// 注册监听器的内部实现（类型擦除）
	UE_API FGameplayMessageListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType,
		EGameplayMessageMatch MatchType);

	UE_API void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

private:
	// 单个频道上的监听器列表（带递增的 HandleID 分配器）
	struct FChannelListenerList
	{
		TArray<FGameplayMessageListenerData> Listeners;
		int32 HandleID = 0;
	};

private:
	// 频道 Tag → 该频道所有监听器
	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};

#undef UE_API
