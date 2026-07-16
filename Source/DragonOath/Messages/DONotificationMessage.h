// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#include "DONotificationMessage.generated.h"

class UObject;

DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DO_AddNotification_Message);

class APlayerState;

/**
 * DragonOath 的"通知"消息：一条「转瞬即逝的 UI 提示」专用消息。
 *
 * 用途：击杀提示流 / 拾取提示流 / 成就达成等不需要长期保留的 UI 提示。
 * 区别于 FDOVerbMessage：FDONotificationMessage 携带 FText + TargetPlayer（要显示给谁），
 * 不靠 Verb tag 区分事件——靠 TargetChannel 区分（成就 / 击杀流 / 拾取流 各订各的）。
 *
 * 设计哲学：通用币（FDOVerbMessage）适合"事件结构同构"场景；UI 通知要带本地化文案与目标玩家，
 * 用单独 struct 更直接，避免把 FText / PlayerState 塞进通用币造成「鸡肋字段」。
 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDONotificationMessage
{
	GENERATED_BODY()

	// 目标频道：决定这条通知被哪种 UI 组件消费（成就 / 击杀流 / 拾取流... 各自订自己感兴趣的 channel）。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag TargetChannel;

	// 目标玩家：留空表示给本机所有本地玩家都显示。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<APlayerState> TargetPlayer = nullptr;

	// 给玩家看的具体文案（一般经本地化处理过的 FText）。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FText PayloadMessage;

	// 频道特定的额外负载：tag 形式，常用来在通知里挂一个"风格"或"资产 id"。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag PayloadTag;

	// 频道特定的额外负载：对象指针形式，常用于直接挂一个资产（图标 / 音效 / 数据行 row struct）。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<UObject> PayloadObject = nullptr;
};