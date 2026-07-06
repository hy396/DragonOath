// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "DataSource/GameSettingDataSourceDynamic.h"

#include "Engine/LocalPlayer.h"

//--------------------------------------
// FGameSettingDataSourceDynamic
//--------------------------------------

FGameSettingDataSourceDynamic::FGameSettingDataSourceDynamic(const TArray<FString>& InDynamicPath)
	: DynamicPath(InDynamicPath)
{
}

bool FGameSettingDataSourceDynamic::Resolve(ULocalPlayer* InLocalPlayer)
{
	return DynamicPath.Resolve(InLocalPlayer);
}

FString FGameSettingDataSourceDynamic::GetValueAsString(ULocalPlayer* InLocalPlayer) const
{
	FString OutStringValue;

	const bool bSuccess = PropertyPathHelpers::GetPropertyValueAsString(InLocalPlayer, DynamicPath, OutStringValue);
	ensure(bSuccess);

	return OutStringValue;
}

void FGameSettingDataSourceDynamic::SetValue(ULocalPlayer* InLocalPlayer, const FString& InStringValue)
{
	const bool bSuccess = PropertyPathHelpers::SetPropertyValueFromString(InLocalPlayer, DynamicPath, InStringValue);
	ensure(bSuccess);
}

FString FGameSettingDataSourceDynamic::ToString() const
{
	return DynamicPath.ToString();
}
