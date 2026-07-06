// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Lyra 通用游戏 UI/玩家框架，包含 UI 层级管理、弹窗、LocalPlayer 和 PlayerController 基础设施。

#include "Modules/ModuleManager.h"

// CommonGame 模块入口（Runtime 模块）
// 本模块提供通用游戏 UI/玩家框架，包括：
// - GameUIManagerSubsystem / GameUIPolicy / PrimaryGameLayout：UI 层级管理
// - CommonGameInstance / CommonLocalPlayer / CommonPlayerController：游戏基础类
// - CommonUIExtensions：UI 工具函数
// - AsyncAction：异步 UI 操作（创建 Widget、推送层、确认对话框）
// - Messaging：消息/对话框子系统
class FCommonGameModule : public IModuleInterface
{
public:
	FCommonGameModule();
	// 模块启动
	virtual void StartupModule() override;
	// 模块关闭
	virtual void ShutdownModule() override;

private:

};


FCommonGameModule::FCommonGameModule()
{
}

void FCommonGameModule::StartupModule()
{
}

void FCommonGameModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FCommonGameModule, CommonGame);
