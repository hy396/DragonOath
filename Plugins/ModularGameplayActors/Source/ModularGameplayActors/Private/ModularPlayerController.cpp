// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#include "ModularPlayerController.h"

#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularPlayerController)

void AModularPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

// PlayerController 收到 Player 后，发送 GameActorReady 事件并转发给所有 ControllerComponent
void AModularPlayerController::ReceivedPlayer()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::ReceivedPlayer();

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->ReceivedPlayer();
	}
}

// 每帧转发给所有 ControllerComponent
void AModularPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->PlayerTick(DeltaTime);
	}
}
