#include "ItemSystem/Pickup/DOItemPickup.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/DOPlayerCharacter.h"
#include "Player/DOPlayerController.h"
#include "Player/DOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemPickup)

ADOItemPickup::ADOItemPickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(PickupCollision);
	PickupCollision->InitSphereRadius(80.0f);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ADOItemPickup::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &ADOItemPickup::HandleBeginOverlap);
	}
}

void ADOItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADOItemPickup, ItemDefinitionId);
	DOREPLIFETIME(ADOItemPickup, RemainingCount);
}

void ADOItemPickup::SetPickupData(const FPrimaryAssetId InDefinitionId, const int32 InCount)
{
	if (!HasAuthority())
	{
		return;
	}

	ItemDefinitionId = InDefinitionId;
	RemainingCount = FMath::Max(0, InCount);
	if (RemainingCount == 0)
	{
		Destroy();
	}
}

void ADOItemPickup::RequestPickup()
{
	if (UWorld* World = GetWorld())
	{
		// Pickup Actor 通常不属于某个客户端，不能在 Pickup 自身上接收客户端 RPC。
		// 将请求转交给拥有连接的 PlayerController，由服务器从 RPC 调用者读取 Pawn。
		if (ADOPlayerController* PlayerController = Cast<ADOPlayerController>(World->GetFirstPlayerController()))
		{
			PlayerController->RequestPickupItem(this);
		}
	}
}

void ADOItemPickup::HandleBeginOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	const int32 /*OtherBodyIndex*/,
	const bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	if (HasAuthority())
	{
		TryPickupForActor(OtherActor);
	}
}

bool ADOItemPickup::TryPickupForActor(AActor* OtherActor)
{
	if (!HasAuthority() || RemainingCount <= 0 || !ItemDefinitionId.IsValid() || !OtherActor)
	{
		return false;
	}

	if (FVector::DistSquared(GetActorLocation(), OtherActor->GetActorLocation()) > FMath::Square(PickupInteractionDistance))
	{
		return false;
	}

	ADOPlayerCharacter* Character = Cast<ADOPlayerCharacter>(OtherActor);
	ADOPlayerState* PlayerState = Character ? Character->GetPlayerState<ADOPlayerState>() : nullptr;
	UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	const FDOInventoryAddResult Result = Inventory->TryAddItem(ItemDefinitionId, RemainingCount);
	if (Result.AddedCount <= 0)
	{
		return false;
	}

	RemainingCount = Result.RemainingCount;
	if (RemainingCount <= 0)
	{
		Destroy();
	}
	else
	{
		ForceNetUpdate();
	}
	return true;
}
