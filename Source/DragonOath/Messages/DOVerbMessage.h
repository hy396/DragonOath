// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "DOVerbMessage.generated.h"

// DragonOath 的「通用币」：用一条 USTRUCT 表达「主语 Instigator 做了 谓语 Verb 对 宾语 Target」的事件。
//
//   - 字段是 USTRUCT(BlueprintType)，蓝图端能用 "Listen for Gameplay Messages" 节点直接订阅。
//   - 不同事件类型（伤害 / 死亡 / 重置 / 助攻 / 连击 …）共用这一条 struct，靠 Verb 这个 GameplayTag 区分。
//   - 这是 Lyra `FLyraVerbMessage` 的 DragonOath 移植版，命名风格按项目约定（FDO 前缀）。
//   - 详细设计背景与迁移文档：Docs/04_Local_Message_Bus.md。
USTRUCT(BlueprintType)
struct FDOVerbMessage
{
	GENERATED_BODY()

	// 谓语 / 事件类型：用 FGameplayTag 同时充当"事件名"与"订阅频道"。
	// 订阅方能用 ExactMatch 或 PartialMatch 精确 / 模糊匹配。
	// 例如：Message.Combat.Damage.Applied / Message.Combat.Elimination.Fired。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Verb;

	// 主语 / 来源：事件的发起者。一般是 APawn / APlayerState / AActor 之一。
	// 网络回调里常常是裸 AActor，要拿 PlayerState 需要用 UDOVerbMessageHelpers::GetPlayerStateFromObject() 反查。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Instigator = nullptr;

	// 宾语 / 目标：事件作用的对象。同上，网络环境下需要走 Helpers 反查。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Target = nullptr;

	// 主语身上携带的 tag：通常从 GAS GameplayEffectContext 的 CapturedSourceTags 取来。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer InstigatorTags;

	// 宾语身上携带的 tag：通常从 GAS GameplayEffectContext 的 CapturedTargetTags 取来。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer TargetTags;

	// 上下文 / 状语：扩展位。
	// 常用场景：是否团队击杀 / 是否爆头 / 武器类型 / 击杀人所在的阵营...
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer ContextTags;

	// 补语 / 数值大小：根据事件类型含义不同。伤害事件时是伤害值，助攻事件时是助攻伤害贡献值。
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	double Magnitude = 1.0;

	// 返回人类可读的调试字符串；底层用 UScriptStruct::ExportText 把整个 struct 文本化。
	// 实现放在 DOVerbMessageHelpers.cpp（与 Lyra 一致）。
	DRAGONOATH_API FString ToString() const;
};