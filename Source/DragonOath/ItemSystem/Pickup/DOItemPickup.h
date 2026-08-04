#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "DOItemPickup.generated.h"

class APlayerController;
class USphereComponent;
class ADOPlayerController;

/**
 * 服务器权威的世界物品拾取 Actor。
 *
 * 世界掉落只复制物品定义 ID 和剩余数量；真正进入背包前仍由服务器调用
 * UDOInventoryComponent::TryAddItem，背包满时保留没有拾取的剩余数量。
 */
UCLASS(BlueprintType, Blueprintable)
class DRAGONOATH_API ADOItemPickup : public AActor
{
	GENERATED_BODY()

	friend class ADOPlayerController;

public:
	ADOItemPickup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "DO|Inventory|Pickup")
	void RequestPickup();

	UFUNCTION(BlueprintCallable, Category = "DO|Inventory|Pickup")
	void SetPickupData(FPrimaryAssetId InDefinitionId, int32 InCount);

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "DO|Inventory|Pickup")
	FPrimaryAssetId ItemDefinitionId;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "DO|Inventory|Pickup", meta = (ClampMin = "1"))
	int32 RemainingCount = 1;

	/** 手动拾取请求允许的最大距离；自动重叠拾取也经过同一项服务器校验。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Inventory|Pickup", meta = (ClampMin = "1.0"))
	float PickupInteractionDistance = 200.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool TryPickupForActor(AActor* OtherActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DO|Inventory|Pickup")
	TObjectPtr<USphereComponent> PickupCollision;
};
