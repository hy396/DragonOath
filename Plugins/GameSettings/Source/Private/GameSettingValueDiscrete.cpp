// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "GameSettingValueDiscrete.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingValueDiscrete)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingValueDiscrete
//--------------------------------------

UGameSettingValueDiscrete::UGameSettingValueDiscrete()
{

}

FString UGameSettingValueDiscrete::GetAnalyticsValue() const
{
	const TArray<FText> Options = GetDiscreteOptions();
	const int32 CurrentOptionIndex = GetDiscreteOptionIndex();
	if (Options.IsValidIndex(CurrentOptionIndex))
	{
		const FString* SourceString = FTextInspector::GetSourceString(Options[CurrentOptionIndex]);
		if (SourceString)
		{
			return *SourceString;
		}
	}

	return TEXT("<Unknown Index>");
}

#undef LOCTEXT_NAMESPACE
