// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DOGameplayAbility.generated.h"

class UDOAbilitySystemComponent;
struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;
struct FGameplayEventData;

UENUM(BlueprintType)
enum class EDOAbilityActivationPolicy : uint8
{
	// 按下输入时触发一次，适合普攻、闪避、瞬发技能。
	OnInputTriggered,

	// 按键保持按下时尝试激活，适合格挡、蓄力、引导、持续瞄准。
	WhileInputActive,

	// 技能被授予或 Avatar 切换完成后自动尝试激活。
	OnSpawn,
};

/**
 * DragonOath 的技能基类。
 *
 * 这层基类不重复封装 UGameplayAbility 已经提供的能力，只保留项目真正需要统一的规则：
 * - 默认使用 InstancedPerActor，避免 UE5.8 对 NonInstanced 的边界问题。
 * - 默认使用 LocalPredicted，适合玩家技能的本地预测手感。
 * - 把复杂的 GAS 生命周期入口收束到 DO* 扩展点，降低子类忘记调用 Super 的概率。
 */
UCLASS(Abstract, Blueprintable)
class DRAGONOATH_API UDOGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDOGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//~End of UGameplayAbility interface

	UFUNCTION(BlueprintCallable, Category="DO|Ability")
	UDOAbilitySystemComponent* GetDOAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category="DO|Ability")
	APawn* GetDOPawnAvatar() const;

	UFUNCTION(BlueprintCallable, Category="DO|Ability")
	AController* GetDOController() const;

	UFUNCTION(BlueprintPure, Category="DO|Ability")
	EDOAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	UFUNCTION(BlueprintCallable, Category="DO|Ability")
	void ApplyEffectSpecToTarget(UPARAM(ref) FGameplayEffectSpecHandle& SpecHandle, UAbilitySystemComponent* TargetASC);

	UFUNCTION(BlueprintCallable, Category="DO|Ability")
	void ApplyEffectSpecToHitResult(UPARAM(ref) FGameplayEffectSpecHandle& SpecHandle, const FHitResult& HitResult, UAbilitySystemComponent* TargetASC);

	UFUNCTION(BlueprintCallable, Category="DO|Ability")
	void BroadcastTargetData(const FGameplayAbilityTargetDataHandle& TargetData, bool bShouldReplicate);

	virtual void OnPawnAvatarSet();

protected:
	// 子类扩展点：普通技能优先重写这组 DO* 函数，不要直接重写 GAS 原生入口。
	// 原生入口负责维持 Commit、输入复制、EndAbility 等生命周期顺序；DO* 只放项目玩法逻辑。

	// 激活前的项目自定义检查，例如职业状态、武器状态、连招窗口等。
	virtual bool DOCanActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const;

	// Commit 成功并进入 GAS 激活流程后调用。技能播放蒙太奇、创建任务、发射弹道通常从这里开始。
	virtual void DOActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	// 技能结束时的项目清理点。不要在这里再次调用 EndAbility，基类会统一处理。
	virtual void DOEndAbility(bool bWasCancelled);

	// 已激活技能收到再次按下输入时调用，适合连击、蓄力刷新、确认释放等玩法。
	virtual void DOInputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	// 已激活技能收到松开输入时调用，适合松手释放、停止引导、停止格挡等玩法。
	virtual void DOInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	void LogAbilityFailure(const FString& Reason, const FGameplayAbilityActorInfo* ActorInfo) const;

protected:
	// 输入系统根据这个策略决定按下、按住、生成时是否尝试激活技能。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DO|Ability")
	EDOAbilityActivationPolicy ActivationPolicy = EDOAbilityActivationPolicy::OnInputTriggered;

	// 开启后，技能激活失败会输出 Verbose 日志，方便早期调试技能标签、消耗和冷却。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DO|Ability")
	bool bLogAbilityFailures = true;

	// 大多数技能激活时都需要立刻扣消耗/上冷却；少数蓄力或确认类技能可以关闭后手动 Commit。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DO|Ability")
	bool bAutoCommitOnActivate = true;
};
