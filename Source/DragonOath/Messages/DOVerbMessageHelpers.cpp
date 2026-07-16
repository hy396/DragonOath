// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messages/DOVerbMessageHelpers.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffectTypes.h"
#include "Messages/DOVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOVerbMessageHelpers)

//////////////////////////////////////////////////////////////////////
// FDOVerbMessage
// （调试输出）

FString FDOVerbMessage::ToString() const
{
	// 用 UScriptStruct 自带的 ExportText 把整条 struct 拼成一段可读文本，
	// 启用了 GameplayMessageSubsystem.LogMessages CVar 时会打印出来。
	FString HumanReadableMessage;
	FDOVerbMessage::StaticStruct()->ExportText(/*out*/ HumanReadableMessage, this, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
	return HumanReadableMessage;
}

//////////////////////////////////////////////////////////////////////
// Helpers
// 从任意 UObject 反查对应的 PlayerState / PlayerController。
// 处理三种常见输入：APlayerController / APlayerState / APawn。

APlayerState* UDOVerbMessageHelpers::GetPlayerStateFromObject(UObject* Object)
{
	// 输入本身就是 APlayerController，直接取 PlayerState 字段。
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC->PlayerState;
	}

	// 输入本身就是 APlayerState，原样返回。
	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS;
	}

	// 输入是 APawn，通过它绑定的 Controller 间接到 PlayerState。
	if (APawn* TargetPawn = Cast<APawn>(Object))
	{
		if (APlayerState* TargetPS = TargetPawn->GetPlayerState())
		{
			return TargetPS;
		}
	}
	return nullptr;
}

APlayerController* UDOVerbMessageHelpers::GetPlayerControllerFromObject(UObject* Object)
{
	// 思路与上方对称：按 APlayerController → APlayerState → APawn 的顺序逐级反查。
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		// PlayerState 反查 PlayerController。
		return TargetPS->GetPlayerController();
	}

	if (APawn* TargetPawn = Cast<APawn>(Object))
	{
		// Pawn 通过 GetController() 拿 Controller，不一定能 Cast 到 PlayerController（AI/野怪会失败）。
		return Cast<APlayerController>(TargetPawn->GetController());
	}

	return nullptr;
}

// 把 FDOVerbMessage 转成 FGameplayCueParameters，用于把消息总线事件桥接到 GAS GameplayCue 系统。
// （GameplayCue 是 GAS 触发"轻量无伤害特效/音效"的机制，详见 GAS 文档。）
FGameplayCueParameters UDOVerbMessageHelpers::VerbMessageToCueParameters(const FDOVerbMessage& Message)
{
	FGameplayCueParameters Result;

	Result.OriginalTag           = Message.Verb;                         // 谓语 → GameplayCue 的 OriginalTag（用作 cue 标签）
	Result.Instigator            = Cast<AActor>(Message.Instigator);     // 主语 → GAS 里的 Instigator 字段
	Result.EffectCauser          = Cast<AActor>(Message.Target);         // 宾语 → GAS 里的 EffectCauser 字段
	Result.AggregatedSourceTags  = Message.InstigatorTags;              // 来源 tag
	Result.AggregatedTargetTags  = Message.TargetTags;                  // 目标 tag
	// ContextTags 当前未桥接到 GameplayCue，留作扩展点。
	Result.RawMagnitude          = Message.Magnitude;                   // 数值大小

	return Result;
}

// 上一个函数的反方向：把 FGameplayCueParameters 装回 FDOVerbMessage。
FDOVerbMessage UDOVerbMessageHelpers::CueParametersToVerbMessage(const FGameplayCueParameters& Params)
{
	FDOVerbMessage Result;

	Result.Verb           = Params.OriginalTag;
	Result.Instigator     = Params.Instigator.Get();                 // FGameplayCueParameters 内部存的是 TWeakObjectPtr<AActor>，用 .Get() 取
	Result.Target         = Params.EffectCauser.Get();
	Result.InstigatorTags = Params.AggregatedSourceTags;
	Result.TargetTags     = Params.AggregatedTargetTags;
	// ContextTags 当前无对应 GameplayCue 字段，留作扩展点。
	Result.Magnitude      = Params.RawMagnitude;

	return Result;
}