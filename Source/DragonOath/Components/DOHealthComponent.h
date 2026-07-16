// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"  // UE 5.8 ModularGameplay 插件自带基类
#include "DOHealthComponent.generated.h"

class UDOAbilitySystemComponent;
class UDOHealthSet;
class AActor;
struct FGameplayEffectSpec;

/**
 * 死亡状态机（Lyra 三层架构中的状态机层）。
 *
 * NotDead → DeathStarted（进入 Dying） → DeathFinished（进入 Dead 终态）。
 * 服务器推进状态，客户端通过 OnRep_DeathState 同步；这样死亡 Tag 应用与蓝图委托触发在两端都对齐。
 */
UENUM(BlueprintType)
enum class EDODeathState : uint8
{
	NotDead        = 0  UMETA(DisplayName = "Alive"),
	DeathStarted   = 1  UMETA(DisplayName = "Dying"),
	DeathFinished  = 2  UMETA(DisplayName = "Dead"),
};

// 蓝图可订阅的死亡委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOHealthComponent_DeathEvent, AActor*, OwningActor);
// 蓝图可订阅的属性变化委托（保留 NewValue / OldValue，Instigator 放在 handle 里以便外部按需 cast 出 Component）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FDOHealthComponent_AttributeChanged, UDOHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/**
 * UDOHealthComponent —— DragonOath 的 HealthComponent，Lyra ULyraHealthComponent 的 DO 前缀版本。
 *
 * 职责（与 UDOHealthSet 严格分工）：
 *   - UDOHealthSet：纯数值层（Damage Meta → Health 转换、吸血、bOutOfHealth 幂等）
 *   - UDOHealthComponent（本类）：行为层（死亡状态机、Status Tag 应用、FDOVerbMessage 广播、蓝图委托）
 *   - GA_Death（未来蓝图技能）：死亡流程执行（死亡 Montage、Ragdoll、Input/Collision 禁用、Respawn）
 *
 * 网络：
 *   - DeathState 用 UPROPERTY(ReplicatedUsing=OnRep_DeathState) 同步到所有客户端
 *   - FDOVerbMessage 广播走 GameplayMessageRouter（GameplayMessageSubsystem），是本地频道；跨网靠 FDOVerbMessageReplication（PlayerState 上挂）补充
 *
 * 使用方式（仿 Lyra）：
 *   1. ADOCharacter / ADOPlayerState 构造时 CreateDefaultSubobject<UDOHealthComponent>
 *   2. ADOCharacter::InitializeAbilitySystemComponent / ADOPlayerState::InitializeAbilitySystemComponent 末尾调用 InitializeWithAbilitySystem(DOASC)
 *   3. 蓝图里绑 OnHealthChanged / OnMaxHealthChanged / OnDeathStarted / OnDeathFinished 即可
 */
UCLASS(ClassGroup=(DO), meta=(BlueprintSpawnableComponent))
class DRAGONOATH_API UDOHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UDOHealthComponent(const FObjectInitializer& ObjectInitializer);

	// ====== 死亡流程入口（子类或外部 Actor 可重写 / 触发）======

	/** 服务器：推进到 Dying。幂等：只在 NotDead → DeathStarted 触发一次。 */
	virtual void StartDeath();

	/** 服务器：推进到 Dead。幂等：只在 DeathStarted → DeathFinished 触发一次。 */
	virtual void FinishDeath();

	/** 蓝图查询：是否在死亡流程中（Dying 或 Dead） */
	UFUNCTION(BlueprintPure, Category = "DO|Health")
	bool IsDeadOrDying() const { return DeathState != EDODeathState::NotDead; }

	/** 蓝图查询：当前死亡状态 */
	UFUNCTION(BlueprintPure, Category = "DO|Health")
	EDODeathState GetDeathState() const { return DeathState; }

	// ====== 注入 ASC（仿 Lyra InitializeWithAbilitySystem 模式）======

	/**
	 * 把 ASC 注入 Component，挂 3 个 AttributeSet 委托。
	 * 调用时机：ADOCharacter::InitializeAbilitySystemComponent 或 ADOPlayerState::InitializeAbilitySystemComponent 末尾，
	 * 在 ASC->InitAbilityActorInfo 之后。
	 */
	void InitializeWithAbilitySystem(UDOAbilitySystemComponent* InASC);

	/** 反注册委托，清理缓存指针（一般在 EndPlay 或 Actor 销毁前调用） */
	void UninitializeFromAbilitySystem();

	// ====== 蓝图委托（FDOVerbMessage 订阅者也可挂这里）======

	UPROPERTY(BlueprintAssignable, Category = "DO|Health")
	FDOHealthComponent_DeathEvent OnDeathStarted;

	UPROPERTY(BlueprintAssignable, Category = "DO|Health")
	FDOHealthComponent_DeathEvent OnDeathFinished;

	UPROPERTY(BlueprintAssignable, Category = "DO|Health")
	FDOHealthComponent_AttributeChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "DO|Health")
	FDOHealthComponent_AttributeChanged OnMaxHealthChanged;

	// ====== 静态访问入口（Lyra FindHealthComponent 同款）======

	/** 从任意 Actor 上找 HealthComponent，找不到返回 nullptr */
	static UDOHealthComponent* FindHealthComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UDOHealthComponent>() : nullptr;
	}

protected:
	// ====== AActorComponent 接口 ======
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ====== AttributeSet 委托回调（匹配 DOHealthSet.h 的 FDOAttributeEvent 六参签名）======

	void HandleHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	void HandleMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	void HandleOutOfHealth(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);

	/** 客户端通过 OnRep 拿到 DeathState 变化后重新触发蓝图委托 */
	UFUNCTION()
	void OnRep_DeathState(EDODeathState OldDeathState);

	// ====== 复制属性 ======

	/** 死亡状态机；ReplicatedUsing 让客户端 OnRep_DeathState 同步触发蓝图委托 */
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EDODeathState DeathState;

	// ====== 缓存指针（非复制，运行时注入）======

	UPROPERTY(Transient)
	TObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UDOHealthSet> HealthSet;

private:
	/** 私有：服务端权威广播 Message.Combat.Damage.Applied */
	void BroadcastDamageApplied(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude);

	/** 私有：服务端权威广播 Message.Combat.Elimination.Fired（仅 HandleOutOfHealth 内调用） */
	void BroadcastEliminationFired(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude);
};