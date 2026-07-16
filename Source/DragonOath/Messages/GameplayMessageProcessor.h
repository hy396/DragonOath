// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "GameplayMessageProcessor.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UObject;

/**
 * DragonOath 的消息处理器基类（与 Lyra 同名）。
 *
 * 用途：观察游戏消息总线上的事件，在合适的时机组合 / 转换后再次广播；
 *       比如连击（chain）或连杀（combo）的检测逻辑就常继承这个类。
 *
 * 基类名沿用 `UGameplayMessageProcessor` —— 与 GameplayMessageRouter 插件生态一致，
 * 引擎内 / 第三方示例代码 / 未来引擎升级都可以直接复用。
 *
 * 这些处理器通常在服务器上 spawn **一次**（每个世界一份，而不是每个玩家一份）。
 * 当事件只对部分玩家有意义时，要在处理器内部自行过滤（按 PlayerState 等）。
 */
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class DRAGONOATH_API UGameplayMessageProcessor : public UActorComponent
{
	GENERATED_BODY()

public:
	//~UActorComponent interface（重写 UActorComponent 生命周期）
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	// 开始监听：子类重写此函数，把想订阅的 channel + 回调注册到 UGameplayMessageSubsystem。
	virtual void StartListening();

	// 停止监听：StartListening 的对称操作；默认实现是空的，子类按需实现。
	virtual void StopListening();

protected:
	// 把注册成功的 handle 存起来，统一在 EndPlay 时取消注册，避免野回调。
	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);

	// 取 GameState 的服务器时间，便于做时间窗判定（如"3 秒内 2 杀算连击"）。
	double GetServerTime() const;

private:
	// 持有所有订阅句柄，EndPlay 时遍历它来 UnregisterListener。
	TArray<FGameplayMessageListenerHandle> ListenerHandles;
};