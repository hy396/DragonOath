// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/LyraMappableConfigPair.h"

#include "CommonUISettings.h"
#include "ICommonUIModule.h"
#include "PlayerMappableInputConfig.h"
#include "Settings/LyraSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraMappableConfigPair)

PRAGMA_DISABLE_DEPRECATION_WARNINGS

bool FMappableConfigPair::CanBeActivated() const
{
	const FGameplayTagContainer& PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();

	if (!DependentPlatformTraits.IsEmpty() && !PlatformTraits.HasAll(DependentPlatformTraits))
	{
		return false;
	}

	if (!ExcludedPlatformTraits.IsEmpty() && PlatformTraits.HasAny(ExcludedPlatformTraits))
	{
		return false;
	}

	return true;
}

bool FMappableConfigPair::RegisterPair(const FMappableConfigPair& Pair)
{
	if (ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get())
	{
		if (const UPlayerMappableInputConfig* LoadedConfig = Pair.Config.LoadSynchronous())
		{
			Settings->RegisterInputConfig(Pair.Type, LoadedConfig, false);
			return true;
		}
	}

	return false;
}

void FMappableConfigPair::UnregisterPair(const FMappableConfigPair& Pair)
{
	if (ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get())
	{
		if (const UPlayerMappableInputConfig* LoadedConfig = Pair.Config.LoadSynchronous())
		{
			Settings->UnregisterInputConfig(LoadedConfig);
		}
	}
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS
