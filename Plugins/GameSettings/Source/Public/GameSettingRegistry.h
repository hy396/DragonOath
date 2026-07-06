// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSetting.h"
#include "Templates/Casts.h"

#include "GameSettingRegistry.generated.h"

struct FGameplayTag;

//--------------------------------------
// UGameSettingRegistry
//--------------------------------------

class ULocalPlayer;
struct FGameSettingFilterState;

enum class EGameSettingChangeReason : uint8;

/**
 * 设置注册表——管理所有设置的树状结构
 *
 * 游戏子类化此对象来定义自己的设置项，注册表持有所有顶级设置的引用，
 * 并提供设置变更、编辑条件变更、命名动作等事件。
 */
UCLASS(Abstract, BlueprintType)
class GAMESETTINGS_API UGameSettingRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** 设置值变更事件 */
	DECLARE_EVENT_TwoParams(UGameSettingRegistry, FOnSettingChanged, UGameSetting*, EGameSettingChangeReason);
	/** 编辑条件变更事件 */
	DECLARE_EVENT_OneParam(UGameSettingRegistry, FOnSettingEditConditionChanged, UGameSetting*);

	FOnSettingChanged OnSettingChangedEvent;  // 设置值变更事件委托
	FOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;  // 编辑条件变更事件委托

	/** 命名动作事件 */
	DECLARE_EVENT_TwoParams(UGameSettingRegistry, FOnSettingNamedActionEvent, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);
	FOnSettingNamedActionEvent OnSettingNamedActionEvent;  // 命名动作事件委托

	/** 导航到子设置事件 */
	DECLARE_EVENT_OneParam(UGameSettingRegistry, FOnExecuteNavigation, UGameSetting* /*Setting*/);
	FOnExecuteNavigation OnExecuteNavigationEvent;  // 导航事件委托

public:
	UGameSettingRegistry();

	/** 初始化注册表 */
	void Initialize(ULocalPlayer* InLocalPlayer);

	/** 重新生成所有设置 */
	virtual void Regenerate();

	/** 是否完成初始化 */
	virtual bool IsFinishedInitializing() const;

	/** 保存变更 */
	virtual void SaveChanges();

	/** 根据过滤条件获取设置列表 */
	void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings);

	/** 通过开发者名称查找设置 */
	UGameSetting* FindSettingByDevName(const FName& SettingDevName);

	/** 通过开发者名称查找设置（带类型检查） */
	template<typename T = UGameSetting>
	T* FindSettingByDevNameChecked(const FName& SettingDevName)
	{
		T* Setting = Cast<T>(FindSettingByDevName(SettingDevName));
		check(Setting);
		return Setting;
	}

protected:
	/** 初始化回调，子类必须实现以注册设置 */
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) PURE_VIRTUAL(, )

	/** 设置应用回调 */
	virtual void OnSettingApplied(UGameSetting* Setting) { }

	/** 注册设置到注册表 */
	void RegisterSetting(UGameSetting* InSetting);
	/** 注册内部嵌套设置 */
	void RegisterInnerSettings(UGameSetting* InSetting);

	// 内部事件处理器
	/** 处理设置值变更 */
	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);
	/** 处理设置应用 */
	void HandleSettingApplied(UGameSetting* Setting);
	/** 处理编辑条件变更 */
	void HandleSettingEditConditionsChanged(UGameSetting* Setting);
	/** 处理命名动作 */
	void HandleSettingNamedAction(UGameSetting* Setting, FGameplayTag GameSettings_Action_Tag);
	/** 处理导航 */
	void HandleSettingNavigation(UGameSetting* Setting);

	/** 顶级设置列表 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> TopLevelSettings;

	/** 所有已注册的设置列表 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> RegisteredSettings;

	/** 所属本地玩家 */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> OwningLocalPlayer;
};
