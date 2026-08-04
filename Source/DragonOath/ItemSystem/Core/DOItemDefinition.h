#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "UObject/SoftObjectPtr.h"

#include "ItemSystem/Core/DOItemAttributeTypes.h"

#include "DOItemDefinition.generated.h"


/** 所有物品 Fragment 的共同基类。Fragment 使用 Instanced 子对象保存类型专属配置。 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class DRAGONOATH_API UDOItemFragment : public UObject
{
	GENERATED_BODY()
};

/** 背包通用规则 Fragment。 */
UCLASS(BlueprintType, EditInlineNew)
class DRAGONOATH_API UDOItemFragment_Inventory : public UDOItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	bool bCanDiscard = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	bool bCanSell = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	bool bUnique = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	bool bBindOnPickup = false;
};

/** 装备专属 Fragment。属性由装备组件通过 SetByCaller 写入 GAS。 */
UCLASS(BlueprintType, EditInlineNew)
class DRAGONOATH_API UDOItemFragment_Equipment : public UDOItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag EquipmentSlotTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTagQuery RequiredProfessionQuery;

	/** 新方案：直接填写装备属性，服务器由 C++ Builder 统一转换成 SetByCaller。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ShowOnlyInnerProperties))
	FDOAttributeModifierValues AttributeModifiers;

	/** 旧方案兼容字段，迁移完成后删除。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Legacy", meta = (Categories = "Data.Equipment", DeprecatedProperty, DeprecationMessage = "请迁移到 AttributeModifiers。"))
	TMap<FGameplayTag, FScalableFloat> BaseAttributeMagnitudes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ClampMin = "0"))
	int32 MaxDurability = 0;
};

/** 消耗品专属 Fragment。简单物品使用原生 GE，复杂物品可以触发 GA/Event。 */
UCLASS(BlueprintType, EditInlineNew)
class DRAGONOATH_API UDOItemFragment_Consumable : public UDOItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	EDOConsumableEffectKind EffectKind = EDOConsumableEffectKind::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ShowOnlyInnerProperties, EditCondition = "EffectKind == EDOConsumableEffectKind::InstantRestore", EditConditionHides))
	FDOResourceRestoreValues InstantRestore;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ShowOnlyInnerProperties, EditCondition = "EffectKind == EDOConsumableEffectKind::TimedAttributeModifier", EditConditionHides))
	FDOItemTimedModifierValues TimedModifier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (EditCondition = "EffectKind == EDOConsumableEffectKind::GameplayAbility", EditConditionHides))
	TSubclassOf<UGameplayAbility> UseGameplayAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (Categories = "Event", EditCondition = "EffectKind == EDOConsumableEffectKind::GameplayEvent", EditConditionHides))
	FGameplayTag UseEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ShowOnlyInnerProperties))
	FDOItemCooldownConfig Cooldown;

	/** 旧方案兼容字段：普通效果迁移到 EffectKind + 类型化结构体。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Legacy", meta = (DeprecatedProperty, DeprecationMessage = "请迁移到 EffectKind 和类型化效果字段。"))
	TSubclassOf<UGameplayEffect> UseGameplayEffect;

	/** 旧方案兼容字段：仅用于读取插件或旧资产配置的公共冷却 Tag。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Legacy", meta = (Categories = "Cooldown", DeprecatedProperty, DeprecationMessage = "请迁移到 Cooldown。"))
	FGameplayTag SharedCooldownTag;
};

/** 物品静态定义。运行时只复制该资产的 PrimaryAssetId，不复制文本和资源指针。 */
UCLASS(BlueprintType, Blueprintable)
class DRAGONOATH_API UDOItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type", meta = (Categories = "Item.Type"))
	FGameplayTag ItemType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type", meta = (Categories = "Item.Rarity"))
	FGameplayTag Rarity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type", meta = (Categories = "Item"))
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules")
	int32 SortPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules", meta = (ClampMin = "0"))
	int32 SellPrice = 0;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Item|Fragments")
	TArray<TObjectPtr<UDOItemFragment>> Fragments;

	template <typename FragmentType>
	const FragmentType* FindFragment() const
	{
		for (const TObjectPtr<UDOItemFragment>& Fragment : Fragments)
		{
			if (const FragmentType* TypedFragment = Cast<FragmentType>(Fragment))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}
};
