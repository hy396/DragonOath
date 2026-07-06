// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSetting.h"

#include "GameSettingValue.generated.h"

class UObject;

//--------------------------------------
// UGameSettingValue
//--------------------------------------

/**
 * 值型设置基类——所有可修改值的设置类型的基类
 *
 * 提供 StoreInitial / ResetToDefault / RestoreToInitial 等值管理接口，
 * 支持将设置值恢复到默认值或打开设置面板时的初始值。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingValue : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingValue();

	/** 存储初始值，在初始化和应用时调用 */
	virtual void StoreInitial() PURE_VIRTUAL(, );

	/** 重置为默认值 */
	virtual void ResetToDefault() PURE_VIRTUAL(, );

	/** 恢复到初始值（打开设置面板前的值） */
	virtual void RestoreToInitial() PURE_VIRTUAL(, );

protected:
	virtual void OnInitialized() override;
};
