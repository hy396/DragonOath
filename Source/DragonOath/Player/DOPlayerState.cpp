#include "Player/DOPlayerState.h"

#include "AbilitySystem/Abilities/DOAbilitySet.h"
#include "AbilitySystem/Abilities/DOProfessionAbilityConfig.h"
#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "DOLogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlayerState)

ADOPlayerState::ADOPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UDOAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthSet = CreateDefaultSubobject<UDOHealthSet>(TEXT("HealthSet"));
	ResourceSet = CreateDefaultSubobject<UDOResourceSet>(TEXT("ResourceSet"));
	CombatSet = CreateDefaultSubobject<UDOCombatSet>(TEXT("CombatSet"));
}

void ADOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADOPlayerState, ProfessionTag);
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

UDOResourceSet* ADOPlayerState::GetResourceSet() const
{
	return ResourceSet;
}

UDOCombatSet* ADOPlayerState::GetCombatSet() const
{
	return CombatSet;
}

void ADOPlayerState::SetProfession(FGameplayTag NewProfession)
{
	if (!HasAuthority())
	{
		return;
	}

	if (ProfessionTag == NewProfession)
	{
		return;
	}

	// 清除旧职业技能
	if (bProfessionAbilitiesGranted)
	{
		AbilitySystemComponent->ClearDOAbilities();
		bProfessionAbilitiesGranted = false;
	}

	ProfessionTag = NewProfession;

	// 授予新职业技能
	GrantProfessionAbilities();
}

void ADOPlayerState::GrantProfessionAbilities()
{
	if (bProfessionAbilitiesGranted)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (!ProfessionTag.IsValid())
	{
		UE_LOG(LogDragonOath, Warning, TEXT("GrantProfessionAbilities: ProfessionTag is not set."));
		return;
	}

	if (!ProfessionAbilityConfig)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("GrantProfessionAbilities: ProfessionAbilityConfig is not set."));
		return;
	}

	const TObjectPtr<UDOAbilitySet>* AbilitySetPtr = ProfessionAbilityConfig->ProfessionAbilitySets.Find(ProfessionTag);
	if (!AbilitySetPtr || !*AbilitySetPtr)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("GrantProfessionAbilities: No AbilitySet found for profession %s."), *ProfessionTag.ToString());
		return;
	}

	AbilitySystemComponent->GiveDOAbilitySet(*AbilitySetPtr);
	bProfessionAbilitiesGranted = true;

	UE_LOG(LogDragonOath, Log, TEXT("Granted profession abilities for %s."), *ProfessionTag.ToString());
}

void ADOPlayerState::OnRep_ProfessionTag()
{
	// 客户端可以在这里更新 UI、切换角色外观等
	// 后续可以委托给 UI 系统或 GameplayMessageRouter
}