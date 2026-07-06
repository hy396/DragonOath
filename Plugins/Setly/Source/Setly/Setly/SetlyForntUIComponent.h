// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "UObject/SoftObjectPtr.h"

#include "SetlyForntUIComponent.generated.h"

class UInputMappingContext;
class ULyraLocalPlayer;

USTRUCT(BlueprintType)
struct SETLY_API FSetlyInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "GameForntUI|Input", meta = (AssetBundles = "Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "GameForntUI|Input")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, Category = "GameForntUI|Input")
	bool bRegisterWithSettings = true;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = GameForntUI, meta = (BlueprintSpawnableComponent))
class SETLY_API USetlyForntUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USetlyForntUIComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "GameForint|UI")
	bool IsForceFeedbackEnabled() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameForntUI|Input")
	TArray<FSetlyInputMappingContextAndPriority> DefaultInputMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameForntUI|Input")
	bool bForceFeedbackEnabled = true;

	UPROPERTY(Transient)
	TObjectPtr<ULyraLocalPlayer> CachedLocalPlayer = nullptr;
};
