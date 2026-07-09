#include "Characters/DOCharacter.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

UAbilitySystemComponent* ADOCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDOAbilitySystemComponent* ADOCharacter::GetDOAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADOCharacter::InitializeAbilitySystem()
{
	// 怪物或 NPC 可以直接把 UDOAbilitySystemComponent 挂在自身 Actor 上。
	if (UDOAbilitySystemComponent* OwnedASC = FindComponentByClass<UDOAbilitySystemComponent>())
	{
		InitializeAbilitySystemComponent(OwnedASC, this);
	}
}

void ADOCharacter::InitializeAbilitySystemComponent(UDOAbilitySystemComponent* InAbilitySystemComponent, AActor* InOwnerActor)
{
	if (!InAbilitySystemComponent || !InOwnerActor)
	{
		return;
	}

	AbilitySystemComponent = InAbilitySystemComponent;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, this);
}
