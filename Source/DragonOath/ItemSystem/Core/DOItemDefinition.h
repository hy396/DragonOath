#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "UObject/SoftObjectPtr.h"

#include "ItemSystem/Core/DOItemAttributeTypes.h"

class UDOAbilitySet;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "可丢弃"))
	bool bCanDiscard = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "可出售"))
	bool bCanSell = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "唯一物品"))
	bool bUnique = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "拾取绑定"))
	bool bBindOnPickup = false;
};

/** 装备专属 Fragment。属性由装备组件通过 SetByCaller 写入 GAS。 */
UCLASS(BlueprintType, EditInlineNew)
class DRAGONOATH_API UDOItemFragment_Equipment : public UDOItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot", DisplayName = "装备槽位"))
	FGameplayTag EquipmentSlotTag;

	/** 可选的槽位兼容查询；为空时使用 EquipmentSlotTag 的精确匹配。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot", DisplayName = "兼容槽位查询"))
	FGameplayTagQuery CompatibleSlotQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ClampMin = "1", DisplayName = "需求等级"))
	int32 RequiredLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (DisplayName = "职业需求查询"))
	FGameplayTagQuery RequiredProfessionQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|GAS", meta = (DisplayName = "装备技能集"))
	TObjectPtr<UDOAbilitySet> EquipmentAbilitySet;

	/** 新方案：直接填写装备属性，服务器由 C++ Builder 统一转换成 SetByCaller。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ShowOnlyInnerProperties, DisplayName = "属性修正"))
	FDOAttributeModifierValues AttributeModifiers;

	/** 旧方案兼容字段，迁移完成后删除。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Legacy", meta = (Categories = "Data.Equipment", DeprecatedProperty, DeprecationMessage = "请迁移到 AttributeModifiers。", DisplayName = "旧版属性数值"))
	TMap<FGameplayTag, FScalableFloat> BaseAttributeMagnitudes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ClampMin = "0", DisplayName = "最大耐久"))
	int32 MaxDurability = 0;
};

/**
 * 装备公开表现 Fragment。
 *
 * 该数据只描述远端客户端可以看到的外观身份，不包含实例 ID、词缀、耐久或属性。
 * AppearanceId 由角色表现系统映射到具体模型/材质资源，运行时不直接复制资源指针。
 */
UCLASS(BlueprintType, EditInlineNew)
class DRAGONOATH_API UDOItemFragment_EquipmentAppearance : public UDOItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Appearance", meta = (DisplayName = "外观标识"))
	FName AppearanceId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Appearance", meta = (Categories = "Equipment.Appearance.Variant", DisplayName = "外观变体"))
	FGameplayTag VariantTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Appearance", meta = (DisplayName = "外观染色"))
	FLinearColor Tint = FLinearColor::White;
};

/** 消耗品专属 Fragment。简单物品使用原生 GE，复杂物品可以触发 GA/Event。 */
UCLASS(BlueprintType, EditInlineNew)
class DRAGONOATH_API UDOItemFragment_Consumable : public UDOItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (DisplayName = "消耗品效果类型"))
	EDOConsumableEffectKind EffectKind = EDOConsumableEffectKind::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ShowOnlyInnerProperties, EditCondition = "EffectKind == EDOConsumableEffectKind::InstantRestore", EditConditionHides, DisplayName = "即时恢复数值"))
	FDOResourceRestoreValues InstantRestore;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ShowOnlyInnerProperties, EditCondition = "EffectKind == EDOConsumableEffectKind::TimedAttributeModifier", EditConditionHides, DisplayName = "限时属性效果"))
	FDOItemTimedModifierValues TimedModifier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (EditCondition = "EffectKind == EDOConsumableEffectKind::GameplayAbility", EditConditionHides, DisplayName = "使用时激活技能"))
	TSubclassOf<UGameplayAbility> UseGameplayAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (Categories = "Event", EditCondition = "EffectKind == EDOConsumableEffectKind::GameplayEvent", EditConditionHides, DisplayName = "使用事件标签"))
	FGameplayTag UseEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ShowOnlyInnerProperties, DisplayName = "公共冷却"))
	FDOItemCooldownConfig Cooldown;

	/** 旧方案兼容字段：普通效果迁移到 EffectKind + 类型化结构体。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Legacy", meta = (DeprecatedProperty, DeprecationMessage = "请迁移到 EffectKind 和类型化效果字段。", DisplayName = "旧版使用效果"))
	TSubclassOf<UGameplayEffect> UseGameplayEffect;

	/** 旧方案兼容字段：仅用于读取插件或旧资产配置的公共冷却 Tag。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Legacy", meta = (Categories = "Cooldown", DeprecatedProperty, DeprecationMessage = "请迁移到 Cooldown。", DisplayName = "旧版公共冷却标签"))
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display", meta = (DisplayName = "物品名称"))
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display", meta = (MultiLine = "true", DisplayName = "物品描述"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display", meta = (DisplayName = "物品图标"))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type", meta = (Categories = "Item.Type", DisplayName = "物品类型"))
	FGameplayTag ItemType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type", meta = (Categories = "Item.Rarity", DisplayName = "稀有度"))
	FGameplayTag Rarity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type", meta = (Categories = "Item", DisplayName = "物品标签"))
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules", meta = (ClampMin = "1", DisplayName = "最大堆叠数量"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules", meta = (DisplayName = "排序优先级"))
	int32 SortPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules", meta = (ClampMin = "0", DisplayName = "出售价格"))
	int32 SellPrice = 0;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Item|Fragments", meta = (DisplayName = "物品配置片段"))
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
