// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LoadingProcessInterface.generated.h"

/** 加载过程接口——实现此接口的对象可以声明"我正在加载，需要显示加载屏" */
UINTERFACE(BlueprintType)
class COMMONLOADINGSCREEN_API ULoadingProcessInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 加载过程接口（ILoadingProcessInterface）
 *
 * 任何需要阻止游戏显示的对象（如地图加载、资产流式加载等）
 * 都可以实现此接口，告知 LoadingScreenManager 当前需要显示加载屏。
 */
class COMMONLOADINGSCREEN_API ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	// 检查对象是否实现了此接口，如果是则询问是否应显示加载屏
	static bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason);

	/** 子类重写：返回 true 表示需要显示加载屏，OutReason 为原因描述 */
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const
	{
		return false;
	}
};
