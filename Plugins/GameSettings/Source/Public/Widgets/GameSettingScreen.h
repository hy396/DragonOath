// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "CommonActivatableWidget.h"
#include "GameSettingRegistry.h"
#include "GameSettingRegistryChangeTracker.h"

#include "GameSettingScreen.generated.h"

class UGameSetting;
class UGameSettingCollection;
class UGameSettingPanel;
class UObject;
class UWidget;
struct FFrame;

enum class EGameSettingChangeReason : uint8;

/**
 *
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UFUNCTION(BlueprintCallable)
	void NavigateToSetting(FName SettingDevName);

	UFUNCTION(BlueprintCallable)
	void NavigateToSettings(const TArray<FName>& SettingDevNames);

	UFUNCTION(BlueprintNativeEvent)
	void OnSettingsDirtyStateChanged(bool bSettingsDirty);
	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) { }

	UFUNCTION(BlueprintCallable)
	bool AttemptToPopNavigation();

	UFUNCTION(BlueprintCallable)
	UGameSettingCollection* GetSettingCollection(FName SettingDevName, bool& HasAnySettings);

protected:
	virtual UGameSettingRegistry* CreateRegistry() PURE_VIRTUAL(, return nullptr;);

	template <typename GameSettingRegistryT = UGameSettingRegistry>
	GameSettingRegistryT* GetRegistry() const { return Cast<GameSettingRegistryT>(const_cast<UGameSettingScreen*>(this)->GetOrCreateRegistry()); }

	UFUNCTION(BlueprintCallable)
	virtual void CancelChanges();

	UFUNCTION(BlueprintCallable)
	virtual void ApplyChanges();

	UFUNCTION(BlueprintCallable)
	bool HaveSettingsBeenChanged() const { return ChangeTracker.HaveSettingsBeenChanged(); }

	void ClearDirtyState();

	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	FGameSettingRegistryChangeTracker ChangeTracker;

private:
	UGameSettingRegistry* GetOrCreateRegistry();

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingPanel> Settings_Panel;

	UPROPERTY(Transient)
	mutable TObjectPtr<UGameSettingRegistry> Registry;
};
