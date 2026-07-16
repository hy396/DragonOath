// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Messages/DOVerbMessage.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "DOVerbMessageReplication.generated.h"

class UObject;
struct FDOVerbMessageReplication;
struct FNetDeltaSerializeInfo;

/**
 * DragonOath 的"事件记录型"网络复制容器（FastArray）。
 *
 * 一条 verb 消息（封装单条 FDOVerbMessage，作为 FastArray 的元素项）。
 *
 * 服务端调用 AddMessage(...) 把消息塞进容器，UE 通过 FastArraySerializer 增量复制到所有客户端；
 * 客户端在 PostReplicatedAdd / PostReplicatedChange 回调里把收到的消息再次广播到本地
 * UGameplayMessageSubsystem——这样"事件记录型"消息（伤害 / 击杀）能让所有客户端触发普通订阅方，
 * 与 PlayerState 上的 Client RPC 路径相互独立，但最终落到同一个本地总线。
 */
USTRUCT(BlueprintType)
struct FDOVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FDOVerbMessageReplicationEntry()
	{}

	FDOVerbMessageReplicationEntry(const FDOVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FDOVerbMessageReplication;

	// 实际承载的那条 FDOVerbMessage。
	UPROPERTY()
	FDOVerbMessage Message;
};

/** 用于复制的 verb 消息容器 — FastArraySerializer 形式，支持增量同步（只同步新增 / 变更 / 删除的条目）。 */
USTRUCT(BlueprintType)
struct FDOVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FDOVerbMessageReplication()
	{
	}

public:
	// 设置宿主：FastArray 复制时需要拿宿主才能取到 World 进而取本地的 UGameplayMessageSubsystem。
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients.
	// 服务端调用：把消息塞进 FastArray 容器（新增项会触发增量复制到客户端）。
	void AddMessage(const FDOVerbMessage& Message);

	//~FFastArraySerializer contract
	// FastArray 序列化契约函数，由 UE 在复制系统里回调。
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);   // 客户端即将删除这些项前调
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);       // 客户端收到新增后调
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);  // 客户端收到变更后调
	//~End of FFastArraySerializer contract

	// FastArray 序列化钩子：让 UE 知道如何对 CurrentMessages 做增量 delta 序列化。
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FDOVerbMessageReplicationEntry, FDOVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	// 客户端收到增量后调：把收到的条目再次通过本地 UGameplayMessageSubsystem 广播出去。
	void RebroadcastMessage(const FDOVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks.
	// 真正被 FastArray 同步的"消息队列"。每条目是一项 FDOVerbMessageReplicationEntry。
	UPROPERTY()
	TArray<FDOVerbMessageReplicationEntry> CurrentMessages;

	// Owner (for a route to a world).
	// 宿主：用来路由到 World，从而拿到客户端本地的 UGameplayMessageSubsystem。
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FDOVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FDOVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};