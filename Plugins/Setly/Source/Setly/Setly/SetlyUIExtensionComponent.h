// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Data/LyraUIExData.h"
#include "UIExtensionSystem.h"

#include "SetlyUIExtensionComponent.generated.h"

class UCommonActivatableWidget;

UCLASS(BlueprintType, Blueprintable, ClassGroup = UGameForntUI, meta = (BlueprintSpawnableComponent))
class SETLY_API USetlyUIExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USetlyUIExtensionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<FContentToLayerWidgetEntry> Layouts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<FRegisterExtensionAsWidgetEntry> RegisterWidgets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension", meta = (ClampMin = "0"))
	int32 PlayerIndex = 0;

private:
	TArray<FUIExtensionHandle> ExtensionHandles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonActivatableWidget>> PushedWidgets;
};
