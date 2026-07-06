// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"

#include "Engine/LocalPlayer.h"

#define LOCTEXT_NAMESPACE "GameSetting"

TSharedRef<FWhenPlayingAsPrimaryPlayer> FWhenPlayingAsPrimaryPlayer::Get()
{
	static TSharedRef<FWhenPlayingAsPrimaryPlayer> Instance = MakeShared<FWhenPlayingAsPrimaryPlayer>();
	return Instance;
}

void FWhenPlayingAsPrimaryPlayer::GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
{
	if (!InLocalPlayer->IsPrimaryPlayer())
	{
		InOutEditState.Disable(LOCTEXT("OnlyPrimaryPlayerEditable", "Can only be changed by the primary player."));
	}
}

#undef LOCTEXT_NAMESPACE
