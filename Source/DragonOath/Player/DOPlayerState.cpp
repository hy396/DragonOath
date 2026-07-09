#include "Player/DOPlayerState.h"

#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOPlaySet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlayerState)

ADOPlayerState::ADOPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UDOAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthSet = CreateDefaultSubobject<UDOHealthSet>(TEXT("HealthSet"));
	PlaySet = CreateDefaultSubobject<UDOPlaySet>(TEXT("PlaySet"));
}

UAbilitySystemComponent* ADOPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDOAbilitySystemComponent* ADOPlayerState::GetDOAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDOHealthSet* ADOPlayerState::GetHealthSet() const
{
	return HealthSet;
}

UDOPlaySet* ADOPlayerState::GetPlaySet() const
{
	return PlaySet;
}
