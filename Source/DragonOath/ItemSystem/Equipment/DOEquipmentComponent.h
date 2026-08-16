#pragma once

#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"

#include "ItemSystem/Equipment/DOEquipmentTypes.h"
#include "ItemSystem/Equipment/DOEquipmentInstance.h"
#include "ItemSystem/Core/DOItemOperationTypes.h"

#include "DOEquipmentComponent.generated.h"

class UGameplayEffect;
class UDOItemDefinition;
class UDOItemFragment_Equipment;
class UDOEquipmentLayout;

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
	void GetSupportedSlotTags(TArray<FGameplayTag>& OutSlotTags) const;

	/** 校验一份装备存档快照，不修改当前装备状态。 */
	bool ValidateEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries) const;

	/** 在服务器上恢复装备快照，并重新构建装备 GameplayEffect。 */
	bool RestoreEquippedSnapshot(const TArray<FDOEquippedItemEntry>& Entries);

	const FDOEquippedItemEntry* FindEquippedBySlot(const FGameplayTag& SlotTag) const;
	const UDOEquipmentInstance* FindEquipmentInstance(const FGameplayTag& SlotTag) const;

	/** 服务器将当前装备转换为 Pawn 级公开外观摘要；重生/重新 Possess 后可重复调用。 */
	void RebuildPublicPresentation();

	UFUNCTION(BlueprintPure, Category = "DO|Equipment")
	bool IsSlotEquipped(FGameplayTag SlotTag) const;

	UFUNCTION(BlueprintCallable, Category = "DO|Equipment")
	void RequestEquipItem(const FGuid& InstanceId, int32 ClientOperationId);

	UFUNCTION(BlueprintCallable, Category = "DO|Equipment")
	void RequestUnequipItem(FGameplayTag SlotTag, int32 ClientOperationId);

	int32 GetRevision() const { return Revision; }
	const UDOEquipmentLayout* GetEquipmentLayout() const { return EquipmentLayout; }

	/** 服务器更新已穿戴装备耐久；归零时撤销属性和装备技能，修复后自动重建。 */
	bool SetEquippedDurability(FGameplayTag SlotTag, int32 NewDurability);

	void HandleFastArrayChanged(const TArray<FGameplayTag>& ChangedSlotTags);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestEquipItem(const FGuid& InstanceId, int32 ClientOperationId);

	UFUNCTION(Server, Reliable)
	void Server_RequestUnequipItem(FGameplayTag SlotTag, int32 ClientOperationId);

	UFUNCTION(Client, Reliable)
	void Client_EquipmentOperationResult(int32 ClientOperationId, bool bSuccess, EDOInventoryFailureReason FailureReason);

	UFUNCTION(Client, Reliable)
	void Client_EquipmentOperationResultEx(int32 ClientOperationId, EDOItemOperationOutcome Outcome, EDOInventoryFailureReason FailureReason, int32 AuthoritativeRevision);

	/** 旧配置兼容入口。新方案默认使用 C++ 原生 UDOEquipmentAttributeEffect，迁移完成后删除。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Equipment|Legacy", meta = (DeprecatedProperty, DeprecationMessage = "装备属性 GE 已由 C++ 原生模板提供。", DisplayName = "旧版装备属性效果"))
	TSubclassOf<UGameplayEffect> EquipmentAttributeEffect;

private:
	friend struct FDOEquipmentList;

	const UDOItemDefinition* ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const;
	bool ValidateEquipment(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment*& OutFragment) const;
	bool IsEquipmentRuntimeActive(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment& Fragment) const;
	bool IsSlotConfigured(const FGameplayTag& SlotTag) const;
	bool ApplyEquipmentEffect(const FDOItemInstanceRecord& Item, const UDOItemFragment_Equipment& Fragment, UObject* SourceObject, FActiveGameplayEffectHandle& OutHandle);
	void RemoveEquipmentEffect(const FGameplayTag& SlotTag);
	bool RestoreEquipmentEffect(const FDOItemInstanceRecord& Item, const FGameplayTag& SlotTag);
	bool GrantEquipmentAbilities(UDOEquipmentInstance& Instance, const UDOItemFragment_Equipment& Fragment);
	void RemoveEquipmentInstanceFromAbilitySystem(UDOEquipmentInstance& Instance);
	void BroadcastChanged(const TArray<FGameplayTag>& ChangedSlotTags, bool bAdvanceRevision = true);
	void BroadcastOperationFailure(int32 ClientOperationId, EDOInventoryFailureReason FailureReason);
	void BroadcastOperationResult(int32 ClientOperationId, EDOItemOperationOutcome Outcome, EDOInventoryFailureReason FailureReason, int32 AuthoritativeRevision = INDEX_NONE);

	UPROPERTY(Replicated)
	int32 Revision = 0;

	UPROPERTY(Replicated)
	FDOEquipmentList EquipmentList;

	UPROPERTY(EditDefaultsOnly, Category = "DO|Equipment|Layout", meta = (DisplayName = "装备布局"))
	TObjectPtr<UDOEquipmentLayout> EquipmentLayout;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UDOEquipmentInstance>> EquipmentInstances;
};
