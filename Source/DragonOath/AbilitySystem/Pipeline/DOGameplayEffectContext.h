// DragonOath custom GameplayEffectContext.
// 说明：携带暴击、格挡、部位、元素等额外战斗数据穿过 GAS 伤害管线。

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "DOGameplayEffectContext.generated.h"

/**
 * 自定义GameplayEffectContext —— 生产项目必备
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                 为什么需要自定义 EffectContext？               │
 * ├──────────────────────────────────────────────────────────────┤
 * │                                                              │
 * │  默认 FGameplayEffectContext 只包含：                         │
 * │    - Instigator (谁发起的)                                   │
 * │    - EffectCauser (什么造成的)                                │
 * │    - SourceObject (来源对象)                                  │
 * │    - HitResult (命中结果)                                    │
 * │                                                              │
 * │  但实际项目中，伤害管线需要传递更多信息：                       │
 * │    - 是否暴击？                                              │
 * │    - 命中了哪个骨骼/部位？                                    │
 * │    - 伤害元素类型（火/冰/雷）                                 │
 * │    - 弹道方向（用于击退计算）                                 │
 * │    - 武器信息                                                │
 * │    - 是否是背刺？                                            │
 * │                                                              │
 * │  这些数据需要从GA一路传递到：                                 │
 * │    GA → GE → ExecCalc → AttributeSet → GameplayCue          │
 * │                                                              │
 * │  自定义EffectContext就是在这条管线上"搭便车"传递数据的方式     │
 * │                                                              │
 * └──────────────────────────────────────────────────────────────┘
 *
 * 【网络序列化】
 * EffectContext会随GE一起在网络上传输。
 * 自定义字段必须实现 NetSerialize，否则这些数据不会同步到客户端。
 * 这对GameplayCue很重要——Cue需要知道是否暴击来播放不同特效。
 *
 * 【设置步骤】
 * 1. 继承 FGameplayEffectContext（本文件）
 * 2. 继承 UAbilitySystemGlobals，重写 AllocGameplayEffectContext()
 * 3. 在 DefaultGame.ini 中指定自定义 Globals 类
 */
USTRUCT()
struct FDOGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_USTRUCT_BODY()

public:

	FDOGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	// ============================================================
	// 自定义数据字段
	// ============================================================

	/** 是否暴击 */
	bool IsCriticalHit() const { return bIsCriticalHit; }
	void SetIsCriticalHit(bool bCrit) { bIsCriticalHit = bCrit; }

	/** 是否格挡 */
	bool IsBlocked() const { return bIsBlocked; }
	void SetIsBlocked(bool bBlock) { bIsBlocked = bBlock; }

	/** 命中骨骼名（用于部位伤害） */
	FName GetHitBoneName() const { return HitBoneName; }
	void SetHitBoneName(FName Bone) { HitBoneName = Bone; }

	/** 伤害方向（用于击退/受击动画方向） */
	FVector GetDamageDirection() const { return DamageDirection; }
	void SetDamageDirection(const FVector& Dir) { DamageDirection = Dir; }

	/** 伤害元素类型Tag */
	FGameplayTag GetDamageElementTag() const { return DamageElementTag; }
	void SetDamageElementTag(FGameplayTag Tag) { DamageElementTag = Tag; }

	/** 伤害倍率（部位伤害：爆头2x、身体1x、四肢0.5x） */
	float GetDamageMultiplier() const { return DamageMultiplier; }
	void SetDamageMultiplier(float Mult) { DamageMultiplier = Mult; }

	// ============================================================
	// 必须重写的虚函数
	// ============================================================

	/**
	 * 返回实际的Struct类型
	 * 【关键】如果不重写，父类的模板函数会用错误的类型创建副本
	 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FDOGameplayEffectContext::StaticStruct();
	}

	/**
	 * 复制Context
	 * 【关键】所有自定义字段都必须在这里复制
	 * 否则GE复制（如Stacking创建新Spec时）会丢失自定义数据
	 */
	virtual FDOGameplayEffectContext* Duplicate() const override
	{
		FDOGameplayEffectContext* NewContext = new FDOGameplayEffectContext();
		*NewContext = *this;  // 浅拷贝所有字段
		// 深拷贝 HitResult（父类的指针成员）
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/**
	 * 【网络关键】自定义网络序列化
	 * EffectContext随GE在网络上传输时调用此函数
	 * 不实现 = 自定义字段不会同步到客户端
	 *
	 * 这对GameplayCue非常重要：
	 * 客户端需要知道是否暴击来播放对应的受击特效（暴击闪光 vs 普通命中）
	 */
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

protected:

	/** 是否暴击 */
	UPROPERTY()
	bool bIsCriticalHit = false;

	/** 是否被格挡 */
	UPROPERTY()
	bool bIsBlocked = false;

	/** 命中骨骼名 */
	UPROPERTY()
	FName HitBoneName = NAME_None;

	/** 伤害方向 */
	UPROPERTY()
	FVector DamageDirection = FVector::ZeroVector;

	/** 伤害元素类型 */
	UPROPERTY()
	FGameplayTag DamageElementTag;

	/** 部位伤害倍率 */
	UPROPERTY()
	float DamageMultiplier = 1.0f;
};

// ================================================================
// 【关键】注册自定义EffectContext的序列化
// ================================================================
// TStructOpsTypeTraits 告诉UE这个struct有自定义的NetSerialize
// 没有这个，UE会跳过NetSerialize调用

template<>
struct TStructOpsTypeTraits<FDOGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FDOGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true,
	};
};
