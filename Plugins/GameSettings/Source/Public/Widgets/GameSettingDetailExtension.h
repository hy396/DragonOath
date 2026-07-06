// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "Blueprint/UserWidget.h"

#include "GameSettingDetailExtension.generated.h"

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UObject;

/**
 *
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingDetailExtension : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSetting(UGameSetting* InSetting);

protected:
	virtual void NativeSettingAssigned(UGameSetting* InSetting);
	virtual void NativeSettingValueChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(UGameSetting* InSetting);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingValueChanged(UGameSetting* InSetting);

protected:
	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> Setting;
};
