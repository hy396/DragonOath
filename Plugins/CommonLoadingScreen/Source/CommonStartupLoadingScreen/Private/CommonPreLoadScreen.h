// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#pragma once

#include "PreLoadScreenBase.h"

class SWidget;

class FCommonPreLoadScreen : public FPreLoadScreenBase
{
public:

    /*** IPreLoadScreen Implementation ***/
	virtual void Init() override;
    virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
    virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }
private:

    TSharedPtr<SWidget> EngineLoadingWidget;
};
