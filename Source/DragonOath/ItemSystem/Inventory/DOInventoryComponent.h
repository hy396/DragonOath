#pragma once

#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"
#include "ItemSystem/Core/DOItemOperationTypes.h"

#include "DOInventoryComponent.generated.h"

class UDOItemDefinition;
class UDOItemFragment_Consumable;
class UDOInventorySortConfig;

/**
 * 玩家背包运行时组件。
 *
 * 组件挂在 PlayerState 上，服务器是唯一修改方；客户端只持有 Owner-only
 * FastArray 的复制结果，并通过 ViewModel 查询显示快照。
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (DragonOath), meta = (BlueprintSpawnableComponent))
class DRAGONOATH_API UDOInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDOInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "DO|Inventory")
	int32 GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintPure, Category = "DO|Inventory")
	int32 GetRevision() const { return Revision; }

	UFUNCTION(BlueprintPure, Category = "DO|Inventory")
	int32 GetUsedSlotCount() const { return InventoryList.Entries.Num(); }

	void SetSortConfig(UDOInventorySortConfig* InSortConfig) { SortConfig = InSortConfig; }
	const UDOInventorySortConfig* GetSortConfig() const { return SortConfig; }

	/** 返回当前复制快照，UI 只应读取副本，不保存 Entry 指针。 */
	void GetInventorySnapshot(TArray<FDOItemInstanceRecord>& OutItems) const;

	const FDOItemInstanceRecord* FindItemByInstanceId(const FGuid& InstanceId) const;
	FDOItemInstanceRecord* FindItemByInstanceId(const FGuid& InstanceId);

	/** 服务器拾取系统使用的添加入口。 */
	FDOInventoryAddResult TryAddItem(const FPrimaryAssetId& DefinitionId, int32 Count);

	/** 服务器消费指定实例的数量。 */
	bool TryConsumeItem(const FGuid& InstanceId, int32 Count, EDOInventoryFailureReason& OutFailureReason);

	/** 服务器按 DefinitionId 消费最靠前的有效堆栈。 */
	bool TryConsumeByDefinition(const FPrimaryAssetId& DefinitionId, int32 Count, EDOInventoryFailureReason& OutFailureReason);

	/** 供装备事务使用：删除完整实例并返回原始记录。 */
	bool TryRemoveItemByInstanceId(const FGuid& InstanceId, FDOItemInstanceRecord& OutRemovedItem, EDOInventoryFailureReason& OutFailureReason);

	/** 供装备事务使用：把一个已有实例放回普通背包。 */
	bool CanInsertExistingItem(const FDOItemInstanceRecord& Item, EDOInventoryFailureReason& OutFailureReason) const;

	/** 供装备事务使用：把一个已有实例放回普通背包。 */
	bool TryInsertExistingItem(const FDOItemInstanceRecord& Item, EDOInventoryFailureReason& OutFailureReason);

	/** 服务器调试或掉落系统调用的整理入口。 */
	bool TrySortInventory(EDOInventoryFailureReason& OutFailureReason);

	/** 校验一份存档快照，但不修改当前背包。 */
	bool ValidateInventorySnapshot(const TArray<FDOItemInstanceRecord>& Items, int32 InCapacity) const;

	/** 服务端恢复存档快照，并触发一次本地刷新消息。 */
	bool RestoreInventorySnapshot(const TArray<FDOItemInstanceRecord>& Items, int32 InCapacity);

	/** 服务端执行一次消耗品使用，并在成功后扣除一个物品。 */
	bool TryUseItemByInstanceId(const FGuid& InstanceId, EDOInventoryFailureReason& OutFailureReason, int32 ClientOperationId = 0, bool* bOutDeferredCompletion = nullptr);

	/** 快捷栏按物品定义查找并使用一个消耗品。 */
	bool TryUseItemByDefinition(const FPrimaryAssetId& DefinitionId, EDOInventoryFailureReason& OutFailureReason, int32 ClientOperationId = 0, bool* bOutDeferredCompletion = nullptr);

	/** 复杂道具 Ability/Event 在最终提交点调用的服务器接口。 */
	bool CommitConsumableUse(const FGuid& InstanceId, const FPrimaryAssetId& ExpectedDefinitionId, EDOInventoryFailureReason& OutFailureReason, int32 ClientOperationId = 0);

	void CancelConsumableUse(int32 ClientOperationId);

	/** UI / C++ ViewModel 请求接口，实际操作在服务器执行。 */
	UFUNCTION(BlueprintCallable, Category = "DO|Inventory")
	void RequestMoveItem(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot, int32 RequestedCount, int32 ClientOperationId);

	UFUNCTION(BlueprintCallable, Category = "DO|Inventory")
	void RequestSplitStack(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot, int32 SplitCount, int32 ClientOperationId);

	UFUNCTION(BlueprintCallable, Category = "DO|Inventory")
	void RequestSortInventory(int32 ClientOperationId);

	UFUNCTION(BlueprintCallable, Category = "DO|Inventory")
	void RequestDiscardItem(const FGuid& InstanceId, int32 Count, int32 ClientOperationId);

	/** 客户端请求使用物品，真正的效果应用和扣除由服务器执行。 */
	UFUNCTION(BlueprintCallable, Category = "DO|Inventory")
	void RequestUseItem(const FGuid& InstanceId, int32 ClientOperationId);

	/** FastArray 回调入口。由列表聚合一帧内所有变化后调用一次。 */
	void HandleFastArrayChanged(const TArray<FGuid>& ChangedInstanceIds);

	/** 跨组件服务器事务使用：暂存 Changed，结束时只发布一次 Revision。 */
	void BeginMutation();
	void EndMutation();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestMoveItem(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot, int32 RequestedCount, int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestSplitStack(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot, int32 SplitCount, int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestSortInventory(int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestDiscardItem(const FGuid& InstanceId, int32 Count, int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestUseItem(const FGuid& InstanceId, int32 ClientOperationId);

	UFUNCTION(Client, Reliable)
	void Client_InventoryOperationResult(int32 ClientOperationId, bool bSuccess, EDOInventoryFailureReason FailureReason);

	UFUNCTION(Client, Reliable)
	void Client_InventoryOperationResultEx(int32 ClientOperationId, EDOItemOperationOutcome Outcome, EDOInventoryFailureReason FailureReason, int32 AuthoritativeRevision);

private:
	friend struct FDOInventoryList;

	UDOItemDefinition* ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const;

	bool TryMoveItemInternal(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot, int32 RequestedCount, EDOInventoryFailureReason& OutFailureReason, TArray<FGuid>& OutChangedIds);
	bool TrySplitStackInternal(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot, int32 SplitCount, EDOInventoryFailureReason& OutFailureReason, TArray<FGuid>& OutChangedIds);
	bool TryUseItemInternal(const FDOItemInstanceRecord& Item, EDOInventoryFailureReason& OutFailureReason, int32 ClientOperationId = 0, bool* bOutDeferredCompletion = nullptr);
	bool CanUseConsumable(const FDOItemInstanceRecord& Item, const UDOItemFragment_Consumable& Fragment, EDOInventoryFailureReason& OutFailureReason) const;
	bool ApplyDirectConsumableEffect(const FDOItemInstanceRecord& Item, const UDOItemFragment_Consumable& Fragment, FActiveGameplayEffectHandle& OutPersistentEffectHandle, EDOInventoryFailureReason& OutFailureReason);
	bool ApplyConsumableCooldown(const UDOItemFragment_Consumable& Fragment, FActiveGameplayEffectHandle& OutCooldownHandle, EDOInventoryFailureReason& OutFailureReason);
	bool BeginComplexConsumableUse(const FDOItemInstanceRecord& Item, const UDOItemFragment_Consumable& Fragment, EDOInventoryFailureReason& OutFailureReason, int32 ClientOperationId);
	bool IsValidSlot(int32 SlotIndex) const;
	int32 FindEntryIndexBySlot(int32 SlotIndex) const;
	int32 FindEmptySlot() const;
	void MarkEntryDirty(FDOInventoryEntry& Entry);
	void BroadcastOperationFailure(int32 ClientOperationId, EDOInventoryFailureReason FailureReason);
	void BroadcastOperationResult(int32 ClientOperationId, EDOItemOperationOutcome Outcome, EDOInventoryFailureReason FailureReason, int32 AuthoritativeRevision = INDEX_NONE);
	void BroadcastChanged(const TArray<FGuid>& ChangedInstanceIds, bool bAdvanceRevision = true);

	UPROPERTY(Replicated)
	int32 Capacity = 40;

	UPROPERTY(Replicated)
	int32 Revision = 0;

	UPROPERTY(Replicated)
	FDOInventoryList InventoryList;

	/** 可选排序权重资产；为空时使用项目默认权重和 Definition SortPriority。 */
	UPROPERTY(EditDefaultsOnly, Category = "DO|Inventory|Sort", meta = (DisplayName = "排序配置"))
	TObjectPtr<UDOInventorySortConfig> SortConfig;

	TSet<int32> PendingComplexConsumableOperations;
	int32 MutationDepth = 0;
	TArray<FGuid> DeferredChangedInstanceIds;
	bool bDeferredMutationAdvancesRevision = false;
};
