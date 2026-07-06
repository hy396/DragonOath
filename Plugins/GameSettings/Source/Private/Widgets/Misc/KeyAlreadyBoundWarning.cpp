// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "Widgets/Misc/KeyAlreadyBoundWarning.h"
#include "Components/TextBlock.h"

void UKeyAlreadyBoundWarning::SetWarningText(const FText& InText)
{
	WarningText->SetText(InText);
}

void UKeyAlreadyBoundWarning::SetCancelText(const FText& InText)
{
	CancelText->SetText(InText);
}
