// Copyright Epic Games, Inc. All Rights Reserved.
//
// UDOHealthComponent 实现 —— 行为层（死亡状态机 / Status Tag / FDOVerbMessage 广播 / 蓝图委托）。
// 数值层在 UDOHealthSet，本类只负责订阅三个 AttributeSet 委托并把"事件"翻译为蓝图 / Tag / 消息。
// Lyra 等价参考：Source/LyraGame/Character/LyraHealthComponent.cpp

#include "Components/DOHealthComponent.h"

#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/DOVerbMessage.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOHealthComponent)

UDOHealthComponent::UDOHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	// 状态机初始化为 NotDead
	DeathState = EDODeathState::NotDead;

	// ASC 在外部 InitializeWithAbilitySystem 时才注入；这里不强制
	SetIsReplicatedByDefault(true);
}

void UDOHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DeathState 复制到所有客户端；OnRep 回调触发蓝图委托
	DOREPLIFETIME(UDOHealthComponent, DeathState);
}

void UDOHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// 真正注入在 InitializeWithAbilitySystem；这里只兜底（如果在 BeginPlay 之前还没注入）
}

void UDOHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 反注册委托，防止 GC 后触发野回调
	UninitializeFromAbilitySystem();

	Super::EndPlay(EndPlayReason);
}

void UDOHealthComponent::InitializeWithAbilitySystem(UDOAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent == InASC)
	{
		// 已经被注入过，幂等保护
		return;
	}

	AbilitySystemComponent = InASC;

	// HealthSet 通过 ASC 的 GetSet 拿到（玩家走 PlayerState 上的；怪物走 Character 上的）。
	// 客户端上也走这条路径，因为 ASC->GetSet<UDOHealthSet>() 是 OwnerActor 的子对象。
	// GetSet 返回 const UDOHealthSet*，本类需绑定非 const 委托，故 const_cast 脱 const（Lyra 同款做法）。
	HealthSet = AbilitySystemComponent ? const_cast<UDOHealthSet*>(AbilitySystemComponent->GetSet<UDOHealthSet>()) : nullptr;

	if (!HealthSet)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[DO|HealthComponent] %s: InitializeWithAbilitySystem failed: UDOHealthSet not found on OwnerActor %s. "
				 "Check that CreateDefaultSubobject<UDOHealthSet> was called before InitAbilityActorInfo."),
			*GetNameSafe(Owner), *GetNameSafe(Owner));
		return;
	}

	// 三个委托挂钩 —— HealthSet 已经声明 OnHealthChanged / OnMaxHealthChanged / OnOutOfHealth 三个 FDOAttributeEvent
	HealthSet->OnHealthChanged.AddUObject(this, &UDOHealthComponent::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &UDOHealthComponent::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &UDOHealthComponent::HandleOutOfHealth);
}

void UDOHealthComponent::UninitializeFromAbilitySystem()
{
	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

// ====================================================================
// 三个 AttributeSet 委托回调
// ====================================================================

void UDOHealthComponent::HandleHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	// 蓝图委托 —— 客户端和服务器都会触发（PostAttributeChange 是双端调用）
	OnHealthChanged.Broadcast(this, OldValue, NewValue, EffectCauser ? EffectCauser : EffectInstigator);

	// 服务端权威广播 DamageApplied（仅权威；客户端走 FDOVerbMessageReplication 跨网）
	if (AbilitySystemComponent && AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		BroadcastDamageApplied(EffectInstigator, EffectCauser, EffectSpec, EffectMagnitude);
	}
}

void UDOHealthComponent::HandleMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	// 蓝图委托
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, EffectCauser ? EffectCauser : EffectInstigator);
	// MaxHealth 变化不广播 FDOVerbMessage —— 上限变化通常与战斗表现无关
}

void UDOHealthComponent::HandleOutOfHealth(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	// 蓝图委托
	OnHealthChanged.Broadcast(this, OldValue, NewValue, EffectCauser ? EffectCauser : EffectInstigator);

	// 服务端权威动作：触发 GAS 死亡事件 + 广播 EliminationFired + 推进状态机
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// 1) 触发 Event::Death —— 给 GA_Death 等 GAS 监听者用（目前无监听者，未来 GA_Death 创建后可 WaitGameplayEvent 接收）
		if (EffectSpec)
		{
			FGameplayEventData EventData;
			EventData.EventTag       = DragonOathGameplayTags::Event::Death;
			EventData.Instigator     = EffectCauser ? EffectCauser : EffectInstigator;
			EventData.Target         = GetOwner();
			EventData.EventMagnitude = FMath::Abs(NewValue - OldValue);
			EventData.ContextHandle  = EffectSpec->GetEffectContext();

			AbilitySystemComponent->HandleGameplayEvent(EventData.EventTag, &EventData);
		}

		// 2) 权威广播 FDOVerbMessage「击杀事件」—— 给 HUD / 击杀横幅 / 助攻检测 用
		BroadcastEliminationFired(EffectInstigator, EffectCauser, EffectSpec, EffectMagnitude);

		// 3) 推进状态机到 Dying
		StartDeath();
	}
#endif
}

// ====================================================================
// 死亡流程
// ====================================================================

