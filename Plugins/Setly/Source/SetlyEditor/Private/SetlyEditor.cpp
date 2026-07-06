// Copyright Epic Games, Inc. All Rights Reserved.

#include "SetlyEditor.h"

#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"

#define LOCTEXT_NAMESPACE "FSetlyEditorModule"

namespace
{
	UObject* GetMutableDefaultObjectByClassPath(const TCHAR* ClassPath)
	{
		UClass* SettingsClass = FindObject<UClass>(nullptr, ClassPath);
		return SettingsClass ? SettingsClass->GetDefaultObject() : nullptr;
	}
}

void FSetlyEditorModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		if (UObject* UIManagerSettings = GetMutableDefaultObjectByClassPath(TEXT("/Script/Setly.LyraUIManagerSubsystem")))
		{
			SettingsModule->RegisterSettings(
				"Project",
				"Game",
				"SetlyUIManager",
				LOCTEXT("SetlyUIManagerName", "Setly UI Manager"),
				LOCTEXT("SetlyUIManagerDescription", "Configure the Setly root UI policy used to create CommonUI layout layers."),
				UIManagerSettings);
		}

		if (UObject* UIMessagingSettings = GetMutableDefaultObjectByClassPath(TEXT("/Script/Setly.LyraUIMessaging")))
		{
			SettingsModule->RegisterSettings(
				"Project",
				"Game",
				"SetlyUIMessaging",
				LOCTEXT("SetlyUIMessagingName", "Setly UI Messaging"),
				LOCTEXT("SetlyUIMessagingDescription", "Configure the Setly confirmation and error dialog widgets."),
				UIMessagingSettings);
		}
	}
}

void FSetlyEditorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Game", "SetlyUIManager");
		SettingsModule->UnregisterSettings("Project", "Game", "SetlyUIMessaging");
	}
}

IMPLEMENT_MODULE(FSetlyEditorModule, SetlyEditor);

#undef LOCTEXT_NAMESPACE
