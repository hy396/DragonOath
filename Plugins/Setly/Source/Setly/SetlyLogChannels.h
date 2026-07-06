// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

SETLY_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
SETLY_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
SETLY_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
SETLY_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);

SETLY_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
