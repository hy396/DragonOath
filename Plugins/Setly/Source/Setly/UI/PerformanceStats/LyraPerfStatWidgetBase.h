// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"

#include "LyraPerfStatWidgetBase.generated.h"

enum class ELyraDisplayablePerformanceStat : uint8;

class ULyraPerformanceStatSubsystem;
class UObject;
struct FFrame;

/**
 * ULyraPerfStatWidgetBase
 *
 * Base class for a widget that displays a single stat, e.g., FPS, ping, etc...
 */
 UCLASS(Abstract)
class ULyraPerfStatWidgetBase : public UCommonUserWidget
{
public:
	ULyraPerfStatWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	GENERATED_BODY()

public:
	// Returns the stat this widget is supposed to display
	UFUNCTION(BlueprintPure)
	ELyraDisplayablePerformanceStat GetStatToDisplay() const
	{
		return StatToDisplay;
	}

	// Polls for the value of this stat (unscaled)
	UFUNCTION(BlueprintPure)
	double FetchStatValue();

	// Compatibility entry point for graph widgets migrated from Lyra UI assets.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Display)
	void UpdateGraphData(float ScaleFactor);

protected:
	// Cached subsystem pointer
	UPROPERTY(Transient)
	TObjectPtr<ULyraPerformanceStatSubsystem> CachedStatSubsystem;

	// The stat to display
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Display)
	ELyraDisplayablePerformanceStat StatToDisplay;
 };
