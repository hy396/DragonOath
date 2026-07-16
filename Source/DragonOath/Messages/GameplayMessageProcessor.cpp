// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messages/GameplayMessageProcessor.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayMessageProcessor)

void UGameplayMessageProcessor::BeginPlay()
{
	Super::BeginPlay();

	// 进入世界后开启监听 — 子类在 StartListening() 里实际注册订阅。
	StartListening();
}

void UGameplayMessageProcessor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 给子类一个主动反注册的机会（多数情况下靠下面的统一兜底即可）。
	StopListening();

	// Remove any listener handles.
	// 兜底：遍历所有 handle，统一调 UnregisterListener，防止 GC 后 lambda 还触发野回调。
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	for (FGameplayMessageListenerHandle& Handle : ListenerHandles)
	{
		MessageSubsystem.UnregisterListener(Handle);
	}
	ListenerHandles.Empty();
}

void UGameplayMessageProcessor::StartListening()
{
	// 默认空实现 — 子类按需重写，并在里面调 Subsystem->RegisterListener(...) 拿到 handle，
	// 再用 AddListenerHandle(...) 存到本组件的 ListenerHandles 数组里。
}

void UGameplayMessageProcessor::StopListening()
{
	// 默认空实现 — 子类可重写做"暂停监听"逻辑；本组件销毁走 EndPlay 兜底。
}

void UGameplayMessageProcessor::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
	// 把右值引用塞进数组，零拷贝。EndPlay 时统一遍历它来反注册。
	ListenerHandles.Add(MoveTemp(Handle));
}

double UGameplayMessageProcessor::GetServerTime() const
{
	// 从 GameState 拿权威服务器时间；无 GameState 时（极少）返回 0。
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		return GameState->GetServerWorldTimeSeconds();
	}
	else
	{
		return 0.0;
	}
}