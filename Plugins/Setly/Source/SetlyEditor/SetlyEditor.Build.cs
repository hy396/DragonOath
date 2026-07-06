// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SetlyEditor : ModuleRules
{
	public SetlyEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Setly"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Settings"
			}
		);
	}
}
