// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Setly : ModuleRules
{
	public Setly(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.AddRange(Directory.GetDirectories(ModuleDirectory, "*", SearchOption.AllDirectories));

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ApplicationCore",
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
				"EnhancedInput",
				"CommonInput",
				"CommonUI",
				"CommonGame",
				"CommonUser",
				"CommonLoadingScreen",
				"GameSettings",
				"GameplayMessageRuntime",
				"UIExtension",
				"UMG",
				"AudioModulation",
				"AudioMixer",
				"DataRegistry",
				"ControlFlows",
				"GameSubtitles",
				"PropertyPath",
				"DeveloperSettings",
				"Projects",
				"InputCore",
				"Slate",
				"SlateCore",
				"RenderCore",
				"RHI",
				"ModularGameplay",
				"ModularGameplayActors",
				"AIModule",
				"NetCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AudioGameplay",
				"AudioGameplayVolume",
				"SoundUtilities",
				"EngineSettings"
			}
		);
	}
}
