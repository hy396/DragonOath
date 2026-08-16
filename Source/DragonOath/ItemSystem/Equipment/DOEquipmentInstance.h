#pragma once

#include "CoreMinimal.h"

#include "AbilitySystem/Abilities/Core/DOAbilitySet.h"
#include "ItemSystem/Core/DOItemInstanceTypes.h"

#include "DOEquipmentInstance.generated.h"

class UDOAbilitySystemComponent;

/** 服务器侧单件穿戴装备的运行时生命周期对象，不复制、不存档。 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FDOItemInstanceRecord& InItem, const FGameplayTag& InSlotTag);
	/** 更新实例快照但保留当前 GAS 句柄；用于耐久仍处于同一激活区间时的轻量更新。 */
	void UpdateItemRuntimeState(const FDOItemInstanceRecord& InItem);

	const FDOItemInstanceRecord& GetItem() const { return Item; }
	const FGameplayTag& GetSlotTag() const { return SlotTag; }

	void SetAttributeEffectHandle(const FActiveGameplayEffectHandle& InHandle) { AttributeEffectHandle = InHandle; }
	const FActiveGameplayEffectHandle& GetAttributeEffectHandle() const { return AttributeEffectHandle; }

	FDOAbilitySetGrantedHandles& GetGrantedHandles() { return GrantedHandles; }
	const FDOAbilitySetGrantedHandles& GetGrantedHandles() const { return GrantedHandles; }

private:
	UPROPERTY()
	FDOItemInstanceRecord Item;

	UPROPERTY()
	FGameplayTag SlotTag;

	FActiveGameplayEffectHandle AttributeEffectHandle;
	FDOAbilitySetGrantedHandles GrantedHandles;
};
