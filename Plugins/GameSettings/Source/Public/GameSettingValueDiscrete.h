// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingValue.h"

#include "GameSettingValueDiscrete.generated.h"

class UObject;
struct FFrame;

/**
 * 离散值设置基类——选项为有限列表的设置（如下拉框）
 *
 * 提供按索引设置/获取选项的接口，子类需实现 GetDiscreteOptionIndex / SetDiscreteOptionByIndex。
 */
/**
 * 离散值设置基类，用于具有有限选项列表的设置（如下拉框、旋转器）。
 * 子类需实现选项的获取、设置和索引操作。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingValueDiscrete : public UGameSettingValue
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscrete();

	/** 通过索引设置离散选项 */
	virtual void SetDiscreteOptionByIndex(int32 Index) PURE_VIRTUAL(,);

	/** 获取当前选项索引 */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetDiscreteOptionIndex() const PURE_VIRTUAL(,return INDEX_NONE;);

	/** 获取默认选项索引（可选实现） */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetDiscreteOptionDefaultIndex() const { return INDEX_NONE; }

	/** 获取所有离散选项文本 */
	UFUNCTION(BlueprintCallable)
	virtual TArray<FText> GetDiscreteOptions() const PURE_VIRTUAL(,return TArray<FText>(););

	/** 获取分析用值 */
	virtual FString GetAnalyticsValue() const;
};
