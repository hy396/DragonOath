// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "GameSettingRegistryChangeTracker.h"

#include "GameSettingRegistry.h"
#include "GameSettingValue.h"

#define LOCTEXT_NAMESPACE "GameSetting"

FGameSettingRegistryChangeTracker::FGameSettingRegistryChangeTracker()
{
}

FGameSettingRegistryChangeTracker::~FGameSettingRegistryChangeTracker()
{
	if (UGameSettingRegistry* StrongRegistry = Registry.Get())
	{
		StrongRegistry->OnSettingChangedEvent.RemoveAll(this);
	}
}

void FGameSettingRegistryChangeTracker::WatchRegistry(UGameSettingRegistry* InRegistry)
{
	ClearDirtyState();
	StopWatchingRegistry();

	if (Registry.Get() != InRegistry)
	{
		Registry = InRegistry;
		InRegistry->OnSettingChangedEvent.AddRaw(this, &FGameSettingRegistryChangeTracker::HandleSettingChanged);
	}
}

void FGameSettingRegistryChangeTracker::StopWatchingRegistry()
{
	if (UGameSettingRegistry* StrongRegistry = Registry.Get())
	{
		StrongRegistry->OnSettingChangedEvent.RemoveAll(this);
		Registry.Reset();
	}
}

void FGameSettingRegistryChangeTracker::ClearDirtyState()
{
	ensure(!bRestoringSettings);
	if (bRestoringSettings)
	{
		return;
	}

	bSettingsChanged = false;
	DirtySettings.Reset();
}

void FGameSettingRegistryChangeTracker::ApplyChanges()
{
	for (auto Entry : DirtySettings)
	{
		if (UGameSettingValue* SettingValue = Cast<UGameSettingValue>(Entry.Value))
		{
			SettingValue->Apply();
			SettingValue->StoreInitial();
		}
	}

	ClearDirtyState();
}

void FGameSettingRegistryChangeTracker::RestoreToInitial()
{
	ensure(!bRestoringSettings);
	if (bRestoringSettings)
	{
		return;
	}

	{
		TGuardValue<bool> LocalGuard(bRestoringSettings, true);
		for (auto Entry : DirtySettings)
		{
			if (UGameSettingValue* SettingValue = Cast<UGameSettingValue>(Entry.Value))
			{
				SettingValue->RestoreToInitial();
			}
		}
	}

	ClearDirtyState();
}

void FGameSettingRegistryChangeTracker::HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason)
{
	if (bRestoringSettings)
	{
		return;
	}

	bSettingsChanged = true;
	DirtySettings.Add(FObjectKey(Setting), Setting);
}

#undef LOCTEXT_NAMESPACE
