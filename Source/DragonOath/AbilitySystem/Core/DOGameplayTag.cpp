#include "AbilitySystem/Core/DOGameplayTag.h"

namespace DragonOathGameplayTags
{
	namespace Gameplay
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityInputBlocked, "Gameplay.AbilityInputBlocked", "Blocks ability input processing on the owning ASC.");
	}

	namespace Event
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death, "Event.Death", "Gameplay event sent when an actor reaches zero health.");
	}

	namespace InputTag
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Jump, "InputTag.Jump", "Native jump input used by DragonOath player characters.");

		namespace Ability
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Primary, "InputTag.Ability.Primary", "Primary dragon fighter ability input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary, "InputTag.Ability.Secondary", "Secondary dragon fighter ability input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill1, "InputTag.Ability.Skill1", "Dragon fighter skill slot 1 input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill2, "InputTag.Ability.Skill2", "Dragon fighter skill slot 2 input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill3, "InputTag.Ability.Skill3", "Dragon fighter skill slot 3 input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill4, "InputTag.Ability.Skill4", "Dragon fighter skill slot 4 input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ultimate, "InputTag.Ability.Ultimate", "Dragon fighter ultimate ability input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dodge, "InputTag.Ability.Dodge", "Dragon fighter dodge ability input.");
		}
	}
}
