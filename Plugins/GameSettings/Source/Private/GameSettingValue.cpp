// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "GameSettingValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingValue)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingValue
//--------------------------------------

UGameSettingValue::UGameSettingValue()
{
	// Values will report to analytics.
	bReportAnalytics = true;
}

void UGameSettingValue::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DescriptionRichText.IsEmpty() || DynamicDetails.IsBound(), TEXT("You must provide a description or it must specify a dynamic details function for settings with values."));
#endif

	StoreInitial();
}

#undef LOCTEXT_NAMESPACE
