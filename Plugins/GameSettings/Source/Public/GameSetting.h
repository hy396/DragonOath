// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameplayTagContainer.h"
#include "Components/SlateWrapperTypes.h"
#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

#include "GameSetting.generated.h"

class ULocalPlayer;
class UGameSettingRegistry;

//--------------------------------------
// UGameSetting
//--------------------------------------

DECLARE_DELEGATE_RetVal_OneParam(FText, FGetGameSettingsDetails, ULocalPlayer& /*InLocalPlayer*/);

/**
 * 设置基类，管理 DevName/DisplayName/描述/编辑条件/事件。
 * 所有具体设置类型（动作、离散值、标量值、集合等）均从此类派生。
 */
UCLASS(Abstract, BlueprintType)
class GAMESETTINGS_API UGameSetting : public UObject
{
	GENERATED_BODY()

public:
	UGameSetting() { }

public:
	// 设置值变更事件
	DECLARE_EVENT_TwoParams(UGameSetting, FOnSettingChanged, UGameSetting* /*InSetting*/, EGameSettingChangeReason /*InChangeReason*/);
	// 设置应用事件
	DECLARE_EVENT_OneParam(UGameSetting, FOnSettingApplied, UGameSetting* /*InSetting*/);
	// 编辑条件变更事件
	DECLARE_EVENT_OneParam(UGameSetting, FOnSettingEditConditionChanged, UGameSetting* /*InSetting*/);

	FOnSettingChanged OnSettingChangedEvent;  // 设置值变更事件委托
	FOnSettingApplied OnSettingAppliedEvent;  // 设置应用事件委托
	FOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;  // 编辑条件变更事件委托

public:

	/** 获取开发者名称（非本地化），在注册表中唯一标识此设置 */
	UFUNCTION(BlueprintCallable)
	FName GetDevName() const { return DevName; }
	// 设置开发者名称
	void SetDevName(const FName& Value) { DevName = Value; }

	// 是否在刷新后调整列表视图
	bool GetAdjustListViewPostRefresh() const { return bAdjustListViewPostRefresh; }
	void SetAdjustListViewPostRefresh(const bool Value) { bAdjustListViewPostRefresh = Value; }

	/** 获取显示名称 */
	UFUNCTION(BlueprintCallable)
	FText GetDisplayName() const { return DisplayName; }
	// 设置显示名称
	void SetDisplayName(const FText& Value) { DisplayName = Value; }
#if !UE_BUILD_SHIPPING
	/** 非发布版本的重载，用于作弊等不需要本地化的场景 */
	void SetDisplayName(const FString& Value) { SetDisplayName(FText::FromString(Value)); }
#endif
	/** 获取显示名称可见性 */
	UFUNCTION(BlueprintCallable)
	ESlateVisibility GetDisplayNameVisibility() { return DisplayNameVisibility; }
	// 设置显示名称可见性
	void SetNameDisplayVisibility(ESlateVisibility InVisibility) { DisplayNameVisibility = InVisibility; }

	/** 获取富文本描述 */
	UFUNCTION(BlueprintCallable)
	FText GetDescriptionRichText() const { return DescriptionRichText; }
	// 设置富文本描述
	void SetDescriptionRichText(const FText& Value) { DescriptionRichText = Value; InvalidateSearchableText(); }
#if !UE_BUILD_SHIPPING
	/** 非发布版本的重载，用于作弊等不需要本地化的场景 */
	void SetDescriptionRichText(const FString& Value) { SetDescriptionRichText(FText::FromString(Value)); }
#endif

	/** 获取标签容器 */
	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetTags() const { return Tags; }
	// 添加标签
	void AddTag(const FGameplayTag& TagToAdd) { Tags.AddTag(TagToAdd); }

	// 设置所属注册表
	void SetRegistry(UGameSettingRegistry* InOwningRegistry) { OwningRegistry = InOwningRegistry; }

	/** 获取可搜索的纯文本描述 */
	const FString& GetDescriptionPlainText() const;

	/** 初始化设置，分配所属本地玩家。集合会自动初始化其包含的子设置 */
	void Initialize(ULocalPlayer* InLocalPlayer);

	/** 获取所属本地玩家 */
	ULocalPlayer* GetOwningLocalPlayer() const { return LocalPlayer; }

	/** 设置动态详情回调，用于构建描述面板时查询，该文本不可搜索 */
	void SetDynamicDetails(const FGetGameSettingsDetails& InDynamicDetails) { DynamicDetails = InDynamicDetails; }

	/** 获取动态详情文本，例如剩余退款次数、账号信息等 */
	UFUNCTION(BlueprintCallable)
	FText GetDynamicDetails() const;

	/** 获取警告富文本 */
	UFUNCTION(BlueprintCallable)
	FText GetWarningRichText() const { return WarningRichText; }
	// 设置警告富文本
	void SetWarningRichText(const FText& Value) { WarningRichText = Value; InvalidateSearchableText(); }
#if !UE_BUILD_SHIPPING
	/** 非发布版本的重载，用于作弊等不需要本地化的场景 */
	void SetWarningRichText(const FString& Value) { SetWarningRichText(FText::FromString(Value)); }
#endif

	/** 获取当前编辑状态（基于编辑条件和过滤状态的缓存结果） */
	const FGameSettingEditableState& GetEditState() const { return EditableStateCache; }

