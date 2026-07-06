// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "Widgets/GameSettingDetailExtension.h"

#include "GameSetting.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingDetailExtension)

void UGameSettingDetailExtension::SetSetting(UGameSetting* InSetting)
{
	NativeSettingAssigned(InSetting);
}

void UGameSettingDetailExtension::NativeSettingAssigned(UGameSetting* InSetting)
{
	if (Setting)
	{
		Setting->OnSettingChangedEvent.RemoveAll(this);
	}

	Setting = InSetting;
	Setting->OnSettingChangedEvent.AddUObject(this, &ThisClass::NativeSettingValueChanged);

	OnSettingAssigned(InSetting);
}

void UGameSettingDetailExtension::NativeSettingValueChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason)
{
	OnSettingValueChanged(InSetting);
}
