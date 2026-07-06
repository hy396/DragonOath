// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#include "GameSettingAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingAction)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingAction
//--------------------------------------

UGameSettingAction::UGameSettingAction()
{

}

void UGameSettingAction::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	ensureMsgf(HasCustomAction() || NamedAction.IsValid(), TEXT("Action settings need either a custom action or a named action."));
	ensureMsgf(!ActionText.IsEmpty(), TEXT("You must provide a ActionText for settings with actions."));
	ensureMsgf(!DescriptionRichText.IsEmpty(), TEXT("You must provide a description for settings with actions."));
#endif
}

void UGameSettingAction::SetCustomAction(TFunction<void(ULocalPlayer*)> InAction)
{
	CustomAction = UGameSettingCustomAction::CreateLambda([InAction](UGameSetting* /*Setting*/, ULocalPlayer* InLocalPlayer) {
		InAction(InLocalPlayer);
	});
}

void UGameSettingAction::ExecuteAction()
{
	if (HasCustomAction())
	{
		CustomAction.ExecuteIfBound(this, LocalPlayer);
	}
	else
	{
		OnExecuteNamedActionEvent.Broadcast(this, NamedAction);
	}

	if (bDirtyAction)
	{
		NotifySettingChanged(EGameSettingChangeReason::Change);
	}
}

#undef LOCTEXT_NAMESPACE
