// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：用户、平台和在线会话辅助插件，后续登录、房间、匹配阶段再深入接入。

#include "CommonUserModule.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FCommonUserModule"

// CommonUser 模块入口（Runtime 模块）
// 本模块提供用户登录、权限检查、在线会话（创建/搜索/加入/主持）等在线功能，
// 同时支持 OSSv1 和 OSSv2 两套在线子系统接口

void FCommonUserModule::StartupModule()
{
	// 本模块无需自定义初始化逻辑，用引擎默认的模块实现即可
}

void FCommonUserModule::ShutdownModule()
{
	// 本模块无需自定义清理逻辑
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCommonUserModule, CommonUser)
