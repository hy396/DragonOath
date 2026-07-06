// DragonOath GAS AttributeSet base.
// 说明：统一属性访问器、ASC 获取方式，以及属性集通用事件类型。

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "DOAttributeSet.generated.h"

// 宏：简化属性访问器声明。每个 FGameplayAttributeData 属性都应配套这组 Getter/Setter/Init 函数。
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** 
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients: 
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FDOAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

/**
 * DragonOath 属性集基类。
 *
 * 项目里的具体属性集都继承这里，获得统一的 World/ASC 获取方式。
 * 注意：真正的数值规则应放在具体 AttributeSet、GameplayEffect 或 ExecutionCalculation 中。
 */
class UDOAbilitySystemComponent;

UCLASS()
class DRAGONOATH_API UDOAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UDOAttributeSet();

	// AttributeSet 本身不是 Actor，通过 Outer 链找到 ASC/Owner 所在的 World。
	UWorld* GetWorld() const override;

	// 统一返回项目 ASC，避免每个属性集里重复 Cast。
	UDOAbilitySystemComponent* GetUDOAbilitySystemComponent() const;
};
