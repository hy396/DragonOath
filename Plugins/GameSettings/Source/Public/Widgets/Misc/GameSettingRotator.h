// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "CommonRotator.h"

#include "GameSettingRotator.generated.h"

class UObject;

/**
 *
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	UGameSettingRotator(const FObjectInitializer& Initializer);

	void SetDefaultOption(int32 DefaultOptionIndex);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = Events, meta = (DisplayName = "On Default Option Specified"))
	void BP_OnDefaultOptionSpecified(int32 DefaultOptionIndex);
};
