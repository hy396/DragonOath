// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"

#include "LyraPerfStatGraph.generated.h"

/**
 * Base class for the performance stat graph widget blueprint.
 */
UCLASS(Blueprintable)
class SETLY_API ULyraPerfStatGraph : public UUserWidget
{
	GENERATED_BODY()

public:
	ULyraPerfStatGraph(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
