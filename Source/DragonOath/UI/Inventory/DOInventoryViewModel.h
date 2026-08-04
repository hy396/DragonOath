#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Object.h"

#include "ItemSystem/Inventory/DOInventoryTypes.h"

#include "DOInventoryViewModel.generated.h"

class ADOPlayerState;
class UDOAbilitySystemComponent;
class UDOEquipmentComponent;
class UDOInventoryComponent;
class UDOItemDefinition;
class UDOItemQuickBarComponent;
class UDOInventoryPreviewComponent;
class UTextureRenderTarget2D;

/** 单个 Slate 物品格使用的本地显示快照，不保存权威组件指针。 */
struct DRAGONOATH_API FDOInventorySlotViewModel
{
	FDOItemInstanceRecord Item;
	FText DisplayName;
	FText Description;
	TSoftObjectPtr<UTexture2D> Icon;
	FGameplayTag ItemType;
	FGameplayTag Rarity;
	FGameplayTag EquipmentSlotTag;
	bool bIsEmpty = true;
	bool bIsEquipped = false;
	bool bIsUsable = false;
	bool bCanDiscard = true;
	bool bIsPending = false;

	FGuid GetInstanceId() const { return Item.InstanceId; }
	int32 GetSlotIndex() const { return Item.SlotIndex; }
};

/** 背包分类按钮的本地配置。 */
struct DRAGONOATH_API FDOInventoryCategoryOption
{
	FGameplayTag FilterTag;
	FText DisplayName;
};

/** 从 ASC 读取的最终属性快照，只用于背包页面显示和战力推荐值。 */
struct DRAGONOATH_API FDOInventoryAttributeSnapshot
{
	float AttackPower = 0.0f;
	float DefensePower = 0.0f;
	float MaxHealth = 0.0f;
	float MaxMana = 0.0f;
	float CriticalRating = 0.0f;
	float HitRating = 0.0f;
	float EvasionRating = 0.0f;
	float AttackSpeed = 0.0f;
	float MoveSpeed = 0.0f;
	float LifeStealRate = 0.0f;
	int32 CombatPower = 0;
	int32 GuardPower = 0;
};

DECLARE_MULTICAST_DELEGATE(FDOInventoryViewModelChanged);

/** ViewModel 记录的一次客户端请求，用于在成功刷新或失败回包后清理 Pending 状态。 */
enum class EDOInventoryPendingDomain : uint8
{
	Inventory,
	Equipment,
	QuickBar
};

struct FDOInventoryPendingOperation
{
	EDOInventoryPendingDomain Domain = EDOInventoryPendingDomain::Inventory;
	FGuid InstanceId;
	FGameplayTag SlotTag;
};

/**
 * 背包 Slate 的 C++ ViewModel。
 *
 * ViewModel 只做快照、筛选、分页和操作请求编排，不修改 FastArray 的真实数据。
 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOInventoryViewModel : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ADOPlayerState* InPlayerState);
	void Shutdown();
	void Refresh();

	FDOInventoryViewModelChanged& OnChanged() { return ChangedDelegate; }

	const TArray<TSharedPtr<FDOInventorySlotViewModel>>& GetVisibleSlots() const { return VisibleSlots; }
	TArray<TSharedPtr<FDOInventorySlotViewModel>>& GetVisibleSlotsMutable() { return VisibleSlots; }
	const TArray<FDOInventoryCategoryOption>& GetCategories() const { return Categories; }
	const FDOInventoryAttributeSnapshot& GetAttributeSnapshot() const { return AttributeSnapshot; }

	UDOInventoryComponent* GetInventoryComponent() const { return InventoryComponent.Get(); }
	UDOEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent.Get(); }
	ADOPlayerState* GetPlayerState() const { return PlayerState.Get(); }
	UTextureRenderTarget2D* GetPreviewRenderTarget() const;

	FGameplayTag GetCurrentFilter() const { return CurrentFilter; }
	int32 GetCurrentPage() const { return CurrentPage; }
	int32 GetPageCount() const { return PageCount; }
	const FGuid& GetSelectedInstanceId() const { return SelectedInstanceId; }

	void SetFilter(FGameplayTag NewFilter);
	void SetPage(int32 NewPage);
	void SelectInstance(const FGuid& InstanceId);

	void RequestMoveOrEquip(const FGuid& InstanceId, int32 SourceSlot, int32 TargetSlot);
	void RequestSplitStack(const FGuid& InstanceId, int32 TargetSlot, int32 SplitCount);
	/** 使用当前选中的消耗品。 */
	void RequestActivateSelected();
	/** 请求装备指定实例，供装备槽拖放等不依赖当前选择状态的交互使用。 */
	void RequestEquipInstance(const FGuid& InstanceId);
	/** 将当前选中的物品定义绑定到指定快捷栏槽位。 */
	void RequestAssignSelectedToQuickBar(int32 QuickBarSlot);
	void RequestEquipSelected();
	void RequestUnequip(FGameplayTag SlotTag);
	void RequestSort();
	void RequestDiscardSelected();
	void RequestDiscardSelectedCount(int32 Count);

	FText GetSelectedDisplayName() const;
	FText GetSelectedDescription() const;
	int32 GetSelectedStackCount() const;

	const FDOInventorySlotViewModel* FindVisibleSlot(const FGuid& InstanceId) const;

