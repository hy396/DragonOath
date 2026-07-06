// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：异步加载生命周期辅助，用来串联和取消异步加载请求，避免对象销毁后回调失效。

#include "Modules/ModuleManager.h"

// AsyncMixin 模块入口（Runtime 模块）
// 本模块提供 FAsyncMixin / FAsyncScope / FAsyncCondition，
// 用于简化异步加载请求的线性管理，自动处理对象销毁后的回调安全
class FAsyncMixinModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FAsyncMixinModule::StartupModule()
{
	// 本模块无需自定义初始化逻辑，用引擎默认的模块实现即可
}

void FAsyncMixinModule::ShutdownModule()
{
	// 本模块无需自定义清理逻辑
}

IMPLEMENT_MODULE(FAsyncMixinModule, AsyncMixin)
