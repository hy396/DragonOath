// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#include "Modules/ModuleManager.h"

// CommonLoadingScreen 模块入口（Runtime 模块）
// 本模块提供运行时加载屏管理器，负责在地图切换、资产加载等场景下
// 自动显示/隐藏加载屏，聚合所有 ILoadingProcessInterface 的加载需求
IMPLEMENT_MODULE(FDefaultModuleImpl, CommonLoadingScreen)
