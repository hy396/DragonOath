#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"

#include "ItemSystem/Core/DOItemAttributeTypes.h"
#include "ItemSystem/Core/DOItemInstanceTypes.h"

class UDOAbilitySystemComponent;
class UGameplayEffect;
class UObject;

/**
 * 把 ItemDefinition 中的类型化数据转换成 GAS Spec。
 *
 * Builder 只负责“数据 -> Spec”，不修改背包、不扣除物品，也不保存 ActiveGameplayEffectHandle。
 * 所有调用都应发生在服务器组件内部，客户端不能提交最终属性值、持续时间或冷却时间。
 */
struct DRAGONOATH_API FDOItemEffectSpecBuilder final
{
	public:
	/** 构建一件装备的无限时长属性 Spec。 */
	static bool BuildEquipmentSpec(
		UDOAbilitySystemComponent& ASC,
		UObject& SourceObject,
		const FDOAttributeModifierValues& Values,
		const TArray<FDOItemAffixRoll>& Affixes,
		float UpgradeScale,
		FGameplayEffectSpecHandle& OutSpec);

	/** 兼容没有动态词缀的旧调用方。 */
	static bool BuildEquipmentSpec(
		UDOAbilitySystemComponent& ASC,
		UObject& SourceObject,
		const FDOAttributeModifierValues& Values,
		float UpgradeScale,
		FGameplayEffectSpecHandle& OutSpec)
	{
		return BuildEquipmentSpec(ASC, SourceObject, Values, TArray<FDOItemAffixRoll>(), UpgradeScale, OutSpec);
	}

	/** 构建即时回复 Spec。 */
	static bool BuildInstantRestoreSpec(
		UDOAbilitySystemComponent& ASC,
		UObject& SourceObject,
		const FDOResourceRestoreValues& Values,
		FGameplayEffectSpecHandle& OutSpec);

	/** 构建限时属性 Spec。 */
	static bool BuildTimedModifierSpec(
		UDOAbilitySystemComponent& ASC,
		UObject& SourceObject,
		const FDOItemTimedModifierValues& Values,
		FGameplayEffectSpecHandle& OutSpec);

	/** 构建公共冷却 Spec，并给 Spec 添加动态冷却 Tag。 */
	static bool BuildCooldownSpec(
		UDOAbilitySystemComponent& ASC,
		UObject& SourceObject,
		const FDOItemCooldownConfig& Cooldown,
		FGameplayEffectSpecHandle& OutSpec);

	private:
	/** 创建稳定原生 GE 的 Spec，并统一设置来源对象。 */
	static bool InitializeSpec(
		UDOAbilitySystemComponent& ASC,
		UObject& SourceObject,
		TSubclassOf<UGameplayEffect> EffectClass,
		FGameplayEffectSpecHandle& OutSpec);

	/** 将装备/限时属性字段写入统一的 Data.Equipment.* 标签。 */
	static void WriteAttributeMagnitudes(
		FGameplayEffectSpec& Spec,
		const FDOAttributeModifierValues& Values,
		float Scale);

	/** 将即时回复字段写入 Data.ItemUse.* 标签。 */
	static void WriteRestoreMagnitudes(
		FGameplayEffectSpec& Spec,
		const FDOResourceRestoreValues& Values);
};
