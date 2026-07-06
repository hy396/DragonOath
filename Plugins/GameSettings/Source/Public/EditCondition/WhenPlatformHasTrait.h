// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

class ULocalPlayer;

//////////////////////////////////////////////////////////////////////
// FWhenPlatformHasTrait

/**
 * 平台特征编辑条件——检查 CommonUI 的平台特征标签
 *
 * KillIfMissing：平台缺少标签时隐藏设置
 * DisableIfMissing：平台缺少标签时禁用设置
 * KillIfPresent / DisableIfPresent：反向逻辑
 */
class GAMESETTINGS_API FWhenPlatformHasTrait : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlatformHasTrait> KillIfMissing(FGameplayTag InVisibilityTag, const FString& InKillReason);
	static TSharedRef<FWhenPlatformHasTrait> DisableIfMissing(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	static TSharedRef<FWhenPlatformHasTrait> KillIfPresent(FGameplayTag InVisibilityTag, const FString& InKillReason);
	static TSharedRef<FWhenPlatformHasTrait> DisableIfPresent(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	//~FGameSettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
	//~End of FGameSettingEditCondition interface

private:
	FGameplayTag VisibilityTag;
	bool bTagDesired;
	FString KillReason;
	FText DisableReason;
};
