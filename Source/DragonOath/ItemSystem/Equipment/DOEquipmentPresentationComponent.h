#pragma once

#include "Components/ActorComponent.h"

#include "ItemSystem/Equipment/DOEquipmentAppearanceRegistry.h"
#include "ItemSystem/Equipment/DOEquipmentPresentationTypes.h"

#include "DOEquipmentPresentationComponent.generated.h"

class UDOEquipmentComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDOEquipmentPresentationChanged, const TArray<FGameplayTag>& /*ChangedSlotTags*/, int32 /*Revision*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDOEquipmentPresentationChangedDynamic, FGameplayTagContainer, ChangedSlotTags, int32, Revision);

/**
 * Pawn 级公开装备表现组件。
 *
 * 完整装备状态仍保存在 PlayerState 的 OwnerOnly EquipmentComponent；本组件只复制
 * 远端角色表现所需的槽位、外观 ID、变体和染色参数，并按 Pawn relevancy 同步。
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (DragonOath), meta = (BlueprintSpawnableComponent))
class DRAGONOATH_API UDOEquipmentPresentationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDOEquipmentPresentationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 服务器根据 PlayerState 装备快照重建公开摘要。 */
	void RebuildFromEquipment(const UDOEquipmentComponent& EquipmentComponent);

	/** 重生/重新 Possess 后由角色调用，确保服务器重新发布当前装备表现。 */
	void RebuildFromOwnerEquipment();

	UFUNCTION(BlueprintPure, Category = "DO|Equipment|Presentation")
	void GetPublicSnapshot(TArray<FDOEquipmentPublicEntry>& OutEntries) const;
	const FDOEquipmentPublicEntry* FindPublicEntry(const FGameplayTag& SlotTag) const;

	UFUNCTION(BlueprintPure, Category = "DO|Equipment|Presentation")
	bool GetPublicEntry(FGameplayTag SlotTag, FDOEquipmentPublicEntry& OutEntry) const;

	UFUNCTION(BlueprintPure, Category = "DO|Equipment|Presentation")
	UDOEquipmentAppearanceRegistry* GetAppearanceRegistry() const { return AppearanceRegistry; }

	UFUNCTION(BlueprintPure, Category = "DO|Equipment|Presentation")
	bool ResolveAppearance(FName AppearanceId, FGameplayTag VariantTag, FDOEquipmentAppearanceEntry& OutAppearance) const;
	int32 GetRevision() const { return Revision; }

	/** FastArray 复制完成后由容器调用。 */
	void HandleFastArrayChanged(const TArray<FGameplayTag>& ChangedSlotTags);

	FOnDOEquipmentPresentationChanged OnPresentationChanged;

	/** Blueprint-facing hook for character appearance components. */
	UPROPERTY(BlueprintAssignable, Category = "DO|Equipment|Presentation")
	FDOEquipmentPresentationChangedDynamic OnPresentationChangedBP;

protected:
	virtual void BeginPlay() override;

private:
	friend struct FDOEquipmentPublicList;

	void BroadcastChanged(const TArray<FGameplayTag>& ChangedSlotTags, bool bAdvanceRevision = false);
	static bool AreEntriesEqual(const FDOEquipmentPublicEntry& A, const FDOEquipmentPublicEntry& B);

	UPROPERTY(Replicated)
	int32 Revision = 0;

	UPROPERTY(Replicated)
	FDOEquipmentPublicList PublicEquipmentList;

	/** Static client-side mapping from public IDs to visual assets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Equipment|Appearance", meta = (AllowPrivateAccess = "true", DisplayName = "外观注册表"))
	TObjectPtr<UDOEquipmentAppearanceRegistry> AppearanceRegistry;
};
