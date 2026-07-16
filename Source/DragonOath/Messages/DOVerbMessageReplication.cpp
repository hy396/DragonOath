// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messages/DOVerbMessageReplication.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/DOVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOVerbMessageReplication)

//////////////////////////////////////////////////////////////////////
// FDOVerbMessageReplicationEntry（单条消息项：调试输出）

FString FDOVerbMessageReplicationEntry::GetDebugString() const
{
	// 复用 FDOVerbMessage::ToString 输出人类可读文本。
	return Message.ToString();
}

//////////////////////////////////////////////////////////////////////
// FDOVerbMessageReplication（FastArray 复制 + 客户端再广播）

// 服务端调用：把消息塞进 FastArray，并标记该项 dirty，触发 UE 的增量复制流程。
void FDOVerbMessageReplication::AddMessage(const FDOVerbMessage& Message)
{
	FDOVerbMessageReplicationEntry& NewStack = CurrentMessages.Emplace_GetRef(Message);
	MarkItemDirty(NewStack);  // FastArraySerializerItem 自带接口，通知复制系统本项有变更
}

// 客户端即将删除若干项之前的回调。这里留空是允许的——如果将来要做"按 tag 引用计数"之类可以在这里做清理。
void FDOVerbMessageReplication::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
// 	for (int32 Index : RemovedIndices)
// 	{
// 		const FGameplayTag Tag = CurrentMessages[Index].Tag;
// 		TagToCountMap.Remove(Tag);
// 	}
}

// 客户端收到新增项之后调：遍历新增项，每一条都重新广播到客户端本地的 UGameplayMessageSubsystem。
//   - 这条路径让"事件记录型"消息能在所有客户端触发普通订阅方（UI / 处理器），
//     与玩家 PlayerState 上的 Client RPC 路径相互独立，但最终落到同一个本地总线。
void FDOVerbMessageReplication::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FDOVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

// 客户端收到变更项之后调：和 PostReplicatedAdd 同语义（同样再次广播），FastArray 用变更事件区分"修改过的项"。
void FDOVerbMessageReplication::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FDOVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

// 客户端的"再广播"：把消息灌进本地总线，订阅方（UMG / 处理基类）可以像服务端的本地消息一样消费。
//   - 与 PlayerState 上的 ClientBroadcastMessage RPC 是两条独立的"穿网"路径，
//     目的不同：FastArray 适合"事件记录型"，RPC 适合"轻量通知"。
void FDOVerbMessageReplication::RebroadcastMessage(const FDOVerbMessage& Message)
{
	check(Owner);   // Owner 必须在 SetOwner 时已经设好；为 null 一般意味着忘记调用 SetOwner。
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner);
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}