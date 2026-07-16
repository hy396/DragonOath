#include "AbilitySystem/Core/DOGameplayTag.h"

namespace DragonOathGameplayTags
{
	namespace Gameplay
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityInputBlocked, "Gameplay.AbilityInputBlocked", "阻塞所属 ASC 的技能输入处理。");
	}

	namespace Event
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death, "Event.Death", "当 Actor 生命值降为零时发送的游戏事件。");
	}

	namespace Data
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage, "Data.Damage", "技能基础伤害值，通过 SetByCaller 传给伤害 GE。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(DamageMultiplier, "Data.DamageMultiplier", "技能伤害倍率，通过 SetByCaller 传给伤害 GE。");
	}

	namespace Damage
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(TypePlayer, "Damage.Type.Player", "玩家造成的伤害，跳过命中判定（必中）。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(TypeMonster, "Damage.Type.Monster", "怪物造成的伤害，走命中/闪避判定。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(TypePet, "Damage.Type.Pet", "召唤物/宠物造成的伤害，走命中/闪避判定。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CanLifeSteal, "Damage.CanLifeSteal", "该次伤害可触发吸血（LifeSteal）。");
	}

	namespace Status
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dashing, "Status.Dashing", "角色正在冲刺中，带有无敌帧。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(DashAttackWindow, "Status.DashAttackWindow", "冲刺结束后的冲刺攻击窗口，此期间按普攻触发冲刺攻击。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death_Dying, "Status.Death.Dying", "死亡流程中（Dying）：UDOHealthComponent::StartDeath 时应用。技能 ActivationOwnedTags Blocking 可据此自动拒绝激活。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death_Dead, "Status.Death.Dead", "死亡终态（Dead）：UDOHealthComponent::FinishDeath 时应用。AI / 技能互斥 / 战斗系统全部按 Dead 走。");
	}

	namespace InputTag
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Jump, "InputTag.Jump", "DragonOath 玩家角色使用的原生跳跃输入。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dash, "InputTag.Dash", "双击 A/D 触发冲刺的输入标签，由增强输入 UInputTriggerDoubleTap 触发。");

		namespace Ability
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Primary, "InputTag.Ability.Primary", "龙斗士主技能输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary, "InputTag.Ability.Secondary", "龙斗士副技能输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill1, "InputTag.Ability.Skill1", "龙斗士技能槽 1 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill2, "InputTag.Ability.Skill2", "龙斗士技能槽 2 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill3, "InputTag.Ability.Skill3", "龙斗士技能槽 3 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill4, "InputTag.Ability.Skill4", "龙斗士技能槽 4 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ultimate, "InputTag.Ability.Ultimate", "龙斗士终极技能输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dodge, "InputTag.Ability.Dodge", "龙斗士闪避技能输入。");
		}
	}

	namespace Profession
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(DragonFighter, "Profession.DragonFighter", "龙斗士职业标识。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mage, "Profession.Mage", "法师职业标识。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Archer, "Profession.Archer", "弓箭手职业标识。");
	}

	namespace Message
	{
		namespace Combat
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(DamageApplied,      "Message.Combat.Damage.Applied",      "伤害事件 verb，Payload = FDOVerbMessage（Instigator / Target / Magnitude）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(EliminationFired,   "Message.Combat.Elimination.Fired",   "击杀事件 verb，Payload = FDOVerbMessage。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(AssistContributed, "Message.Combat.Assist.Contributed",  "助攻事件 verb，Payload = FDOVerbMessage，Magnitude = 助攻伤害贡献值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayReset,      "Message.Combat.GameplayReset",       "服务器权威重置事件 verb。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat,             "Message.Combat",                     "父频道，PartialMatch 用。");
		}

		namespace UI
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(RedDotChanged, "Message.UI.RedDot.Changed", "红点节点状态变化时广播，Payload = FDORedDotChangedMessage。");

			namespace Tutorial
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(StepChanged, "Message.UI.Tutorial.StepChanged", "引导步骤切换/进度变化时广播，Payload = FDOTutorialStepMessage。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Focus, "Message.UI.Tutorial.Focus", "请求高亮某个控件，Payload = FDOTutorialFocusMessage。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(ClearFocus, "Message.UI.Tutorial.ClearFocus", "清除高亮，Payload = FDOTutorialFocusMessage（bShow=false）。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Available, "Message.UI.Tutorial.Available", "存在可进行的新手引导时点亮入口红点。");
			}
		}
	}
}