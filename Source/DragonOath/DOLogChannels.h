// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

DRAGONOATH_API DECLARE_LOG_CATEGORY_EXTERN(LogDragonOath, Log, All);
DRAGONOATH_API DECLARE_LOG_CATEGORY_EXTERN(LogDragonOathExperience, Log, All);
DRAGONOATH_API DECLARE_LOG_CATEGORY_EXTERN(LogDragonOathAbilitySystem, Log, All);
DRAGONOATH_API DECLARE_LOG_CATEGORY_EXTERN(LogDragonOathTeams, Log, All);

DRAGONOATH_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
