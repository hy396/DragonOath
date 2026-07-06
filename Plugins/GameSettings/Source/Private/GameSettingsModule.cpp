// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "Modules/ModuleManager.h"

// GameSettings 模块入口（Runtime 模块）
// 本模块提供通用设置界面框架，包括：
// - UGameSetting / UGameSettingValue / UGameSettingAction：设置数据模型
// - UGameSettingCollection / UGameSettingRegistry：设置集合与注册表
// - FGameSettingDataSource：动态数据源（CVar/属性绑定）
// - FGameSettingEditCondition：编辑条件（平台特征/主玩家限制等）
// - 设置 UI Widget：面板/列表/详情/滑块/旋转器等
class FGameSettingsModule : public IModuleInterface
{
public:
	FGameSettingsModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

};


FGameSettingsModule::FGameSettingsModule()
{
}

void FGameSettingsModule::StartupModule()
{
}

void FGameSettingsModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FGameSettingsModule, GameSettings);
