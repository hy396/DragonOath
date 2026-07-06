// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#include "LoadingProcessTask.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "LoadingScreenManager.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingProcessTask)

/*static*/ ULoadingProcessTask* ULoadingProcessTask::CreateLoadingScreenProcessTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	ULoadingScreenManager* LoadingScreenManager = GameInstance ? GameInstance->GetSubsystem<ULoadingScreenManager>() : nullptr;

	if (LoadingScreenManager)
	{
		ULoadingProcessTask* NewLoadingTask = NewObject<ULoadingProcessTask>(LoadingScreenManager);
		NewLoadingTask->SetShowLoadingScreenReason(ShowLoadingScreenReason);

		LoadingScreenManager->RegisterLoadingProcessor(NewLoadingTask);

		return NewLoadingTask;
	}

	return nullptr;
}

void ULoadingProcessTask::Unregister()
{
	ULoadingScreenManager* LoadingScreenManager = Cast<ULoadingScreenManager>(GetOuter());
	LoadingScreenManager->UnregisterLoadingProcessor(this);
}

void ULoadingProcessTask::SetShowLoadingScreenReason(const FString& InReason)
{
	Reason = InReason;
}

bool ULoadingProcessTask::ShouldShowLoadingScreen(FString& OutReason) const
{
	OutReason = Reason;
	return true;
}
