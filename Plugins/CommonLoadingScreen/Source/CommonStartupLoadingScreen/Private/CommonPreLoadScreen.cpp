// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#include "CommonPreLoadScreen.h"

#include "Misc/App.h"
#include "SCommonPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "CommonPreLoadingScreen"

void FCommonPreLoadScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		EngineLoadingWidget = SNew(SCommonPreLoadingScreenWidget);
	}
}

#undef LOCTEXT_NAMESPACE
