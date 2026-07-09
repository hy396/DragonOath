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

	namespace InputTag
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Jump, "InputTag.Jump", "DragonOath 玩家角色使用的原生跳跃输入。");

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
}
