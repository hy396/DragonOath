// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraHUDLayout.h"

#include "CommonUIExtensions.h"
#include "Input/CommonUIInputTypes.h"
#include "NativeGameplayTags.h"
#include "UI/LyraActivatableWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraHUDLayout)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_ACTION_ESCAPE, "UI.Action.Escape");

ULyraHUDLayout::ULyraHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// TODO: 目前我想要鼠标常驻
TOptional<FUIInputConfig> ULyraHUDLayout::GetDesiredInputConfig() const
{
	// HUD 常驻期间保留游戏和 UI 输入，但不捕获鼠标，避免 CommonUI 隐藏光标。
	return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture);
}

void ULyraHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(TAG_UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void ULyraHUDLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass);
	}
}
