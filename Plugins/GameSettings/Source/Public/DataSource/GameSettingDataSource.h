// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "Delegates/Delegate.h"

class ULocalPlayer;

//--------------------------------------
// FGameSettingDataSource
//--------------------------------------

/** 设置数据源基类——提供设置的读取/写入接口 */
class GAMESETTINGS_API FGameSettingDataSource : public TSharedFromThis<FGameSettingDataSource>
{
public:
	virtual ~FGameSettingDataSource() { }

	/**
	 * Some settings may take an async amount of time to finish initializing.  The settings system will wait
	 * for all settings to be ready before showing the setting.
	 */
	virtual void Startup(ULocalPlayer* InLocalPlayer, FSimpleDelegate StartupCompleteCallback) { StartupCompleteCallback.ExecuteIfBound(); }

	virtual bool Resolve(ULocalPlayer* InContext) = 0;

	virtual FString GetValueAsString(ULocalPlayer* InContext) const = 0;

	virtual void SetValue(ULocalPlayer* InContext, const FString& Value) = 0;

	virtual FString ToString() const = 0;
};
