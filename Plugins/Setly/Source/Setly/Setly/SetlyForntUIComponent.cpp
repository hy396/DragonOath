// Copyright Epic Games, Inc. All Rights Reserved.

#include "Setly/SetlyForntUIComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Player/LyraLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SetlyForntUIComponent)

USetlyForntUIComponent::USetlyForntUIComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USetlyForntUIComponent::BeginPlay()
{
	Super::BeginPlay();

	const APawn* PawnOwner = Cast<APawn>(GetOwner());
	const APlayerController* PlayerController = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : Cast<APlayerController>(GetOwner());
	if (!PlayerController)
	{
		return;
	}

	CachedLocalPlayer = Cast<ULyraLocalPlayer>(PlayerController->GetLocalPlayer());
	if (!CachedLocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = CachedLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		for (const FSetlyInputMappingContextAndPriority& Mapping : DefaultInputMappings)
		{
			if (const UInputMappingContext* MappingContext = Mapping.InputMapping.LoadSynchronous())
			{
				InputSubsystem->AddMappingContext(MappingContext, Mapping.Priority);
			}
		}
	}
}

void USetlyForntUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedLocalPlayer)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = CachedLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FSetlyInputMappingContextAndPriority& Mapping : DefaultInputMappings)
			{
				if (const UInputMappingContext* MappingContext = Mapping.InputMapping.Get())
				{
					InputSubsystem->RemoveMappingContext(MappingContext);
				}
			}
		}
	}

	CachedLocalPlayer = nullptr;

	Super::EndPlay(EndPlayReason);
}

bool USetlyForntUIComponent::IsForceFeedbackEnabled() const
{
	return bForceFeedbackEnabled;
}
