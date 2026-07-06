// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonPlayerController.h"
#include "UObject/SoftObjectPtr.h"

#include "SetlyPlayerController.generated.h"

class UInputMappingContext;

USTRUCT(BlueprintType)
struct SETLY_API FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "GameForntUI|Input", meta = (AssetBundles = "Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "GameForntUI|Input")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, Category = "GameForntUI|Input")
	bool bRegisterWithSettings = true;
};

UCLASS(Config = Game)
class SETLY_API ASetlyPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	ASetlyPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual ~ASetlyPlayerController() override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
};
