// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Common/DOGameplayAbility_NormalAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
// 本 ability 是「普攻命中」时的天然广播点，可作为 FDOVerbMessage 的「备选发布者」——
// 当伤害 GE 尚未接入、或想跳出 GE 流程直接广播表现层事件时（比如立即在命中瞬间飘字），
// 在 ActivateAbility 命中分支 / OnMontageCompleted 末尾追加 BroadcastMessage 即可。
// 优先选择：仍走伤害 GE → DOHealthSet::PostGameplayEffectExecute（见 DOHealthSet.cpp），
// 这样能统一拿到暴击 / 格挡 / 部位 / 元素 等 EffectContext 信息，避免重复广播。
#include "Messages/DOVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility_NormalAttack)

UDOGameplayAbility_NormalAttack::UDOGameplayAbility_NormalAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 普攻是瞬发技能，按下即触发
	ActivationPolicy = EDOAbilityActivationPolicy::OnInputTriggered;

	// 冲刺攻击窗口期间被阻止，让冲刺攻击优先
	ActivationBlockedTags.AddTag(DragonOathGameplayTags::Status::DashAttackWindow);
}

// ==================== 「立即广播伤害事件」备选路径（注释参考）====================
// 适用场景：项目早期阶段伤害 GE / ExecutionCalculation 还没接通，但 ability 已经能命中目标，
// 想先把 UI / HUD 通路打通，看到伤害数字飘出来。
// 接入方式：在 ActivateAbility 里命中目标时（HitResult 拿到之后）调用下面这段。
// 接入位置示意（实际要等命中逻辑实现）：
/*
{
    // 假设 HitResult 已经拿到，TargetActor 已选定，伤害值已计算（LocalDamage）
    AActor* TargetActor = HitResult.GetActor();
    AActor* SourceActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
    if (!TargetActor || !SourceActor) { return; }

    FDOVerbMessage DamageMsg;
    DamageMsg.Verb       = DragonOathGameplayTags::Message::Combat::DamageApplied;
    DamageMsg.Instigator = SourceActor;
    DamageMsg.Target     = TargetActor;
    DamageMsg.Magnitude  = LocalDamage; // 普攻伤害值

    if (UWorld* World = GetWorld())
    {
        UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(World);
        MessageSystem.BroadcastMessage(DamageMsg.Verb, DamageMsg);
    }
}
*/
// 重要：一旦伤害 GE 通路接通，请把这段代码删掉，统一走 DOHealthSet::PostGameplayEffectExecute，
// 避免「同一事件被广播两次」。本注释仅作教学引导，不参与运行时逻辑。
// ============================================================================

void UDOGameplayAbility_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 播放普攻动画
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AttackMontage, 1.0f, NAME_None, false);

		MontageTask->OnCompleted.AddDynamic(this, &UDOGameplayAbility_NormalAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UDOGameplayAbility_NormalAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 没有动画直接结束
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UDOGameplayAbility_NormalAttack::OnMontageCompleted()
{
	// 动画正常播放完成，结束技能
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDOGameplayAbility_NormalAttack::OnMontageCancelled()
{
	// 动画被取消（被击、死亡等），结束技能并标记为取消
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}