	/** 添加编辑条件，用于控制设置的可见性和可编辑性 */
	void AddEditCondition(const TSharedRef<FGameSettingEditCondition>& InEditCondition);

	/** 添加设置依赖，当依赖设置变更时会重新评估本设置的编辑条件 */
	void AddEditDependency(UGameSetting* DependencySetting);

	/** 设置父设置对象，通常是集合，顶级设置则为注册表 */
	void SetSettingParent(UGameSetting* InSettingParent);
	// 获取父设置对象
	UGameSetting* GetSettingParent() const { return SettingParent; }

	/** 是否上报到分析系统 */
	bool GetIsReportedToAnalytics() const { return bReportAnalytics; }
	void SetIsReportedToAnalytics(bool bReport) { bReportAnalytics = bReport; }

	/** 获取分析用值 */
	virtual FString GetAnalyticsValue() const { return TEXT(""); }

	/** 设置是否已就绪（部分设置可能需要异步初始化） */
	bool IsReady() const { return bReady; }

	/** 获取子设置列表。集合/动作等非直接可见设置可包含内部子设置 */
	virtual TArray<UGameSetting*> GetChildSettings() { return TArray<UGameSetting*>(); }

	/** 刷新编辑状态并通知 UI 更新 */
	void RefreshEditableState(bool bNotifyEditConditionsChanged = true);

	/** 应用设置。大部分设置立即生效，部分设置（如分辨率）先暂存后统一应用 */
	void Apply();

	/** 获取所属本地玩家的当前 World */
	virtual UWorld* GetWorld() const override;

protected:
	// 异步启动流程
	virtual void Startup();
	// 启动完成回调
	void StartupComplete();

	// 初始化完成回调
	virtual void OnInitialized();
	// 应用设置回调
	virtual void OnApply();
	// 收集编辑状态
	virtual void OnGatherEditState(FGameSettingEditableState& InOutEditState) const;
	// 依赖变更回调
	virtual void OnDependencyChanged();

	// 获取动态详情内部实现
	virtual FText GetDynamicDetailsInternal() const;

	// 处理编辑依赖变更（带原因）
	void HandleEditDependencyChanged(UGameSetting* DependencySetting, EGameSettingChangeReason Reason);
	// 处理编辑依赖变更
	void HandleEditDependencyChanged(UGameSetting* DependencySetting);

	/** 重新生成可搜索纯文本（当文本被标记为脏时） */
	void RefreshPlainText() const;
	// 标记可搜索文本为脏
	void InvalidateSearchableText() { bRefreshPlainSearchableText = true; }

	// 通知设置值已变更
	void NotifySettingChanged(EGameSettingChangeReason Reason);
	// 设置值变更回调
	virtual void OnSettingChanged(EGameSettingChangeReason Reason);

	/** 通知编辑条件已变更（可能导致设置不可见/禁用/选项变化） */
	void NotifyEditConditionsChanged();
	// 编辑条件变更回调
	virtual void OnEditConditionsChanged();

	// 计算当前编辑状态
	FGameSettingEditableState ComputeEditableState() const;

protected:

	/** 所属本地玩家 */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer;

	/** 父设置对象 */
	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> SettingParent;

	/** 所属注册表 */
	UPROPERTY(Transient)
	TObjectPtr<UGameSettingRegistry> OwningRegistry;

	FName DevName;  // 开发者名称（非本地化标识符）
	FText DisplayName;  // 显示名称
	ESlateVisibility DisplayNameVisibility = ESlateVisibility::SelfHitTestInvisible;  // 显示名称可见性
	FText DescriptionRichText;  // 富文本描述
	FText WarningRichText;  // 警告富文本

	/** 标签集合，UI 可用其做任意标记 */
	FGameplayTagContainer Tags;

	/** 动态详情回调 */
	FGetGameSettingsDetails DynamicDetails;

	/** 编辑条件列表 */
	TArray<TSharedRef<FGameSettingEditCondition>> EditConditions;

	/** 字符串文化缓存，用于本地化文本的缓存管理 */
	class FStringCultureCache
	{
		FStringCultureCache(TFunction<FString()> InStringGetter);

		void Invalidate();

		FString Get() const;

	private:
		mutable FString StringCache;  // 缓存的字符串
		mutable FCultureRef Culture;  // 当前文化引用
		TFunction<FString()> StringGetter;  // 字符串获取函数
	};

	/** 文本变更时标记可搜索文本为脏 */
	mutable bool bRefreshPlainSearchableText = true;
	/** 自动生成的纯文本描述 */
	mutable FString AutoGenerated_DescriptionPlainText;

	/** 是否上报分析数据，默认不上报，仅 GameSettingValue 上报 */
	bool bReportAnalytics = false;

private:

	/** 设置是否已就绪，部分设置需要异步初始化 */
	bool bReady = false;

	/** 防止设置变更事件重入 */
	bool bOnSettingChangedEventGuard = false;

	/** 防止编辑条件变更事件重入 */
	bool bOnEditConditionsChangedEventGuard = false;

	/** 是否在刷新后调整列表视图 */
	bool bAdjustListViewPostRefresh = true;

	/** 编辑状态缓存，变更时更新而非每次需要时重新计算 */
	FGameSettingEditableState EditableStateCache;
};
