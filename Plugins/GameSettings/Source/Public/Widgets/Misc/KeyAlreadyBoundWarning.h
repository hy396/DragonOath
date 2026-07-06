// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSettingPressAnyKey.h"
#include "KeyAlreadyBoundWarning.generated.h"

class UTextBlock;

/**
 * UKeyAlreadyBoundWarning
 * Press any key screen with text blocks for warning users when a key is already bound
 */
UCLASS(Abstract)
class GAMESETTINGS_API UKeyAlreadyBoundWarning : public UGameSettingPressAnyKey
{
	GENERATED_BODY()

public:
	void SetWarningText(const FText& InText);

	void SetCancelText(const FText& InText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> WarningText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> CancelText;
};
