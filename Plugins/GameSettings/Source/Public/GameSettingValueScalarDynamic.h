// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingValueScalar.h"

#include "GameSettingValueScalarDynamic.generated.h"

struct FNumberFormattingOptions;

class FGameSettingDataSource;
class UObject;

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueScalarDynamic
//////////////////////////////////////////////////////////////////////////

typedef TFunction<FText(double SourceValue, double NormalizedValue)> FSettingScalarFormatFunction;

/**
 * 动态标量值设置——运行时从数据源读取的连续数值设置
 *
 * 预置了常用格式化函数：Raw / RawOneDecimal / ZeroToOnePercent 等，
 * 可通过 Setter/Getter 函数绑定到任意 CVar 或属性。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueScalarDynamic : public UGameSettingValueScalar
{
	GENERATED_BODY()

public:
	static FSettingScalarFormatFunction Raw;  // 原始值格式化
	static FSettingScalarFormatFunction RawOneDecimal;  // 一位小数格式化
	static FSettingScalarFormatFunction RawTwoDecimals;  // 两位小数格式化
	static FSettingScalarFormatFunction ZeroToOnePercent;  // 0~1 百分比格式化
	static FSettingScalarFormatFunction ZeroToOnePercent_OneDecimal;  // 0~1 百分比（一位小数）
	static FSettingScalarFormatFunction SourceAsPercent1;  // 源值百分比（0~1 范围）
	static FSettingScalarFormatFunction SourceAsPercent100;  // 源值百分比（0~100 范围）
	static FSettingScalarFormatFunction SourceAsInteger;  // 源值整数格式化
private:
	static const FNumberFormattingOptions& GetOneDecimalFormattingOptions();

public:
	UGameSettingValueScalarDynamic();

	/** UGameSettingValue */
	virtual void Startup() override;
	virtual void StoreInitial() override;  // 存储初始值
	virtual void ResetToDefault() override;  // 重置为默认值
	virtual void RestoreToInitial() override;  // 恢复为初始值

	/** UGameSettingValueScalar */
	virtual TOptional<double> GetDefaultValue() const override;  // 获取默认值
	virtual void SetValue(double Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change) override;  // 设置值
	virtual double GetValue() const override;  // 获取当前值
	virtual TRange<double> GetSourceRange() const override;  // 获取源范围
	virtual double GetSourceStep() const override;  // 获取步进值
	virtual FText GetFormattedText() const override;  // 获取格式化文本

	/** UGameSettingValueDiscreteDynamic */
	/** 设置动态 Getter 数据源 */
	void SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter);
	/** 设置动态 Setter 数据源 */
	void SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter);
	/** 设置默认值 */
	void SetDefaultValue(double InValue);

	/** 设置显示格式化函数 */
	void SetDisplayFormat(FSettingScalarFormatFunction InDisplayFormat);

	/** 设置源范围和步进值 */
	void SetSourceRangeAndStep(const TRange<double>& InRange, double InSourceStep);

	/**
	 * 设置最小限制值。例如滑块范围 0~100，但用户最低只能设为 1。
	 */
	void SetMinimumLimit(const TOptional<double>& InMinimum);

	/**
	 * 设置最大限制值。例如滑块范围 0~100，但用户最高只能设为 95。
	 */
	void SetMaximumLimit(const TOptional<double>& InMaximum);

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;

	/** 数据源就绪回调 */
	void OnDataSourcesReady();

protected:

	TSharedPtr<FGameSettingDataSource> Getter;  // Getter 数据源
	TSharedPtr<FGameSettingDataSource> Setter;  // Setter 数据源

	TOptional<double> DefaultValue;  // 默认值
	double InitialValue = 0;  // 初始值

	TRange<double> SourceRange = TRange<double>(0, 1);  // 源范围
	double SourceStep = 0.01;  // 步进值
	TOptional<double> Minimum;  // 最小限制
	TOptional<double> Maximum;  // 最大限制

	FSettingScalarFormatFunction DisplayFormat;  // 显示格式化函数
};
