// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"

#include "IGameSettingActionInterface.generated.h"

class UGameSetting;
class UObject;
struct FFrame;

UINTERFACE(MinimalAPI, meta = (BlueprintType))
class UGameSettingActionInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class GAMESETTINGS_API IGameSettingActionInterface
{
	GENERATED_BODY()

public:
	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ExecuteActionForSetting(FGameplayTag ActionTag, UGameSetting* InSetting);
};
