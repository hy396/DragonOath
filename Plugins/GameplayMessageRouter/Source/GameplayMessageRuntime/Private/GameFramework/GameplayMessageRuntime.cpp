// Copyright Epic Games, Inc. All Rights Reserved.

// GameplayMessageRuntime 模块入口（Runtime 模块）
// 本模块提供运行时可用的消息 Subsystem + 蓝图异步监听节点
// 没有自定义初始化逻辑，用引擎默认的模块实现即可
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, GameplayMessageRuntime)
