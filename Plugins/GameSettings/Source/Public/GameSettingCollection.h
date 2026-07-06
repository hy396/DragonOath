// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSetting.h"

#include "GameSettingCollection.generated.h"

struct FGameSettingFilterState;

//--------------------------------------
// UGameSettingCollection
//--------------------------------------

/**
 * 设置集合，可包含子设置的容器。不可选中，仅作为组织结构。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingCollection : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingCollection();

	/** 获取子设置列表 */
	virtual TArray<UGameSetting*> GetChildSettings() override { return Settings; }
	/** 获取子集合列表 */
	TArray<UGameSettingCollection*> GetChildCollections() const;

	/** 添加子设置 */
	void AddSetting(UGameSetting* Setting);
	/** 根据过滤条件获取设置列表 */
	virtual void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const;

	/** 是否可选中，集合默认不可选中 */
	virtual bool IsSelectable() const { return false; }

protected:
	/** 集合拥有的子设置 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> Settings;
};

//--------------------------------------
// UGameSettingCollectionPage
//--------------------------------------

/**
 * 设置导航页，可选中并导航到子设置页面的集合。
 * 通常对应设置面板中的一个分类页。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingCollectionPage : public UGameSettingCollection
{
	GENERATED_BODY()

public:

	/** 导航执行事件 */
	DECLARE_EVENT_OneParam(UGameSettingCollectionPage, FOnExecuteNavigation, UGameSetting* /*Setting*/);
	FOnExecuteNavigation OnExecuteNavigationEvent;  // 导航执行事件委托

public:
	UGameSettingCollectionPage();

	/** 获取导航文本 */
	FText GetNavigationText() const { return NavigationText; }
	// 设置导航文本
	void SetNavigationText(FText Value) { NavigationText = Value; }
#if !UE_BUILD_SHIPPING
	/** 非发布版本的重载 */
	void SetNavigationText(const FString& Value) { SetNavigationText(FText::FromString(Value)); }
#endif

	virtual void OnInitialized() override;
	virtual void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const override;
	/** 导航页可选中 */
	virtual bool IsSelectable() const override { return true; }

	/** 执行导航操作 */
	void ExecuteNavigation();

private:
	FText NavigationText;  // 导航文本
};
