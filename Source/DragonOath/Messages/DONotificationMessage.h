// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#include "DONotificationMessage.generated.h"

class UObject;

DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DO_AddNotification_Message);

class APlayerState;

/**
 * DragonOath 的 UI 通知消息：用于「一次性 UI 提示」的专用结构体。
 *
 * 典型用途：击杀提示流 / 拾取提示流 / 成就达成等无需长期保留的 UI 提示。
 * 与 FDOVerbMessage 的区别：本结构携带 FText 文案 + TargetPlayer（要显示给谁），
 * 不靠 Verb tag 区分事件，而是靠 TargetChannel 区分（成就 / 击杀流 / 拾取流 各自订阅自己的 channel）。
 *
 * 设计取舍：FDOVerbMessage 适合「事件结构同构」的通用场景；UI 通知需要本地化文案与目标玩家，
 * 用独立结构更直接，避免把 FText / PlayerState 硬塞进通用事件结构变成冗余字段。
 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDONotificationMessage
{
	GENERATED_BODY()

	// 目标频道：决定由哪种 UI 组件消费（成就 / 击杀流 / 拾取流…各订各的）。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag TargetChannel;

	// 目标玩家：留空表示显示给本机所有本地玩家。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<APlayerState> TargetPlayer = nullptr;

	// 给玩家看的具体文案（一般经本地化处理的 FText）。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FText PayloadMessage;

	// 频道相关的额外负载（tag 形式），常用于挂「风格」或「资产 id」。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag PayloadTag;

	// 频道相关的额外负载（对象指针），常用于直接挂资产（图标 / 音效 / DataTable 行等）。
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<UObject> PayloadObject = nullptr;
};
