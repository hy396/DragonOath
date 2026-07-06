// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingValueDiscrete.h"

#include "GameSettingValueDiscreteDynamic.generated.h"

class FGameSettingDataSource;
enum class EGameSettingChangeReason : uint8;

struct FContentControlsRules;

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic
//////////////////////////////////////////////////////////////////////////

/**
 * 动态离散值设置——运行时从数据源（DataSource）读取可用选项
 *
 * 支持动态获取选项列表、Getter/Setter 函数绑定，
 * 以及内容控制规则（如家长控制自动隐藏某些选项）。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic : public UGameSettingValueDiscrete
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscreteDynamic();

	/** UGameSettingValue */
	virtual void Startup() override;
	virtual void StoreInitial() override;  // 存储初始值
	virtual void ResetToDefault() override;  // 重置为默认值
	virtual void RestoreToInitial() override;  // 恢复为初始值

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;  // 通过索引设置选项
	virtual int32 GetDiscreteOptionIndex() const override;  // 获取当前选项索引
	virtual int32 GetDiscreteOptionDefaultIndex() const override;  // 获取默认选项索引
	virtual TArray<FText> GetDiscreteOptions() const override;  // 获取所有选项文本

	/** UGameSettingValueDiscreteDynamic */
	/** 设置动态 Getter 数据源 */
	void SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter);
	/** 设置动态 Setter 数据源 */
	void SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter);
	/** 从字符串设置默认值 */
	void SetDefaultValueFromString(FString InOptionValue);
	/** 添加动态选项 */
	void AddDynamicOption(FString InOptionValue, FText InOptionText);
	/** 移除动态选项 */
	void RemoveDynamicOption(FString InOptionValue);
	/** 获取动态选项值列表 */
	const TArray<FString>& GetDynamicOptions();

	/** 是否包含指定选项 */
	bool HasDynamicOption(const FString& InOptionValue);

	/** 获取当前值字符串 */
	FString GetValueAsString() const;
	/** 通过字符串设置值 */
	void SetValueFromString(FString InStringValue);

protected:
	void SetValueFromString(FString InStringValue, EGameSettingChangeReason Reason);

	/** UGameSettingValue */
	virtual void OnInitialized() override;

	/** 数据源就绪回调 */
	void OnDataSourcesReady();

	/** 比较两个选项是否相等 */
	bool AreOptionsEqual(const FString& InOptionA, const FString& InOptionB) const;

protected:
	TSharedPtr<FGameSettingDataSource> Getter;  // Getter 数据源
	TSharedPtr<FGameSettingDataSource> Setter;  // Setter 数据源

	TOptional<FString> DefaultValue;  // 默认值
	FString InitialValue;  // 初始值

	TArray<FString> OptionValues;  // 选项值列表
	TArray<FText> OptionDisplayTexts;  // 选项显示文本列表
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Bool
//////////////////////////////////////////////////////////////////////////

/**
 * 布尔型动态离散值设置，提供开/关两个选项。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Bool : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscreteDynamic_Bool();

public:
	/** 设置布尔默认值 */
	void SetDefaultValue(bool Value);

	/** 设置"真"选项文本 */
	void SetTrueText(const FText& InText);
	/** 设置"假"选项文本 */
	void SetFalseText(const FText& InText);

#if !UE_BUILD_SHIPPING
	void SetTrueText(const FString& Value) { SetTrueText(FText::FromString(Value)); }
	void SetFalseText(const FString& Value) { SetFalseText(FText::FromString(Value)); }
#endif
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Number
//////////////////////////////////////////////////////////////////////////

/**
 * 数字型动态离散值设置，支持模板化的数字类型操作。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Number : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscreteDynamic_Number();

public:
	/** 设置数字默认值 */
	template<typename NumberType>
	void SetDefaultValue(NumberType InValue)
	{
		SetDefaultValueFromString(LexToString(InValue));
	}

	/** 添加数字选项 */
	template<typename NumberType>
	void AddOption(NumberType InValue, const FText& InOptionText)
	{
		AddDynamicOption(LexToString(InValue), InOptionText);
	}

	/** 获取数字值 */
	template<typename NumberType>
	NumberType GetValue() const
	{
		const FString ValueString = GetValueAsString();

		NumberType OutValue;
		LexFromString(OutValue, *ValueString);

		return OutValue;
	}

	/** 设置数字值 */
	template<typename NumberType>
	void SetValue(NumberType InValue)
	{
		SetValueFromString(LexToString(InValue));
	}

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Enum
//////////////////////////////////////////////////////////////////////////

/**
 * 枚举型动态离散值设置，自动将枚举值与字符串互转。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Enum : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscreteDynamic_Enum();

public:
	/** 设置枚举默认值 */
	template<typename EnumType>
	void SetDefaultValue(EnumType InEnumValue)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue);
		SetDefaultValueFromString(StringValue);
	}

	/** 添加枚举选项 */
	template<typename EnumType>
	void AddEnumOption(EnumType InEnumValue, const FText& InOptionText)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue);
		AddDynamicOption(StringValue, InOptionText);
	}

	/** 获取枚举值 */
	template<typename EnumType>
	EnumType GetValue() const
	{
		const FString Value = GetValueAsString();
		return (EnumType)StaticEnum<EnumType>()->GetValueByNameString(Value);
	}

	/** 设置枚举值 */
	template<typename EnumType>
	void SetValue(EnumType InEnumValue)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue);
		SetValueFromString(StringValue);
	}

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Color
//////////////////////////////////////////////////////////////////////////

/**
 * 颜色型动态离散值设置，支持 FLinearColor 值的设置与获取。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Color : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscreteDynamic_Color();

public:
	/** 设置颜色默认值 */
	void SetDefaultValue(FLinearColor InColor)
	{
		SetDefaultValueFromString(InColor.ToString());
	}

	/** 添加颜色选项 */
	void AddColorOption(FLinearColor InColor)
	{
		const FColor SRGBColor = InColor.ToFColor(true);
		AddDynamicOption(InColor.ToString(), FText::FromString(FString::Printf(TEXT("#%02X%02X%02X"), SRGBColor.R, SRGBColor.G, SRGBColor.B)));
	}

	/** 获取颜色值 */
	FLinearColor GetValue() const
	{
		const FString Value = GetValueAsString();

		FLinearColor ColorValue;
		bool bSuccess = ColorValue.InitFromString(Value);
		ensure(bSuccess);

		return ColorValue;
	}

	/** 设置颜色值 */
	void SetValue(FLinearColor InColor)
	{
		SetValueFromString(InColor.ToString());
	}
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Vector2D
//////////////////////////////////////////////////////////////////////////

/**
 * 二维向量型动态离散值设置，支持 FVector2D 值的设置与获取。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Vector2D : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:

	UGameSettingValueDiscreteDynamic_Vector2D() { }

	/** 设置二维向量默认值 */
	void SetDefaultValue(const FVector2D& InValue)
	{
		SetDefaultValueFromString(InValue.ToString());
	}

	/** 获取二维向量值 */
	FVector2D GetValue() const
	{
		FVector2D ValueVector;
		ValueVector.InitFromString(GetValueAsString());
		return ValueVector;
	}

	/** 设置二维向量值 */
	void SetValue(const FVector2D& InValue)
	{
		SetValueFromString(InValue.ToString());
	}
};
