// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DragonOath : ModuleRules
{
	public DragonOath(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 把模块根目录加入 include path，让 "AbilitySystem/Core/xxx.h" 这种路径能解析
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine",
			"NetCore",  // FFastArraySerializer 在 UE5 中位于 Net/Core 模块（FastArray 网络复制依赖）
			"InputCore", "EnhancedInput",
			"AIModule", "StateTreeModule", "GameplayStateTreeModule",
			"UMG", "Slate", "SlateCore",
			"GameplayAbilities", "GameplayTags", "GameplayTasks",
			"GameplayMessageRuntime",
			"ModularGameplay",
			"Setly",
			"Niagara", "NavigationSystem",
			"HTTP", "WebSockets", "Json", "JsonUtilities",
			});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
