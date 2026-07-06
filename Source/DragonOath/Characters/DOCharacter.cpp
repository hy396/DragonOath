#include "Characters/DOCharacter.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/DOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOCharacter)

ADOCharacter::ADOCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MoveComp->JumpZVelocity = 600.0f;
	MoveComp->AirControl = 0.35f;
	MoveComp->MaxWalkSpeed = 500.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void ADOCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeAbilitySystem();
}

void ADOCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void ADOCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

UAbilitySystemComponent* ADOCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDOAbilitySystemComponent* ADOCharacter::GetDOAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

ADOPlayerState* ADOCharacter::GetDOPlayerState() const
{
	return GetPlayerState<ADOPlayerState>();
}

void ADOCharacter::InitializeAbilitySystem()
{
	ADOPlayerState* DOPlayerState = GetDOPlayerState();
	if (!DOPlayerState)
	{
		return;
	}

	UDOAbilitySystemComponent* DOASC = DOPlayerState->GetDOAbilitySystemComponent();
	if (!DOASC)
	{
		return;
	}

	AbilitySystemComponent = DOASC;
	AbilitySystemComponent->InitAbilityActorInfo(DOPlayerState, this);
}
