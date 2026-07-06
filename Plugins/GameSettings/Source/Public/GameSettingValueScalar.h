// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingValue.h"
#include "Math/Range.h"

#include "GameSettingValueScalar.generated.h"

class UObject;

/**
 * 标量值设置基类——连续数值范围设置（如滑块）
 *
 * 提供 SetValueNormalized / GetValueNormalized 等归一化接口，
 * 子类需实现 GetSourceRange / GetValue / GetDefaultValue 等纯虚函数。
 */
/**
 * 标量值设置基类，用于连续数值型设置（如滑块）。
 * 支持归一化值、源范围、步进和格式化显示。
 */
UCLASS(abstract)
class GAMESETTINGS_API UGameSettingValueScalar : public UGameSettingValue
{
	GENERATED_BODY()

public:
	UGameSettingValueScalar();

	/** 设置归一化值（0~1） */
	void SetValueNormalized(double NormalizedValue);
	/** 获取归一化值（0~1） */
	double GetValueNormalized() const;

	/** 获取默认归一化值 */
	TOptional<double> GetDefaultValueNormalized() const
	{
		TOptional<double> DefaultValue = GetDefaultValue();
		if (DefaultValue.IsSet())
		{
			return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), DefaultValue.GetValue());
		}
		return TOptional<double>();
	}

	/** 获取默认值 */
	virtual TOptional<double> GetDefaultValue() const						PURE_VIRTUAL(, return TOptional<double>(););
	/** 设置值 */
	virtual void SetValue(double Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change)	PURE_VIRTUAL(, );
	/** 获取当前值 */
	virtual double GetValue() const											PURE_VIRTUAL(, return 0;);
	/** 获取源范围 */
	virtual TRange<double> GetSourceRange() const							PURE_VIRTUAL(, return TRange<double>(););
	/** 获取步进值 */
	virtual double GetSourceStep() const									PURE_VIRTUAL(, return 0.01;);
	/** 获取归一化步进大小 */
	double GetNormalizedStepSize() const
	{
		TRange<double> SourceRange = GetSourceRange();
		return GetSourceStep() / FMath::Abs(SourceRange.GetUpperBoundValue() - SourceRange.GetLowerBoundValue());
	}
	/** 获取格式化文本 */
	virtual FText GetFormattedText() const									PURE_VIRTUAL(, return FText::GetEmpty(););

	/** 获取分析用值 */
	virtual FString GetAnalyticsValue() const override
	{
		return LexToString(GetValue());
	}

protected:
};
