#pragma once

#include "NativeGameplayTags.h"

/**
 * DragonOath 项目级 GameplayTag 集中声明。
 *
 * 约定：
 * - C++ 代码优先引用这里的变量，不直接手写 RequestGameplayTag 字符串。
 * - Setly/Lyra 插件自己的 Tag 仍然保留在 SetlyGameplayTags.h；本文件只放 DragonOath 项目自己的 Tag。
 * - 新增 GAS Tag 时，同时考虑是否需要补到 Config/DefaultGameplayTags.ini，方便编辑器和蓝图选择。
 */
namespace DragonOathGameplayTags
{
	// Gameplay 状态 / 阻塞类
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Gameplay_AbilityInputBlocked);

	// GameplayEvent
    namespace Event
    {
        DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
    }

	// // 原生输入
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Jump);

	// // 技能输入槽位
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Primary);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Secondary);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Skill1);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Skill2);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Skill3);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Skill4);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Ultimate);
	// DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Dodge);

    // Gameplay 状态 / 阻塞类
    namespace Gameplay
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityInputBlocked);
	}

	namespace Event
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
	}

	namespace InputTag
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);
        // 技能输入槽位
		namespace Ability
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill1);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill2);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill3);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill4);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ultimate);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dodge);
		}
	}
}
