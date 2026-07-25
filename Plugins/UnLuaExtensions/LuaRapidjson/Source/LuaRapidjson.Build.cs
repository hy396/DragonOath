// Tencent is pleased to support the open source community by making UnLua available.
// 
// Copyright (C) 2019 Tencent. All rights reserved.
//
// Licensed under the MIT License (the "License"); 
// you may not use this file except in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and limitations under the License.

using System;
using System.IO;
using UnrealBuildTool;

public class LuaRapidjson : ModuleRules
{
    public LuaRapidjson(ReadOnlyTargetRules Target) : base(Target)
    {
#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.None;
#else
        bEnforceIWYU = false;
#endif
        bUseUnity = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        // UE 5.5+ 为 TArray 新增 SizeType 成员类型，与 rapidjson 的 SizeType 在 "using namespace rapidjson" 下冲突（C4459）。
        // 该警告来自第三方库 rapidjson 的命名空间使用，关闭 ShadowVariable 警告级别以 /wd4459 抑制，避免阻断编译。
        CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
            });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "UnLua",
            "Lua"
        });

        PrivateDefinitions.AddRange(
            new[]
            {
                "LUA_LIB"
            }
        );
        
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "src"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
    }
}