#include "ItemSystem/Usage/DOItemUseEffects.h"

#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemUseEffects)

namespace
{
	/** 创建一个固定数值的即时属性修改器。 */
	void AddInstantModifier(UGameplayEffect& Effect, const FGameplayAttribute& Attribute, const float Magnitude)
	{
		FGameplayModifierInfo& Modifier = Effect.Modifiers.AddDefaulted_GetRef();
		Modifier.Attribute = Attribute;
		Modifier.ModifierOp = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Magnitude));
	}
}

UDOItemHealthPotionEffect::UDOItemHealthPotionEffect()
{
	// 回复通过 Healing Meta 进入 HealthSet，由服务器统一执行上限和死亡状态校验。
	DurationPolicy = EGameplayEffectDurationType::Instant;
	AddInstantModifier(*this, UDOHealthSet::GetHealingAttribute(), 50.0f);
}

UDOItemManaPotionEffect::UDOItemManaPotionEffect()
{
	// 法力回复直接修改当前 Mana，ResourceSet 会负责将结果限制在 MaxMana 内。
	DurationPolicy = EGameplayEffectDurationType::Instant;
	AddInstantModifier(*this, UDOResourceSet::GetManaAttribute(), 50.0f);
}
