// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace LyraGameplayTags
{
	SETLY_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// Declare all of the custom native tags that Lyra will use
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	SETLY_API	extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	SETLY_API	extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

	SETLY_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
};
