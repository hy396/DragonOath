// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtrTemplates.h"

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UGameSettingRegistry;
struct FObjectKey;

/**
 * 设置注册表变更追踪器——跟踪用户在设置面板中的修改
 *
 * 可批量应用所有变更（ApplyChanges）或恢复到打开面板时的状态（RestoreToInitial）。
 */
class GAMESETTINGS_API FGameSettingRegistryChangeTracker : public FNoncopyable
{
public:
	FGameSettingRegistryChangeTracker();
	~FGameSettingRegistryChangeTracker();

	/** 开始监视注册表 */
	void WatchRegistry(UGameSettingRegistry* InRegistry);
	/** 停止监视注册表 */
	void StopWatchingRegistry();

	/** 应用所有变更 */
	void ApplyChanges();

	/** 恢复到初始值 */
	void RestoreToInitial();

	/** 清除脏状态 */
	void ClearDirtyState();

	/** 是否正在恢复设置中 */
	bool IsRestoringSettings() const { return bRestoringSettings; }
	/** 设置是否已被修改 */
	bool HaveSettingsBeenChanged() const { return bSettingsChanged; }

private:
	/** 处理设置值变更 */
	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	bool bSettingsChanged = false;  // 设置是否已变更
	bool bRestoringSettings = false;  // 是否正在恢复设置

	TWeakObjectPtr<UGameSettingRegistry> Registry;  // 监视的注册表
	TMap<FObjectKey, TWeakObjectPtr<UGameSetting>> DirtySettings;  // 变更的设置映射
};
