// Copyright Epic Games, Inc. All Rights Reserved.

#include "Setly/SetlyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SetlyPlayerController)

ASetlyPlayerController::ASetlyPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ASetlyPlayerController::~ASetlyPlayerController()
{
}

void ASetlyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
			{
				if (const UInputMappingContext* MappingContext = Mapping.InputMapping.LoadSynchronous())
				{
					InputSubsystem->AddMappingContext(MappingContext, Mapping.Priority);
				}
			}
		}
	}
}

void ASetlyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
			{
				if (const UInputMappingContext* MappingContext = Mapping.InputMapping.Get())
				{
					InputSubsystem->RemoveMappingContext(MappingContext);
				}
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}
