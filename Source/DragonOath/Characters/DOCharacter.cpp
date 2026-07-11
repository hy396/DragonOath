#include "Characters/DOCharacter.h"

#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"
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

	// 创建属性集，供挂载在自身 ASC 的角色（怪物 / NPC）自动注册。
	// 玩家 Pawn 也会创建，但其 ASC 挂在 PlayerState，这些实例不参与玩家 ASC 注册。
	HealthSet = CreateDefaultSubobject<UDOHealthSet>(TEXT("HealthSet"));
	ResourceSet = CreateDefaultSubobject<UDOResourceSet>(TEXT("ResourceSet"));
	CombatSet = CreateDefaultSubobject<UDOCombatSet>(TEXT("CombatSet"));
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

	// ASC 初始化后绑定属性变化委托
	BindAttributeChangeDelegates();
}

void ADOCharacter::BindAttributeChangeDelegates()
{
	if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
	{
		// MoveSpeed 属性变化时同步到 MaxWalkSpeed（属性现位于 DOCombatSet）
		DOASC->GetGameplayAttributeValueChangeDelegate(
			UDOCombatSet::GetMoveSpeedAttribute()
		).AddUObject(this, &ADOCharacter::OnMoveSpeedChanged);
	}
}

void ADOCharacter::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	// MoveSpeed 属性值就是最终移动速度，直接写入
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}
