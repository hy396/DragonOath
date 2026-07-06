// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayMessageSubsystem)

DEFINE_LOG_CATEGORY(LogGameplayMessageSubsystem);

namespace UE
{
	namespace GameplayMessageSubsystem
	{
		// 控制台变量：是否打印每条广播消息的日志
		// 用法：控制台输入 `GameplayMessageSubsystem.LogMessages 1` 打开
		static int32 ShouldLogMessages = 0;
		static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("GameplayMessageSubsystem.LogMessages"),
			ShouldLogMessages,
			TEXT("Should messages broadcast through the gameplay message subsystem be logged?"));
	}
}
//////////////////////////////////////////////////////////////////////
// FGameplayMessageListenerHandle

// 反注册监听器并清空句柄（再次调用安全——Subsystem 弱引用失效后直接跳过）
void FGameplayMessageListenerHandle::Unregister()
{
	if (UGameplayMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// UGameplayMessageSubsystem

// 快捷获取函数：通过 World 上下文拿到对应 GameInstance 上的消息路由
// 断言确保返回值非空；若不确定上下文是否有效，改用 HasInstance() 先判断
UGameplayMessageSubsystem& UGameplayMessageSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

// 安全版：World 可能为空（如编辑器预览世界），返回是否存在可用 Subsystem
bool UGameplayMessageSubsystem::HasInstance(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UGameplayMessageSubsystem* Router = World != nullptr ? UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance()) : nullptr;
	return Router != nullptr;
}

// Subsystem 销毁时清理全部监听器（GameInstance 关闭 / PIE 结束时触发）
void UGameplayMessageSubsystem::Deinitialize()
{
	ListenerMap.Reset();

	Super::Deinitialize();
}

// 广播消息的核心实现
//
// 关键机制：沿 Tag 层级向父节点遍历，让 PartialMatch 监听器也能收到
// 例如广播 "A.B.C" 时：
//   第 1 轮 Tag = "A.B.C"  → 触发所有监听 "A.B.C" 的监听器（无视 MatchType）
//   第 2 轮 Tag = "A.B"    → 触发监听 "A.B" 且 MatchType == PartialMatch 的监听器
//   第 3 轮 Tag = "A"      → 触发监听 "A"   且 MatchType == PartialMatch 的监听器
void UGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes)
{
	// 如果打开了 LogMessages CVar，把消息内容打到 Log
	if (UE::GameplayMessageSubsystem::ShouldLogMessages != 0)
	{
		FString* pContextString = nullptr;
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			pContextString = &GPlayInEditorContextString;
		}
#endif

		FString HumanReadableMessage;
		StructType->ExportText(/*out*/ HumanReadableMessage, MessageBytes, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
		UE_LOG(LogGameplayMessageSubsystem, Log, TEXT("BroadcastMessage(%s, %s, %s)"), pContextString ? **pContextString : *GetPathNameSafe(this), *Channel.ToString(), *HumanReadableMessage);
	}

	// 沿 Tag 层级广播
	// bOnInitialTag：首轮（最具体的 Tag）无视 MatchType，否则只触发 PartialMatch
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			// 复制一份列表，避免回调里反注册导致迭代器失效
			TArray<FGameplayMessageListenerData> ListenerArray(pList->Listeners);

			for (const FGameplayMessageListenerData& Listener : ListenerArray)
			{
				if (bOnInitialTag || (Listener.MatchType == EGameplayMessageMatch::PartialMatch))
				{
					// 监听器期望的类型已被 GC → 失效，自动清理
					if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("Listener struct type has gone invalid on Channel %s. Removing listener from list"), *Channel.ToString());
						UnregisterListenerInternal(Channel, Listener.HandleID);
						continue;
					}

					// 类型兼容检查：发送方的 StructType 必须是监听方期望类型的子类
					// （或监听方没指定类型 = 通配，通常内部用）
					if (!Listener.bHadValidType || StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						Listener.ReceivedCallback(Channel, StructType, MessageBytes);
					}
					else
					{
						UE_LOG(LogGameplayMessageSubsystem, Error, TEXT("Struct type mismatch on channel %s (broadcast type %s, listener at %s was expecting type %s)"),
							*Channel.ToString(),
							*StructType->GetPathName(),
							*Tag.ToString(),
							*Listener.ListenerStructType->GetPathName());
					}
				}
			}
		}
		bOnInitialTag = false;
	}
}

// 蓝图版本的广播入口——不会真的执行，execK2_BroadcastMessage 才是实际路径
void UGameplayMessageSubsystem::K2_BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	// This will never be called, the exec version below will be hit instead
	checkNoEntry();
}

// 蓝图调用广播的实际 thunk
// 用 CustomThunk + StepCompiledIn 手动取出通配符 Message 参数的类型和地址
DEFINE_FUNCTION(UGameplayMessageSubsystem::execK2_BroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	// 取出 Message 参数的类型和内存地址（通配符 USTRUCT）
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
	}
}

// 注册监听器的核心实现
// 为该频道创建/查找监听器列表，递增分配 HandleID，返回句柄
FGameplayMessageListenerHandle UGameplayMessageSubsystem::RegisterListenerInternal(FGameplayTag Channel, TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback, const UScriptStruct* StructType, EGameplayMessageMatch MatchType)
{
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	FGameplayMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = StructType != nullptr;
	Entry.HandleID = ++List.HandleID;
	Entry.MatchType = MatchType;

	return FGameplayMessageListenerHandle(this, Channel, Entry.HandleID);
}

// 反注册的对外入口——会校验句柄绑定的 Subsystem 确实是自己
void UGameplayMessageSubsystem::UnregisterListener(FGameplayMessageListenerHandle Handle)
{
	if (Handle.IsValid())
	{
		check(Handle.Subsystem == this);

		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

// 反注册的内部实现——按 HandleID 从该频道监听器数组中移除
// 空列表会连频道一起删（保持 ListenerMap 紧凑）
void UGameplayMessageSubsystem::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	if (FChannelListenerList* pList = ListenerMap.Find(Channel))
	{
		int32 MatchIndex = pList->Listeners.IndexOfByPredicate([ID = HandleID](const FGameplayMessageListenerData& Other) { return Other.HandleID == ID; });
		if (MatchIndex != INDEX_NONE)
		{
			pList->Listeners.RemoveAtSwap(MatchIndex);
		}

		if (pList->Listeners.Num() == 0)
		{
			ListenerMap.Remove(Channel);
		}
	}
}
