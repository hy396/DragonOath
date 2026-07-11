// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/DOAbilitySet.h"
#include "DOAbilitySystemComponent.generated.h"

class UDOAbilitySet;
class AActor;
class UGameplayEffect;

// 技能等级变化广播（权威端触发，供服务器侧系统/调试订阅）。
// 客户端监听等级变化请使用 GAS 的 AbilitySpecDirtiedCallbacks（Level 经 Mixed 复制），再经 Message 总线通知 UI。
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDOAbilityLevelChanged, FGameplayTag /*AbilityId*/, int32 /*OldLevel*/, int32 /*NewLevel*/);

/**
 * DragonOath 的 AbilitySystemComponent。
 *
 * ASC 是 GAS 的核心运行时对象：保存已授予技能、GameplayEffect、GameplayTag 和属性聚合结果。
 * 本项目在这里集中处理"输入标签 -> AbilitySpecHandle -> TryActivateAbility"的转换。
 * 同时封装职业技能授予、升级、取消等项目级接口。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRAGONOATH_API UDOAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDOAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 从 Actor 上查找项目 ASC。Pawn 没有直接持有时，UE 会通过 IAbilitySystemInterface 找到 PlayerState 上的 ASC。
	static UDOAbilitySystemComponent* GetFromActor(const AActor* Actor, bool bLookForComponent = true);

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	// 输入层只把 InputAction 翻译成 GameplayTag 交给 ASC。
	// 这里先缓存匹配到的 AbilitySpecHandle，真正激活放到 ProcessAbilityInput 统一处理。
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	// 每帧由 ADOPlayerController::PostProcessInput 调用一次。
	// 放在 Enhanced Input 事件之后处理，可以保证 Pressed / Held / Released 的顺序稳定。
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

	// 批量取消带有指定标签的技能，常用于硬直、死亡、沉默、切换状态等系统。
	void CancelAbilitiesByTag(const FGameplayTagContainer& WithTags, const FGameplayTagContainer& WithoutTags, UGameplayAbility* IgnoreAbility = nullptr);

	//~ 项目级技能授予/升级接口

	// 授予单个技能。返回 SpecHandle，失败返回无效 Handle。
	// 输入标签会放入 Spec 的 DynamicSpecSourceTags，供输入匹配使用。
	UFUNCTION(BlueprintCallable, Category = "DO|Ability")
	FGameplayAbilitySpecHandle GiveDOAbility(const FDOAbilityGrant& Grant);

	// 批量授予一个 AbilitySet 中的全部技能。按 AbilityId 去重。
	UFUNCTION(BlueprintCallable, Category = "DO|Ability")
	void GiveDOAbilitySet(const UDOAbilitySet* AbilitySet);

	// 修改技能等级。必须在服务端调用。
	// 0 -> 1+ 时自动激活 OnSpawn 被动技能；1+ -> 0 时取消正在运行的技能。
	UFUNCTION(BlueprintCallable, Category = "DO|Ability")
	bool SetDOAbilityLevel(FGameplayTag AbilityId, int32 NewLevel);

	// 技能等级变化广播（权威端触发）。见文件顶部 DECLARE_MULTICAST_DELEGATE 注释。
	FOnDOAbilityLevelChanged OnAbilityLevelChanged;

	// 客户端请求升级技能。必须经由服务器权威校验后才会真正改 Level。
	// 校验项：技能已授予、1 <= NewLevel <= MaxLevel、等级有变化。
	// 升级消耗/前置（技能点、SkillTreeComponent 等）暂未接入，见 Server_RequestAbilityLevel_Implementation 内 TODO。
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestAbilityLevel(FGameplayTag AbilityId, int32 NewLevel);

	// 查询技能当前等级。未找到返回 0。
	UFUNCTION(BlueprintPure, Category = "DO|Ability")
	int32 GetDOAbilityLevel(FGameplayTag AbilityId) const;

	// 通过 AbilityId 取消正在激活的技能。
	UFUNCTION(BlueprintCallable, Category = "DO|Ability")
	void CancelAbilityById(FGameplayTag AbilityId);

	// 清除所有通过 GiveDOAbility/GiveDOAbilitySet 授予的技能，用于切职业。
	UFUNCTION(BlueprintCallable, Category = "DO|Ability")
	void ClearDOAbilities();

	//~End of 项目级技能授予/升级接口

	// 获取角色等级，用于伤害公式中的等级缩放。
	// 玩家从 AvatarActor -> DOCharacter 读取 CharacterLevel；
	// 怪物同样从 DOCharacter 读取。Phase 3 后玩家等级可由 PlayerState 覆盖。
	UFUNCTION(BlueprintPure, Category = "DO|Ability")
	int32 GetCharacterLevel() const;

	// 统一伤害施加入口：创建伤害 GE 并施加到目标。
	// 伤害公式（暴击/命中/闪避/减免）由 DamageGEClass 的 ExecutionCalculation 负责；
	// 暴击/格挡等结果写回 FDOGameplayEffectContext 供客户端 GameplayCue 读取；
	// 吸血（Damage.CanLifeSteal）在 UDOHealthSet::PostGameplayEffectExecute 用真实伤害计算。
	UFUNCTION(BlueprintCallable, Category = "DO|Ability")
	void ApplyDamageToTarget(TSubclassOf<UGameplayEffect> DamageGEClass, AActor* Target,
		float SkillBaseDamage, float SkillDamageMultiplier, const FGameplayTagContainer& SourceTags, float Level = 1.0f);

protected:
	void TryActivateAbilitiesOnInput();
	bool DoesAbilitySpecMatchInputTag(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& InputTag) const;

protected:
	// 单帧输入缓存：每次 ProcessAbilityInput 结束后都会清空。
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// 持续输入缓存：按键或按钮保持按下时一直保留。
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// AbilityId -> SpecHandle 的映射。ASC 自己管理生命周期，切职业时一键 Clear。
	UPROPERTY()
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> AbilityIdToSpecHandle;

	// AbilityId -> 最大等级，授予时从 FDOAbilityGrant 写入，供升级校验使用。
	UPROPERTY()
	TMap<FGameplayTag, int32> AbilityIdToMaxLevel;
};