protected:
	virtual void BeginDestroy() override;

private:
	void HandleInventoryChanged(FGameplayTag Channel, const struct FDOInventoryChangedMessage& Message);
	void HandleEquipmentChanged(FGameplayTag Channel, const struct FDOEquipmentChangedMessage& Message);
	void HandleOperationFailed(FGameplayTag Channel, const struct FDOInventoryOperationFailedMessage& Message);
	void HandleQuickBarChanged(FGameplayTag Channel, const struct FDOItemQuickBarChangedMessage& Message);
	void HandleEquipmentOperationFailed(FGameplayTag Channel, const struct FDOEquipmentOperationFailedMessage& Message);
	void HandleQuickBarOperationFailed(FGameplayTag Channel, const struct FDOItemQuickBarOperationFailedMessage& Message);
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void RegisterAttributeListeners();
	void UnregisterAttributeListeners();
	void RefreshAttributeSnapshot();
	const UDOItemDefinition* ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const;
	bool MatchesFilter(const FDOItemInstanceRecord& Item, const UDOItemDefinition* Definition) const;
	void RebuildVisibleSlots();
	void BroadcastChanged();
	int32 BeginPendingOperation(EDOInventoryPendingDomain Domain, const FGuid& InstanceId = FGuid(), const FGameplayTag& SlotTag = FGameplayTag());
	void ClearPendingOperation(int32 ClientOperationId);
	void ClearPendingOperationsForDomain(EDOInventoryPendingDomain Domain);

	TWeakObjectPtr<ADOPlayerState> PlayerState;
	TWeakObjectPtr<UDOInventoryComponent> InventoryComponent;
	TWeakObjectPtr<UDOEquipmentComponent> EquipmentComponent;
	TWeakObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<UDOInventoryPreviewComponent> PreviewComponent;

	TArray<FDOInventoryCategoryOption> Categories;
	TArray<TSharedPtr<FDOInventorySlotViewModel>> VisibleSlots;
	FDOInventoryAttributeSnapshot AttributeSnapshot;
	TArray<TPair<FGameplayAttribute, FDelegateHandle>> AttributeChangeHandles;

	FGameplayTag CurrentFilter;
	FGuid SelectedInstanceId;
	int32 CurrentPage = 0;
	int32 PageCount = 1;
	int32 NextClientOperationId = 1;
	static constexpr int32 PageSize = 20;
	TMap<int32, FDOInventoryPendingOperation> PendingOperations;

	FDOInventoryViewModelChanged ChangedDelegate;
	FGameplayMessageListenerHandle InventoryChangedHandle;
	FGameplayMessageListenerHandle EquipmentChangedHandle;
	FGameplayMessageListenerHandle OperationFailedHandle;
	FGameplayMessageListenerHandle QuickBarChangedHandle;
	FGameplayMessageListenerHandle EquipmentOperationFailedHandle;
	FGameplayMessageListenerHandle QuickBarOperationFailedHandle;
};
