// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：异步加载生命周期辅助，用来串联和取消异步加载请求，避免对象销毁后回调失效。

using UnrealBuildTool;

public class AsyncMixin : ModuleRules
{
	public AsyncMixin(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

        PublicIncludePathModuleNames.AddRange(
            new string[] {
            }
        );

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
