// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "GameSettingValueScalar.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingValueScalar)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingValueScalar
//--------------------------------------

UGameSettingValueScalar::UGameSettingValueScalar()
{

}

void UGameSettingValueScalar::SetValueNormalized(double NormalizedValue)
{
	SetValue(FMath::GetMappedRangeValueClamped(TRange<double>(0, 1), GetSourceRange(), NormalizedValue));
}

double UGameSettingValueScalar::GetValueNormalized() const
{
	return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), GetValue());
}

#undef LOCTEXT_NAMESPACE
