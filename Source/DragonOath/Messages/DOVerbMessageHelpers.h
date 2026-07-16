// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "DOVerbMessageHelpers.generated.h"

struct FGameplayCueParameters;
struct FDOVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;

/**
 * FDOVerbMessage 的辅助函数库：
 *  - 把 GameplayCue 的参数 / UObject 上下文互转成 FDOVerbMessage；
 *  - 提供"任意 UObject → APlayerState / APlayerController"这种网络安全的反查。
 */
UCLASS()
class DRAGONOATH_API UDOVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 给一个任意 UObject（APawn / APlayerController / APlayerState...），反查对应的 APlayerState。
	// 找不到时返回 nullptr。常用：在订阅 FDOVerbMessage 时把消息里的 Instigator / Target 解析为 PlayerState。
	UFUNCTION(BlueprintCallable, Category = "DO|Messages")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	// 同上，反查 APlayerController。
	UFUNCTION(BlueprintCallable, Category = "DO|Messages")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	// 把 FDOVerbMessage 转成 GAS 的 FGameplayCueParameters，便于把消息总线上的事件桥接到 GameplayCue 系统。
	//   - Verb → OriginalTag
	//   - Instigator / Target → Instigator / EffectCauser
	//   - InstigatorTags / TargetTags → AggregatedSourceTags / AggregatedTargetTags
	//   - Magnitude → RawMagnitude
	//   - ContextTags 当前未桥接（Lyra 原版即留 //@TODO 占位，留作扩展点）。
	UFUNCTION(BlueprintCallable, Category = "DO|Messages")
	static FGameplayCueParameters VerbMessageToCueParameters(const FDOVerbMessage& Message);

	// 上一个函数的反方向：把 FGameplayCueParameters 装回 FDOVerbMessage，用于反向接入消息总线。
	UFUNCTION(BlueprintCallable, Category = "DO|Messages")
	static FDOVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};