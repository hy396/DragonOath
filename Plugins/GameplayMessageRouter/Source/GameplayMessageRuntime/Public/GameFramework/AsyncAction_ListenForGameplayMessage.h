// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayMessageSubsystem.h"
#include "GameplayMessageTypes2.h"

#include "AsyncAction_ListenForGameplayMessage.generated.h"

#define UE_API GAMEPLAYMESSAGERUNTIME_API

class UScriptStruct;
class UWorld;
struct FFrame;

/**
 * 监听到消息时触发的蓝图委托
 *
 * @param ProxyObject    本 AsyncAction 自身（蓝图中不可见——由 K2Node 内部接到 GetPayload 调用）
 * @param ActualChannel  实际收到消息的频道 Tag（PartialMatch 时可能比注册时指定的频道更具体）
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForGameplayMessage*, ProxyObject, FGameplayTag, ActualChannel);

/**
 * 蓝图异步动作：监听 Gameplay 消息
 *
 * 这是 `UGameplayMessageSubsystem` 的蓝图包装。用户在蓝图里放一个
 * `Listen For Gameplay Messages` 节点即可：
 *   - 指定 Channel（FGameplayTag）
 *   - 指定 PayloadType（UScriptStruct）
 *   - 可选 MatchType（精确/部分匹配）
 *
 * 节点会持续监听，直到：
 *   - 蓝图持有者销毁
 *   - 手动断开（通过 CancellableAsyncAction::Cancel）
 *
 * 对应的编辑器节点实现在 `UK2Node_AsyncAction_ListenForGameplayMessages`，
 * 它会把输出的 "Payload" 通配符引脚改成用户指定的 PayloadType。
 */
UCLASS(MinimalAPI, BlueprintType, meta=(HasDedicatedAsyncNode))
class UAsyncAction_ListenForGameplayMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 创建一个异步监听节点（蓝图工厂函数）
	 *
	 * @param WorldContextObject  世界上下文
	 * @param Channel             监听频道
	 * @param PayloadType         期望的消息结构体类型（必须和发送方一致）
	 * @param MatchType           匹配规则
	 */
	UFUNCTION(BlueprintCallable, Category = Messaging, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_ListenForGameplayMessage* ListenForGameplayMessages(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EGameplayMessageMatch MatchType = EGameplayMessageMatch::ExactMatch);

	/**
	 * 把收到的消息 Payload 拷贝到一个通配符引脚
	 *
	 * 通配符必须和 ListenForGameplayMessages 的 PayloadType 一致，否则拷贝失败。
	 * 该函数不会被直接调用——K2Node 会在编译期自动为蓝图连线。
	 *
	 * @param OutPayload  用于接收拷贝结果的通配符引脚
	 * @return            拷贝是否成功
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Messaging", meta = (CustomStructureParam = "OutPayload"))
	UE_API bool GetPayload(UPARAM(ref) int32& OutPayload);

	DECLARE_FUNCTION(execGetPayload);

	UE_API virtual void Activate() override;
	UE_API virtual void SetReadyToDestroy() override;

public:
	/** 消息到达时广播。蓝图通过 GetPayload 节点取出 Payload 内容 */
	UPROPERTY(BlueprintAssignable)
	FAsyncGameplayMessageDelegate OnMessageReceived;

private:
	// 消息到达的处理：校验类型 → 暂存 Payload → 广播委托 → 清空 Payload
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

private:
	// 当前正在处理的 Payload 指针（仅在 Broadcast 生命周期内有效）
	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	EGameplayMessageMatch MessageMatchType = EGameplayMessageMatch::ExactMatch;

	// 向 Subsystem 注册得到的句柄——SetReadyToDestroy 时反注册
	FGameplayMessageListenerHandle ListenerHandle;
};

#undef UE_API
