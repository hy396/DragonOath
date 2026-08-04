#pragma once

#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"

#include "ItemSystem/Equipment/DOEquipmentTypes.h"

#include "DOEquipmentComponent.generated.h"

class UGameplayEffect;

/**
 * 玩家装备组件。
 *
 * 完整装备实例只复制给 Owner。装备属性通过配置的通用 GameplayEffect
 * 以 SetByCaller 方式进入 PlayerState ASC；外观和服饰不属于本组件。
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (DragonOath), meta = (BlueprintSpawnableComponent))
class DRAGONOATH_API UDOEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDOEquipmentComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void GetEquippedSnapshot(TArray<FDOEquippedItemEntry>& OutEntries) const;

	/** 校验一份装备存档快照，不修改当前装备状态。 */
	bool ValidateEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries) const;

	/** 在服务器上恢复装备快照，并重新构建装备 GameplayEffect。 */
	bool RestoreEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries);

	const FDOEquippedItemEntry* FindEquippedBySlot(const FGameplayTag& SlotTag) const;

	UFUNCTION(BlueprintPure, Category = "DO|Equipment")
	bool IsSlotEquipped(FGameplayTag SlotTag) const;

	UFUNCTION(BlueprintCallable, Category = "DO|Equipment")
	void RequestEquipItem(const FGuid& InstanceId, int32 ClientOperationId);

	UFUNCTION(BlueprintCallable, Category = "DO|Equipment")
	void RequestUnequipItem(FGameplayTag SlotTag, int32 ClientOperationId);

	int32 GetRevision() const { return Revision; }

	void HandleFastArrayChanged(const TArray<FGameplayTag>& ChangedSlotTags);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestEquipItem(const FGuid& InstanceId, int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestUnequipItem(FGameplayTag SlotTag, int32 ClientOperationId);

	UFUNCTION(Client, Reliable)
	void Client_EquipmentOperationResult(int32 ClientOperationId, bool bSuccess, EDOInventoryFailureReason FailureReason);

	/** 旧配置兼容入口。新方案默认使用 C++ 原生 UDOEquipmentAttributeEffect，迁移完成后删除。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Equipment|Legacy", meta = (DeprecatedProperty, DeprecationMessage = "装备属性 GE 已由 C++ 原生模板提供。"))
	TSubclassOf<UGameplayEffect> EquipmentAttributeEffect;

private:
	friend struct FDOEquipmentList;

	const UDOItemDefinition* ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const;
	bool ValidateEquipment(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment*& OutFragment) const;
	bool ApplyEquipmentEffect(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment& Fragment, FActiveGameplayEffectHandle& OutHandle);
	void RemoveEquipmentEffect(const FGameplayTag& SlotTag);
	bool RestoreEquipmentEffect(const FDOItemInstanceRecord& Item, const FGameplayTag& SlotTag);
	void BroadcastChanged(const TArray<FGameplayTag>& ChangedSlotTags);
	void BroadcastOperationFailure(int32 ClientOperationId, EDOInventoryFailureReason FailureReason);

	UPROPERTY(Replicated)
	int32 Revision = 0;

	UPROPERTY(Replicated)
	FDOEquipmentList EquipmentList;

	TMap<FGameplayTag, FActiveGameplayEffectHandle> ActiveEquipmentEffects;
};
