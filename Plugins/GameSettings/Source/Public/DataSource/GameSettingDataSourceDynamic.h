// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingDataSource.h"
#include "PropertyPathHelpers.h"

class ULocalPlayer;

//--------------------------------------
// FGameSettingDataSourceDynamic
//--------------------------------------

/** 动态数据源——通过属性路径（如 "UserSettings.GameSettings.Volume"）动态读写设置值 */
class GAMESETTINGS_API FGameSettingDataSourceDynamic : public FGameSettingDataSource
{
public:
	FGameSettingDataSourceDynamic(const TArray<FString>& InDynamicPath);

	virtual bool Resolve(ULocalPlayer* InLocalPlayer) override;

	virtual FString GetValueAsString(ULocalPlayer* InLocalPlayer) const override;

	virtual void SetValue(ULocalPlayer* InLocalPlayer, const FString& Value) override;

	virtual FString ToString() const override;

private:
	FCachedPropertyPath DynamicPath;
};
