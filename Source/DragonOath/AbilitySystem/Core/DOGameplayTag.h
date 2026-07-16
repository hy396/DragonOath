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

		// 死亡流程中（Dying）：UDOHealthComponent::StartDeath 应用。
		// 用途：让 GA / Ability 通过 ActivationOwnedTags Blocking(Status.Death.Dying) 自动拒绝激活；UI 可据此淡出交互按钮。
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death_Dying);

		// 死亡终态（Dead）：UDOHealthComponent::FinishDeath 应用。
		// 用途：最强死亡标记，AI / 技能互斥 / 战斗系统 全部按 Dead 走；复活链路清除此 Tag 并补满 Health。
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death_Dead);
	}

	namespace InputTag
	{
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);

		// 双击 A/D 触发的冲刺输入
		DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);

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
		namespace Combat
		{
			// 伤害事件 verb（FDOVerbMessage 的 Verb 字段），Instigator 攻击 Target，Magnitude = 伤害值。
			// 典型订阅方：HUD 伤害数字、HIT 受击反馈、连击检测处理器。
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageApplied);

			// 击杀事件 verb：Instigator 击杀 Target，Target 携带 Status.Dead 后广播。
			// 典型订阅方：连杀/助攻处理器、UI 击杀提示、GameplayCue（被击杀的轻量特效）。
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(EliminationFired);

			// 助攻事件 verb：Instigator 协助击杀 Target，Magnitude = 助攻伤害贡献值。
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AssistContributed);

			// 重置事件 verb：服务器权威事件，例如回合重置、关卡重载。
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayReset);

			// 父频道：PartialMatch 监听所有 Combat 子事件用。
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Combat);
		}

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