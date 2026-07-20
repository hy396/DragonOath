// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "DOVerbMessage.generated.h"

// DragonOath 通用事件消息结构体（Lyra FLyraVerbMessage 的移植版，命名按项目 FDO 前缀）。
// 用一条 USTRUCT 表达「Instigator 对 Target 执行了 Verb 类型事件」，不同事件（伤害 / 击杀 / 助攻…）
// 共用该结构，靠 Verb 这个 GameplayTag 区分。蓝图可用 "Listen for Gameplay Messages" 节点直接订阅。
// 设计背景见 Docs/04_Local_Message_Bus.md。
USTRUCT(BlueprintType)
struct FDOVerbMessage
{
	GENERATED_BODY()

	// 事件类型标签，同时充当订阅频道。订阅方可 ExactMatch / PartialMatch 精确或模糊匹配，
	// 例如 Message.Combat.Damage.Applied / Message.Combat.Elimination.Fired。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Verb;

	// 事件发起者（APawn / APlayerState / AActor 等）。网络回调中常为裸 AActor，
	// 需经 UDOVerbMessageHelpers::GetPlayerStateFromObject() 反查 PlayerState。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Instigator = nullptr;

	// 事件作用对象。同上，网络环境下需经 Helpers 反查。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Target = nullptr;

	// 发起者携带的 tag，通常取自 GAS GameplayEffectContext 的 CapturedSourceTags。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer InstigatorTags;

	// 目标携带的 tag，取自 GAS GameplayEffectContext 的 CapturedTargetTags。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer TargetTags;

	// 扩展位，用于附带额外上下文，如是否团队击杀 / 爆头 / 武器类型 / 阵营等。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer ContextTags;

	// 事件相关数值，含义随事件类型而定：伤害事件为伤害值，助攻事件为助攻伤害贡献值。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	double Magnitude = 1.0;

	// 调试用，将整个 struct 文本化（底层用 UScriptStruct::ExportText）。实现见 DOVerbMessageHelpers.cpp。
	DRAGONOATH_API FString ToString() const;
};
