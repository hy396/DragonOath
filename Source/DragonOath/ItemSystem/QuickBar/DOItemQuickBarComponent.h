#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"
#include "ItemSystem/Core/DOItemOperationTypes.h"

#include "DOItemQuickBarComponent.generated.h"

class UDOItemDefinition;

/** 玩家物品快捷栏，保存 DefinitionId 而不是易失的 InstanceId。 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (DragonOath), meta = (BlueprintSpawnableComponent))
class DRAGONOATH_API UDOItemQuickBarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDOItemQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static constexpr int32 QuickBarSlotCount = 4;

	UFUNCTION(BlueprintPure, Category = "DO|ItemQuickBar")
	FPrimaryAssetId GetDefinitionForSlot(int32 SlotIndex) const;
	void GetQuickBarSnapshot(TArray<FPrimaryAssetId>& OutDefinitions) const;
	bool RestoreQuickBarSnapshot(const TArray<FPrimaryAssetId>& Definitions);

	UFUNCTION(BlueprintCallable, Category = "DO|ItemQuickBar")
	void RequestAssignDefinition(int32 SlotIndex, FPrimaryAssetId DefinitionId, int32 ClientOperationId);

	UFUNCTION(BlueprintCallable, Category = "DO|ItemQuickBar")
	void RequestUseSlot(int32 SlotIndex, int32 ClientOperationId);

	int32 GetRevision() const { return Revision; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Reliable)
	void Server_RequestAssignDefinition(int32 SlotIndex, FPrimaryAssetId DefinitionId, int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestUseSlot(int32 SlotIndex, int32 ClientOperationId);

	UFUNCTION(Client, Reliable)
	void Client_QuickBarOperationResult(int32 ClientOperationId, bool bSuccess, EDOInventoryFailureReason FailureReason);

	UFUNCTION(Client, Reliable)
	void Client_QuickBarOperationResultEx(int32 ClientOperationId, EDOItemOperationOutcome Outcome, EDOInventoryFailureReason FailureReason, int32 AuthoritativeRevision);

private:
	const UDOItemDefinition* ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const;
	void HandleInventoryOperationResult(FGameplayTag Channel, const struct FDOInventoryOperationResultMessage& Message);
	void BroadcastChanged();
	void BroadcastOperationFailure(int32 ClientOperationId, EDOInventoryFailureReason FailureReason);
	void BroadcastOperationResult(int32 ClientOperationId, EDOItemOperationOutcome Outcome, EDOInventoryFailureReason FailureReason, int32 AuthoritativeRevision = INDEX_NONE);

	UPROPERTY(Replicated)
	int32 Revision = 0;

	UPROPERTY(ReplicatedUsing = OnRep_QuickBarDefinitions)
	TArray<FPrimaryAssetId> QuickBarDefinitions;

	/** 复杂消耗品由 Inventory 完成，QuickBar 仅转发同一 OperationId 的终态回执。 */
	TSet<int32> DelegatedUseOperationIds;
	FGameplayMessageListenerHandle InventoryOperationResultHandle;

	UFUNCTION()
	void OnRep_QuickBarDefinitions();
};
