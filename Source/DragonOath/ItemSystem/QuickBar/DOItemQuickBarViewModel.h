#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Object.h"

#include "DOItemQuickBarViewModel.generated.h"

class ADOPlayerState;
class UDOItemQuickBarComponent;
class UDOInventoryComponent;
class UTexture2D;

/** 快捷栏 HUD 使用的本地显示快照，不保存快捷栏权威数据副本。 */
struct DRAGONOATH_API FDOQuickBarSlotViewModel
{
	FPrimaryAssetId DefinitionId;
	FText DisplayName;
	FText Description;
	TSoftObjectPtr<UTexture2D> Icon;
	int32 StackCount = 0;
	bool bIsEmpty = true;

	/** 没有物品时仍保留 DefinitionId，便于显示“已绑定但当前数量为 0”。 */
	bool bIsBound = false;
	bool bIsPending = false;
};

DECLARE_MULTICAST_DELEGATE(FDOItemQuickBarViewModelChanged);

/**
 * 快捷栏 HUD 的 C++ ViewModel。
 *
 * 只读取 PlayerState 上的 QuickBar/Inventory 快照，按钮点击最终仍通过组件请求接口
 * 进入服务器权威流程。
 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOItemQuickBarViewModel : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ADOPlayerState* InPlayerState);
	void Shutdown();
	void Refresh();

	FDOItemQuickBarViewModelChanged& OnChanged() { return ChangedDelegate; }
	const TArray<TSharedPtr<FDOQuickBarSlotViewModel>>& GetSlots() const { return Slots; }

	void RequestUseSlot(int32 SlotIndex);

protected:
	virtual void BeginDestroy() override;

private:
	void HandleInventoryChanged(FGameplayTag Channel, const struct FDOInventoryChangedMessage& Message);
	void HandleQuickBarChanged(FGameplayTag Channel, const struct FDOItemQuickBarChangedMessage& Message);
	void HandleOperationFailed(FGameplayTag Channel, const struct FDOItemQuickBarOperationFailedMessage& Message);
	void HandleOperationResult(FGameplayTag Channel, const struct FDOItemQuickBarOperationResultMessage& Message);
	const class UDOItemDefinition* ResolveItemDefinition(const FPrimaryAssetId& DefinitionId) const;
	void BroadcastChanged();
	void ProcessPendingTimeouts();

	TWeakObjectPtr<ADOPlayerState> PlayerState;
	TWeakObjectPtr<UDOInventoryComponent> InventoryComponent;
	TWeakObjectPtr<UDOItemQuickBarComponent> QuickBarComponent;
	TArray<TSharedPtr<FDOQuickBarSlotViewModel>> Slots;
	TMap<int32, TSharedPtr<FDOQuickBarSlotViewModel>> SlotViewModelCache;
	int32 NextClientOperationId = 1;
	TMap<int32, int32> PendingOperations;
	TMap<int32, double> PendingOperationTimes;
	FTimerHandle PendingTimeoutTimerHandle;
	FDOItemQuickBarViewModelChanged ChangedDelegate;
	FGameplayMessageListenerHandle InventoryChangedHandle;
	FGameplayMessageListenerHandle QuickBarChangedHandle;
	FGameplayMessageListenerHandle OperationFailedHandle;
	FGameplayMessageListenerHandle OperationResultHandle;
};
