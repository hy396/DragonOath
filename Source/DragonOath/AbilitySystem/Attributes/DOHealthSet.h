// DragonOath health AttributeSet.
// 说明：负责生命值、伤害 Meta Attribute、死亡事件入口。

#pragma once

#include "CoreMinimal.h"
#include "DOAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DOHealthSet.generated.h"


/**
 * 生命属性集。
 *
 * 负责 Health / MaxHealth，以及 Damage 这种只用于结算的 Meta Attribute。
 * 复杂伤害公式不要堆在这里；这里主要做“伤害结果落到生命值”和死亡事件派发。
 */
UCLASS()
class DRAGONOATH_API UDOHealthSet : public UDOAttributeSet
{
	GENERATED_BODY()

public:

	UDOHealthSet();

	// Health 变化事件。客户端收到复制时可能拿不到完整 EffectSpec，因此监听者需要允许部分参数为空。
	mutable FDOAttributeEvent OnHealthChanged;

	// MaxHealth 变化事件，通常用于刷新血条上限。
	mutable FDOAttributeEvent OnMaxHealthChanged;

	// Health 第一次降到 0 时广播，死亡流程应由监听者或 GameplayEvent 接手。
	mutable FDOAttributeEvent OnOutOfHealth;

	// ============================================================
	// 属性定义 —— 所有属性都需要 ReplicatedUsing 来支持网络同步
	// ============================================================

	/** 当前生命值 */
	UPROPERTY(BlueprintReadOnly, Category="DO|Health", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDOHealthSet, Health)

	/** 最大生命值 */
	UPROPERTY(BlueprintReadOnly, Category="DO|Health", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDOHealthSet, MaxHealth)

	/** 生命回复：每秒回复量。仅 Owner 复制，回复 Periodic GE 读取此值后施加治疗。 */
	UPROPERTY(BlueprintReadOnly, Category="DO|Health", ReplicatedUsing=OnRep_HealthRegen)
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UDOHealthSet, HealthRegen)

	// 防止 Health 已经为 0 时重复广播死亡事件。
	bool bOutOfHealth;

	// 记录变更前的值，方便事件广播时给 UI/日志提供 OldValue。
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
    
	// 治疗输入值。当前尚未接入完整治疗结算，可后续像 Damage 一样在 PostGameplayEffectExecute 中转换。
	UPROPERTY(BlueprintReadOnly, Category="DO|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UDOHealthSet, Healing);

	/**
	 * 伤害值 —— 这是一个"Meta Attribute"（元属性）
	 * 【重要】Meta属性不会被同步！它只是一个临时中转值
	 * 流程：GE设置Damage -> PostGameplayEffectExecute读取Damage -> 修改Health -> 清零Damage
	 */
	UPROPERTY(BlueprintReadOnly, Category="DO|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UDOHealthSet, Damage)

	// ============================================================
	// 网络同步 —— GetLifetimeReplicatedProps + OnRep 回调
	// ============================================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 【网络知识点】OnRep 回调
	 * 当服务器修改属性并同步到客户端时触发
	 * GAMEPLAYATTRIBUTE_REPNOTIFY 会通知ASC属性发生了变化，
	 * 这样绑定了 AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate() 的UI才能收到更新
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen);

	// ============================================================
	// 属性变化钩子 —— 理解它们在网络中的执行时机非常关键
	// ============================================================
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	/**
	 * 【在客户端和服务器都会调用】
	 * 在属性值被修改之前调用，用于Clamp值到合法范围
	 * 注意：这里修改的是"当前值"（CurrentValue），不是BaseValue
	 * 不要在这里做游戏逻辑！只做Clamp！
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	// GE 真正修改属性前的最后拦截点，可用于缓存旧值或阻止某些非法执行。
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	
	/**
	 * 【只在服务器执行！】
	 * 在GameplayEffect执行之后调用
	 * 这是处理伤害->生命值转换、死亡判定等逻辑的正确位置
	 * 因为只在服务器执行，所以可以安全地做权威判断
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

};
