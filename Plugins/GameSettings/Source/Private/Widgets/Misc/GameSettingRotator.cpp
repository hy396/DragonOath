// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "Widgets/Misc/GameSettingRotator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingRotator)

UGameSettingRotator::UGameSettingRotator(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

void UGameSettingRotator::SetDefaultOption(int32 DefaultOptionIndex)
{
	BP_OnDefaultOptionSpecified(DefaultOptionIndex);
}
