// DragonOath AttributeSet base.

#include "DOAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOAttributeSet)

class UWorld;

UDOAttributeSet::UDOAttributeSet()
{
}

UWorld* UDOAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UDOAbilitySystemComponent* UDOAttributeSet::GetUDOAbilitySystemComponent() const
{
	return Cast<UDOAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
