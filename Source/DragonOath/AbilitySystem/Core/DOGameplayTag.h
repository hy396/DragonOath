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
	namespace Gameplay
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityInputBlocked);
	}

	// GameplayEvent
	namespace Event
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
	}

	// SetByCaller 数据传递标签
	namespace Data
	{
		// 技能基础伤害值，通过 SetByCaller 传给伤害 GE
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage);

		// 技能伤害倍率，通过 SetByCaller 传给伤害 GE
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageMultiplier);
	}

	// 伤害类型与标记（用于 ExecutionCalculation 区分伤害来源与吸血）
	namespace Damage
	{
		// 玩家造成的伤害：跳过命中判定（必中，靠走位/技能范围决定）
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TypePlayer);

		// 怪物造成的伤害：走命中/闪避判定
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TypeMonster);

		// 召唤物/宠物造成的伤害：走命中/闪避判定
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TypePet);

		// 该次伤害可触发吸血（LifeSteal）
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanLifeSteal);
	}

	// 状态标签（角色运行时状态）
	namespace Status
	{
		// 冲刺中：角色正在冲刺位移，带有无敌帧。
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dashing);

		// 冲刺攻击窗口：冲刺结束后的一段时间，此期间按普攻触发冲刺攻击。
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DashAttackWindow);
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

	// 职业标识
	namespace Profession
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DragonFighter);
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mage);
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Archer);
	}

	// 本地消息总线频道（GameplayMessageRouter）
	namespace Message
	{
		namespace UI
		{
			// 红点系统：某节点状态变化时广播，Payload = FDORedDotChangedMessage
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(RedDotChanged);

			namespace Tutorial
			{
				// 引导步骤切换/进度变化，Payload = FDOTutorialStepMessage
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StepChanged);
				// 请求/通知高亮某个控件，Payload = FDOTutorialFocusMessage
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Focus);
				// 清除高亮，Payload = FDOTutorialFocusMessage（bShow=false）
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ClearFocus);
				// 有可进行的引导时点亮入口红点，Payload 可空
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Available);
			}
		}
	}
}