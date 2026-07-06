// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingFilterState.h"

/** 内联编辑条件——通过 Lambda 定义编辑状态逻辑 */
class FWhenCondition : public FGameSettingEditCondition
{
public:
	FWhenCondition(TFunction<void(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState&)>&& InInlineEditCondition)
		: InlineEditCondition(InInlineEditCondition)
	{
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		InlineEditCondition(InLocalPlayer, InOutEditState);
	}

	virtual FString ToString() const override
	{
		return TEXT("Inline Edit Condition");
	}

private:
	TFunction<void(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState)> InlineEditCondition;
};
