// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingFilterState.h"

class ULocalPlayer;


/** 编辑条件：仅主玩家可编辑（本地多人时限制非主玩家） */
class GAMESETTINGS_API FWhenPlayingAsPrimaryPlayer : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlayingAsPrimaryPlayer> Get();

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
};
