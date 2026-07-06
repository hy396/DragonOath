// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "Misc/TextFilterExpressionEvaluator.h"

#include "UObject/ObjectPtr.h"
#include "GameSettingFilterState.generated.h"

class ULocalPlayer;
class UGameSetting;
class UGameSettingCollection;

/** 设置变更原因 */
enum class EGameSettingChangeReason : uint8
{
	Change,             // 用户主动变更
	DependencyChanged,  // 依赖设置变更
	ResetToDefault,     // 重置为默认值
	RestoreToInitial,   // 恢复为初始值
};

/**
 * 过滤器状态，用于控制哪些设置在 UI 中可见。
 * 支持搜索文本、白名单、根列表等多种过滤方式。
 */
USTRUCT()
struct GAMESETTINGS_API FGameSettingFilterState
{
	GENERATED_BODY()

public:

	FGameSettingFilterState();

	UPROPERTY()
	bool bIncludeDisabled = true;  // 是否包含禁用的设置

	UPROPERTY()
	bool bIncludeHidden = false;  // 是否包含隐藏的设置

	UPROPERTY()
	bool bIncludeResetable = true;  // 是否包含可重置的设置

	UPROPERTY()
	bool bIncludeNestedPages = false;  // 是否包含嵌套页面

public:
	/** 设置搜索文本 */
	void SetSearchText(const FString& InSearchText);

	/** 判断设置是否通过过滤条件 */
	bool DoesSettingPassFilter(const UGameSetting& InSetting) const;

	/** 添加设置到根列表 */
	void AddSettingToRootList(UGameSetting* InSetting);
	/** 添加设置到白名单 */
	void AddSettingToAllowList(UGameSetting* InSetting);

	/** 判断设置是否在白名单中 */
	bool IsSettingInAllowList(const UGameSetting* InSetting) const
	{
		return SettingAllowList.Contains(InSetting);
	}

	/** 获取根列表设置 */
	const TArray<UGameSetting*>& GetSettingRootList() const { return SettingRootList; }
	/** 判断设置是否在根列表中 */
	bool IsSettingInRootList(const UGameSetting* InSetting) const
	{
		return SettingRootList.Contains(InSetting);
	}

private:
	FTextFilterExpressionEvaluator SearchTextEvaluator;  // 搜索文本评估器

	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingRootList;  // 根列表设置

	// 如果白名单非空，则仅白名单中的设置可见
	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingAllowList;  // 白名单设置
};

/**
 * 可编辑状态，捕获设置当前的可见性、启用状态及其原因。
 * 用于控制设置在 UI 中的展示和交互行为。
 */
class GAMESETTINGS_API FGameSettingEditableState
{
public:
	FGameSettingEditableState()
		: bVisible(true)
		, bEnabled(true)
		, bResetable(true)
		, bHideFromAnalytics(false)
	{
	}

	bool IsVisible() const { return bVisible; }
	bool IsEnabled() const { return bEnabled; }
	bool IsResetable() const { return bResetable; }
	bool IsHiddenFromAnalytics() const { return bHideFromAnalytics; }
	const TArray<FText>& GetDisabledReasons() const { return DisabledReasons; }

#if !UE_BUILD_SHIPPING
	const TArray<FString>& GetHiddenReasons() const { return HiddenReasons; }
#endif

	const TArray<FString>& GetDisabledOptions() const { return DisabledOptions; }

	/** 隐藏设置，需提供开发者原因（不面向用户） */
	void Hide(const FString& DevReason);

	/** 禁用设置，需提供禁用原因 */
	void Disable(const FText& Reason);

	/** 禁用离散选项，目前主要用于家长控制 */
	void DisableOption(const FString& Option);

	/** 通过枚举值禁用离散选项 */
	template<typename EnumType>
	void DisableEnumOption(EnumType InEnumValue)
	{
		DisableOption(StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue));
	}

	/** 标记设置不可重置 */
	void UnableToReset();

	/** 对分析系统隐藏，适用于平台特定设置等无需上报的场景 */
	void HideFromAnalytics() { bHideFromAnalytics = true; }

	/** 完全隐藏：视觉隐藏 + 不可重置 + 分析隐藏 */
	void Kill(const FString& DevReason)
	{
		Hide(DevReason);
		HideFromAnalytics();
		UnableToReset();
	}

private:
	uint8 bVisible : 1;              // 是否可见
	uint8 bEnabled : 1;               // 是否启用
	uint8 bResetable : 1;             // 是否可重置
	uint8 bHideFromAnalytics : 1;     // 是否对分析隐藏

	TArray<FString> DisabledOptions;  // 被禁用的选项列表

	TArray<FText> DisabledReasons;     // 禁用原因列表

#if !UE_BUILD_SHIPPING
	TArray<FString> HiddenReasons;     // 隐藏原因列表（仅非发布版）
#endif
};

/**
 * 编辑条件基类，可监控游戏状态或其他设置的变更来调整可见性。
 * 子类通过 GatherEditState() 修改设置的可编辑状态。
 */
class GAMESETTINGS_API FGameSettingEditCondition : public TSharedFromThis<FGameSettingEditCondition>
{
public:
	FGameSettingEditCondition() { }
	virtual ~FGameSettingEditCondition() { }

	/** 编辑条件变更事件 */
	DECLARE_EVENT_OneParam(FGameSettingEditCondition, FOnEditConditionChanged, bool);
	FOnEditConditionChanged OnEditConditionChangedEvent;  // 编辑条件变更事件委托

	/** 广播编辑条件变更事件 */
	void BroadcastEditConditionChanged()
	{
		OnEditConditionChangedEvent.Broadcast(true);
	}

	/** 设置初始化时调用 */
	virtual void Initialize(const ULocalPlayer* InLocalPlayer)
	{
	}

	/** 设置应用时调用 */
	virtual void SettingApplied(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting) const
	{
	}

	/** 设置值变更时调用 */
	virtual void SettingChanged(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting, EGameSettingChangeReason Reason) const
	{
	}

	/** 收集编辑状态，在依赖变更或编辑条件触发时调用 */
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
	{
	}

	/** 生成调试文本，方便排查问题 */
	virtual FString ToString() const { return TEXT(""); }
};