void UDOHealthComponent::StartDeath()
{
	if (DeathState != EDODeathState::NotDead)
	{
		// 已经在 Dying 或 Dead，幂等保护
		return;
	}

	// 服务器写值 → 客户端 OnRep_DeathState 收到后会重新触发 OnDeathStarted.Broadcast
	DeathState = EDODeathState::DeathStarted;

	// 立即本地广播（服务器自己 / ListenServer 主机客户端能立即拿到）
	OnDeathStarted.Broadcast(GetOwner());

	// 应用 Status_Death_Dying Tag 到 ASC —— 后续 GA_Death 的 Ability_ActivateFail_IsDead 会自动拒绝激活
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(DragonOathGameplayTags::Status::Death_Dying);
	}

	// 默认立刻 FinishDeath（后续可改为"播完死亡 Montage 再 Finish"）
	// 这里先简化：让 Character 蓝图技能（GA_Death 蓝图资产）显式调 FinishDeath()
	// 若 Character 层不调，OnRep_DeathState 也会同步 DeathFinished 给客户端，但服务端不会自动推进
}

void UDOHealthComponent::FinishDeath()
{
	if (DeathState != EDODeathState::DeathStarted)
	{
		return;
	}

	DeathState = EDODeathState::DeathFinished;

	OnDeathFinished.Broadcast(GetOwner());

	if (AbilitySystemComponent)
	{
		// Dying 转 Dead：移除 Dying，添加 Dead（Dead 更严格，后续 Ability 互斥按 Dead 走）
		AbilitySystemComponent->RemoveLooseGameplayTag(DragonOathGameplayTags::Status::Death_Dying);
		AbilitySystemComponent->AddLooseGameplayTag(DragonOathGameplayTags::Status::Death_Dead);
	}
}

void UDOHealthComponent::OnRep_DeathState(EDODeathState OldDeathState)
{
	// 客户端走这条路径重新触发蓝图委托（与 Lyra 一致：服务端广播 + 客户端 OnRep 同步）
	if (OldDeathState == EDODeathState::NotDead && DeathState == EDODeathState::DeathStarted)
	{
		OnDeathStarted.Broadcast(GetOwner());
	}
	else if (OldDeathState == EDODeathState::DeathStarted && DeathState == EDODeathState::DeathFinished)
	{
		OnDeathFinished.Broadcast(GetOwner());
	}
}

// ====================================================================
// FDOVerbMessage 广播
// ====================================================================

void UDOHealthComponent::BroadcastDamageApplied(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FDOVerbMessage DamageMsg;
	DamageMsg.Verb       = DragonOathGameplayTags::Message::Combat::DamageApplied;
	DamageMsg.Instigator = EffectCauser ? EffectCauser : EffectInstigator;
	DamageMsg.Target     = GetOwner();
	DamageMsg.Magnitude  = EffectMagnitude;

	if (EffectSpec)
	{
		// 把 Instigator / Target Tag 一起带上，订阅方按需过滤
		if (const FGameplayTagContainer* SrcTags = EffectSpec->CapturedSourceTags.GetAggregatedTags())
		{
			DamageMsg.InstigatorTags = *SrcTags;
		}
		if (const FGameplayTagContainer* TgtTags = EffectSpec->CapturedTargetTags.GetAggregatedTags())
		{
			DamageMsg.TargetTags = *TgtTags;
		}
		// ContextTags：Damage.Source / Damage.Type 等（订阅方可用于筛选"是不是暴击""是不是元素伤害"）
		if (EffectSpec->GetEffectContext().GetSourceObject())
		{
			DamageMsg.ContextTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Combat.Damage.Source"), /*ErrorIfNotFound*/ false));
		}
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
	MessageSubsystem.BroadcastMessage(DamageMsg.Verb, DamageMsg);

	UE_LOG(LogTemp, Verbose,
		TEXT("[DO|HealthComponent] Broadcast DamageApplied: Instigator=%s, Target=%s, Magnitude=%.1f"),
		*GetNameSafe(DamageMsg.Instigator), *GetNameSafe(DamageMsg.Target), DamageMsg.Magnitude);
}

void UDOHealthComponent::BroadcastEliminationFired(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FDOVerbMessage ElimMsg;
	ElimMsg.Verb       = DragonOathGameplayTags::Message::Combat::EliminationFired;
	ElimMsg.Instigator = EffectCauser ? EffectCauser : EffectInstigator;
	ElimMsg.Target     = GetOwner();
	ElimMsg.Magnitude  = EffectMagnitude;  // 击杀那一击的伤害值，订阅方可据此判定是否"一击致命"

	if (EffectSpec)
	{
		if (const FGameplayTagContainer* SrcTags = EffectSpec->CapturedSourceTags.GetAggregatedTags())
		{
			ElimMsg.InstigatorTags = *SrcTags;
		}
		if (const FGameplayTagContainer* TgtTags = EffectSpec->CapturedTargetTags.GetAggregatedTags())
		{
			ElimMsg.TargetTags = *TgtTags;
		}
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
	MessageSubsystem.BroadcastMessage(ElimMsg.Verb, ElimMsg);

	UE_LOG(LogTemp, Display,
		TEXT("[DO|HealthComponent] Broadcast EliminationFired: Instigator=%s, Target=%s, Magnitude=%.1f"),
		*GetNameSafe(ElimMsg.Instigator), *GetNameSafe(ElimMsg.Target), ElimMsg.Magnitude);
}