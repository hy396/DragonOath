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

		// 装备属性通过 SetByCaller 写入通用装备 GameplayEffect。
		namespace Equipment
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttackPower);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DefensePower);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxHealth);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxMana);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CriticalRating);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitRating);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(EvasionRating);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttackSpeed);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MoveSpeed);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(LifeStealRate);
		}

		// 消耗品动态数值通过这些 SetByCaller 标签写入原生物品 GE。
		namespace ItemUse
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Healing);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ManaRestore);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StaminaRestore);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Duration);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CooldownDuration);
		}
	}

	// 物品分类、品质和装备部位标签。
	namespace Item
	{
		namespace Type
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consumable);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Material);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest);
		}

		namespace Category
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Armor);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Accessory);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Potion);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(EnhancementMaterial);
		}

		namespace Rarity
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Common);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Uncommon);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rare);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Epic);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Legendary);
		}
	}

	namespace Equipment
	{
		namespace Slot
		{
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Head);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shoulder);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Back);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chest);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hands);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Legs);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Feet);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Accessory);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon);
		}
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
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
		}
	}

	// ============================================================================
	// Ability.* 子树设计（"耐用性"说明）
	// ----------------------------------------------------------------------------
	// 本命名空间按"正交维度"组织，三个子树互不冲突、可独立扩展：
	//
	//   Ability.Id.*    —— 身份维度（你现在填的）
	//       每个技能恰好一个身份标签，作为 UI / 存档 / 技能树 / 运行时查找的"身份证"。
	//       保持稳定，不要为了"分类互斥"往这里塞 Attack / Movement 之类（见 Type）。
	//
	//   Ability.Type.*  —— 分类维度（未来）
	//       技能分类（Melee / Ranged / Movement / Buff 等），供
	//       CancelAbilitiesWithTag / BlockAbilitiesWithTag 按类互斥。
	//       对应 AGENTS.md 示例里提到的 Ability.Attack / Ability.Move.Dash，
	//       建议后续统一归到 Ability.Type.* 下更清晰，与身份标签解耦。
	//
	//   Ability.Tier.*  —— 阶位维度（未来）
	//       升级阶位（Tier.1 ~ Tier.N），配合技能树升级时改阶。
	//
	// 设计要点：身份是稳定的"点"，分类/阶位是可变的"面"，分维度存放可避免
	// 一个技能既当身份证又被分类逻辑误匹配（如 Cancel 掉所有 Dodge 类技能）。
	// ============================================================================
	namespace Ability
	{
		namespace Id
		{
			// TODO: 新增技能时，在此按 "Ability.Id.<技能名>" 追加身份标签。
			// 命名建议与技能蓝图 / InputTag.Ability.* 槽位对应（如 Ability.Id.PrimaryAttack 对应 InputTag.Ability.Primary）。
			//
			// 作用：作为该技能的"身份证"，供以下系统查找——
			//   - UI：技能图标 / 冷却 / 红点按 AbilityId 取数据
			//   - 存档：保存已学技能与等级（序列化用 AbilityId 而非类名，重命名蓝图不破坏旧存档）
			//   - 升级 / 技能树：SkillTreeComponent 按 AbilityId 定位前置条件与消耗
			//   - 运行时查找：ASC 按 AbilityId 反查已授予的 SpecHandle（中断 / 替换技能用）
			//
			// 注意：身份标签保持稳定，不要为了"分类互斥"往这里塞 Attack / Movement 之类，
			//       那类用途应放 Ability.Type.*（见上方说明），属于正交维度互不干扰。
			// TODO: 未来扩展维度（均挂在 Ability.* 下、与 Id 平级）：
			//   - Ability.Type.*：技能分类（Melee / Ranged / Movement / Buff），供 CancelAbilitiesWithTag / BlockAbilitiesWithTag 按类互斥
			//   - Ability.Tier.*：升级阶位（Tier.1 ~ Tier.N），配合技能树升级时改阶
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryAttack);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DashAttack);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill1);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill2);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill3);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill4);
			DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ultimate);
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
			namespace Layer
			{
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Menu);
			}

			namespace Inventory
			{
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Changed);
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OperationFailed);
			}

			namespace Equipment
			{
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Changed);
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OperationFailed);
			}

			namespace ItemQuickBar
			{
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Changed);
				DRAGONOATH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OperationFailed);
			}

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
