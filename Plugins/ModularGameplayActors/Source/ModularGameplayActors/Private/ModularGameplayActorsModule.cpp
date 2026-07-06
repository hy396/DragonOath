// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：模块化 Actor 基类插件，用于支持 ModularGameplay/GameFeature 向核心 Actor 注入组件。

#include "Modules/ModuleManager.h"

// ModularGameplayActors 模块入口（Runtime 模块）
// 本模块提供模块化版本的 GameMode/GameState/PlayerController/PlayerState/Pawn/Character/AIController，
// 在关键生命周期节点注册/注销 UGameFrameworkComponentManager 接收器，
// 使得 GameFeature 插件可以向这些 Actor 动态注入组件
IMPLEMENT_MODULE(FDefaultModuleImpl, ModularGameplayActors);
