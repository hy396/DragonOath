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

		namespace Equipment
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(AttackPower, "Data.Equipment.AttackPower", "装备提供的攻击力 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(DefensePower, "Data.Equipment.DefensePower", "装备提供的防御力 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxHealth, "Data.Equipment.MaxHealth", "装备提供的最大生命值 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxMana, "Data.Equipment.MaxMana", "装备提供的最大法力值 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(CriticalRating, "Data.Equipment.CriticalRating", "装备提供的暴击属性 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitRating, "Data.Equipment.HitRating", "装备提供的命中属性 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(EvasionRating, "Data.Equipment.EvasionRating", "装备提供的闪避属性 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(AttackSpeed, "Data.Equipment.AttackSpeed", "装备提供的攻击速度 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(MoveSpeed, "Data.Equipment.MoveSpeed", "装备提供的移动速度 SetByCaller 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(LifeStealRate, "Data.Equipment.LifeStealRate", "装备提供的吸血比例 SetByCaller 数值。");
		}

		namespace ItemUse
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Healing, "Data.ItemUse.Healing", "消耗品提供的生命回复 Meta Attribute 数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ManaRestore, "Data.ItemUse.ManaRestore", "消耗品提供的法力回复数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(StaminaRestore, "Data.ItemUse.StaminaRestore", "消耗品提供的体力回复数值。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Duration, "Data.ItemUse.Duration", "限时属性道具的持续时间。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(CooldownDuration, "Data.ItemUse.CooldownDuration", "消耗品公共冷却的持续时间。");
		}
	}

	namespace Item
	{
		namespace Type
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment, "Item.Type.Equipment", "装备类物品。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consumable, "Item.Type.Consumable", "消耗品类物品。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Material, "Item.Type.Material", "材料类物品。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest, "Item.Type.Quest", "任务类物品。");
		}

		namespace Category
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon, "Item.Category.Weapon", "武器分类。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Armor, "Item.Category.Armor", "防具分类。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Accessory, "Item.Category.Accessory", "饰品分类。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Potion, "Item.Category.Potion", "药水分类。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(EnhancementMaterial, "Item.Category.EnhancementMaterial", "强化材料分类。");
		}

		namespace Rarity
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Common, "Item.Rarity.Common", "普通品质。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Uncommon, "Item.Rarity.Uncommon", "优秀品质。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Rare, "Item.Rarity.Rare", "稀有品质。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Epic, "Item.Rarity.Epic", "史诗品质。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Legendary, "Item.Rarity.Legendary", "传说品质。");
		}
	}

	namespace Equipment
	{
		namespace Slot
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Head, "Equipment.Slot.Head", "头部装备槽。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Shoulder, "Equipment.Slot.Shoulder", "肩部装备槽。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Back, "Equipment.Slot.Back", "背部装备槽。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Chest, "Equipment.Slot.Chest", "胸部装备槽。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Hands, "Equipment.Slot.Hands", "手部装备槽。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Legs, "Equipment.Slot.Legs", "腿部装备槽。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feet, "Equipment.Slot.Feet", "脚部装备槽。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Accessory, "Equipment.Slot.Accessory", "饰品装备槽。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon, "Equipment.Slot.Weapon", "武器装备槽。");
	}

	namespace Appearance::Variant
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ruby, "Equipment.Appearance.Variant.Ruby", "红色外观变体。");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sapphire, "Equipment.Appearance.Variant.Sapphire", "蓝色外观变体。");
	}
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
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dash, "InputTag.Ability.Dash", "龙斗士冲刺技能输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary, "InputTag.Ability.Secondary", "龙斗士副技能输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill1, "InputTag.Ability.Skill1", "龙斗士技能槽 1 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill2, "InputTag.Ability.Skill2", "龙斗士技能槽 2 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill3, "InputTag.Ability.Skill3", "龙斗士技能槽 3 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill4, "InputTag.Ability.Skill4", "龙斗士技能槽 4 输入。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ultimate, "InputTag.Ability.Ultimate", "龙斗士终极技能输入。");
		}
	}

	namespace Ability
	{
		namespace Id
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(PrimaryAttack, "Ability.Id.PrimaryAttack", "普攻技能身份标识（AbilityId），用于 UI/存档/升级/技能树查找。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dash, "Ability.Id.Dash", "冲刺技能身份标识（AbilityId）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(DashAttack, "Ability.Id.DashAttack", "冲刺攻击技能身份标识（AbilityId）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill1, "Ability.Id.Skill1", "技能槽 1 身份标识（AbilityId）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill2, "Ability.Id.Skill2", "技能槽 2 身份标识（AbilityId）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill3, "Ability.Id.Skill3", "技能槽 3 身份标识（AbilityId）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill4, "Ability.Id.Skill4", "技能槽 4 身份标识（AbilityId）。");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ultimate, "Ability.Id.Ultimate", "终极技能身份标识（AbilityId）。");
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
			namespace Layer
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Menu, "UI.Layer.Menu", "背包等主动菜单页面所在的 CommonUI 层。");
			}

			namespace Inventory
			{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Changed, "Message.UI.Inventory.Changed", "背包数据在本地完成刷新后广播。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(OperationFailed, "Message.UI.Inventory.OperationFailed", "背包服务器操作失败后广播。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(OperationResult, "Message.UI.Inventory.OperationResult", "背包服务器操作完成、失败、无变化或取消后的统一回执。");
			}

			namespace Equipment
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Changed, "Message.UI.Equipment.Changed", "装备数据在本地完成刷新后广播。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(OperationFailed, "Message.UI.Equipment.OperationFailed", "装备请求被服务器拒绝后广播的失败消息。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(OperationResult, "Message.UI.Equipment.OperationResult", "装备服务器操作完成、失败、无变化或取消后的统一回执。");
			}

			namespace ItemQuickBar
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Changed, "Message.UI.ItemQuickBar.Changed", "物品快捷栏数据刷新后广播。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(OperationFailed, "Message.UI.ItemQuickBar.OperationFailed", "快捷栏请求被服务器拒绝后广播的失败消息。");
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(OperationResult, "Message.UI.ItemQuickBar.OperationResult", "快捷栏服务器操作完成、失败、无变化或取消后的统一回执。");
			}

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
