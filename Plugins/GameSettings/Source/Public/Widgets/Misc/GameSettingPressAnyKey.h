// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "CommonActivatableWidget.h"

#include "GameSettingPressAnyKey.generated.h"

struct FKey;

class UObject;

/**
 *
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingPressAnyKey : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UGameSettingPressAnyKey(const FObjectInitializer& Initializer);

	DECLARE_EVENT_OneParam(UGameSettingPressAnyKey, FOnKeySelected, FKey);
	FOnKeySelected OnKeySelected;

	DECLARE_EVENT(UGameSettingPressAnyKey, FOnKeySelectionCanceled);
	FOnKeySelectionCanceled OnKeySelectionCanceled;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void HandleKeySelected(FKey InKey);
	void HandleKeySelectionCanceled();

	void Dismiss(TFunction<void()> PostDismissCallback);

private:
	bool bKeySelected = false;
	TSharedPtr<class FSettingsPressAnyKeyInputPreProcessor> InputProcessor;
};
