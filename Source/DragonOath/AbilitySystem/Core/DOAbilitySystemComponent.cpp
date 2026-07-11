// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/DOGameplayAbility.h"
#include "AbilitySystem/Abilities/DOAbilitySet.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Characters/DOCharacter.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "DOLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOAbilitySystemComponent)

UDOAbilitySystemComponent::UDOAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

UDOAbilitySystemComponent* UDOAbilitySystemComponent::GetFromActor(const AActor* Actor, const bool bLookForComponent)
{
	return Cast<UDOAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, bLookForComponent));
}

void UDOAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// WaitInputPress 等 AbilityTask 监听的是 GAS 的通用复制事件。
	// 这里沿用 Lyra 的做法，不依赖 bReplicateInputDirectly。
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
		const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
	}
}

void UDOAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// 让已激活技能能响应"松开释放""蓄力松手"等玩法。
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
		const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
	}
}

void UDOAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// 把对外的输入标签转换成 GAS 内部的 AbilitySpecHandle。
	// 多个技能共用同一个输入标签时，也能逐个缓存并处理。
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		// Level <= 0 的技能尚未学习，不参与输入匹配，避免多余的激活尝试和失败日志。
		if (AbilitySpec.Level <= 0)
		{
			continue;
		}

		if (DoesAbilitySpecMatchInputTag(AbilitySpec, InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UDOAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// 松开时只记录 Released，并从 Held 中移除；真正通知技能仍然放在本帧统一处理。
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (DoesAbilitySpecMatchInputTag(AbilitySpec, InputTag))
		{
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}
}

void UDOAbilitySystemComponent::ProcessAbilityInput(const float DeltaTime, const bool bGamePaused)
{
	// 暂停或输入被屏蔽时清空缓存，避免菜单关闭后旧的按键状态立刻触发技能。
	if (bGamePaused || HasMatchingGameplayTag(DragonOathGameplayTags::Gameplay::AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	// Pressed / Held 负责激活技能；Released 在激活之后处理。
	// 这样同一帧刚激活的技能，也有机会收到后续的 Release 事件。
	TryActivateAbilitiesOnInput();

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilitySpec->InputPressed = false;
		if (AbilitySpec->IsActive())
		{
			AbilitySpecInputReleased(*AbilitySpec);
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UDOAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UDOAbilitySystemComponent::TryActivateAbilitiesOnInput()
{
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// Held 适合持续按住类技能，例如冲刺、引导、瞄准、防御。
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability || AbilitySpec->IsActive())
		{
			continue;
		}

		const UDOGameplayAbility* DOAbility = Cast<UDOGameplayAbility>(AbilitySpec->Ability);
		if (DOAbility && DOAbility->GetActivationPolicy() == EDOAbilityActivationPolicy::WhileInputActive)
		{
			AbilitiesToActivate.AddUnique(SpecHandle);
		}
	}

	// Pressed 适合点按触发技能；如果技能已经激活，也会继续把按下事件传进去。
	// 连招窗口、多段点击等玩法可以在 Ability 内部处理。
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilitySpec->InputPressed = true;
		if (AbilitySpec->IsActive())
		{
			AbilitySpecInputPressed(*AbilitySpec);
			continue;
		}

		const UDOGameplayAbility* DOAbility = Cast<UDOGameplayAbility>(AbilitySpec->Ability);
		if (DOAbility && DOAbility->GetActivationPolicy() == EDOAbilityActivationPolicy::OnInputTriggered)
		{
			AbilitiesToActivate.AddUnique(SpecHandle);
		}
	}

	// 扫描完输入后再统一激活，避免某个技能激活时修改 Ability 列表影响遍历。
	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToActivate)
	{
		const bool bActivated = TryActivateAbility(SpecHandle);
		if (!bActivated)
		{
			UE_LOG(LogDragonOathAbilitySystem, Verbose, TEXT("Ability input spec %s failed to activate on %s"),
				*SpecHandle.ToString(), *GetNameSafe(GetOwner()));
		}
	}
}

bool UDOAbilitySystemComponent::DoesAbilitySpecMatchInputTag(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& InputTag) const
{
	if (!AbilitySpec.Ability || !InputTag.IsValid())
	{
		return false;
	}

	if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
	{
		return true;
	}

	// 正式授予技能时优先把输入标签放在 DynamicAbilityTags。
	// 读取 AbilityTags 只是为了早期蓝图技能更容易调试和迁移。
	if (const UDOGameplayAbility* DOAbility = Cast<UDOGameplayAbility>(AbilitySpec.Ability))
	{
		return DOAbility->GetAssetTags().HasTagExact(InputTag);
	}

	return false;
}

void UDOAbilitySystemComponent::CancelAbilitiesByTag(const FGameplayTagContainer& WithTags, const FGameplayTagContainer& WithoutTags, UGameplayAbility* IgnoreAbility)
{
	CancelAbilities(&WithTags, &WithoutTags, IgnoreAbility);
}

FGameplayAbilitySpecHandle UDOAbilitySystemComponent::GiveDOAbility(const FDOAbilityGrant& Grant)
{
	if (!Grant.AbilityClass)
	{
		UE_LOG(LogDragonOathAbilitySystem, Warning, TEXT("GiveDOAbility: AbilityClass is null for AbilityId %s."), *Grant.AbilityId.ToString());
		return FGameplayAbilitySpecHandle();
	}

	// 按 AbilityId 去重，防止重复授予
	if (AbilityIdToSpecHandle.Contains(Grant.AbilityId))
	{
		UE_LOG(LogDragonOathAbilitySystem, Verbose, TEXT("GiveDOAbility: AbilityId %s already granted, skipping."), *Grant.AbilityId.ToString());
		return AbilityIdToSpecHandle[Grant.AbilityId];
	}

	FGameplayAbilitySpec Spec(Grant.AbilityClass, Grant.InitialLevel);

	// 输入标签放入 DynamicSpecSourceTags，供 DoesAbilitySpecMatchInputTag 匹配
	if (Grant.InputTag.IsValid())
	{
		Spec.GetDynamicSpecSourceTags().AddTag(Grant.InputTag);
	}

	FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);

	AbilityIdToSpecHandle.Add(Grant.AbilityId, Handle);
	AbilityIdToMaxLevel.Add(Grant.AbilityId, Grant.MaxLevel);

	return Handle;
}

void UDOAbilitySystemComponent::GiveDOAbilitySet(const UDOAbilitySet* AbilitySet)
{
	if (!AbilitySet)
	{
		return;
	}

	for (const FDOAbilityGrant& Grant : AbilitySet->GrantedAbilities)
	{
		GiveDOAbility(Grant);
	}
}

bool UDOAbilitySystemComponent::SetDOAbilityLevel(FGameplayTag AbilityId, int32 NewLevel)
{
	// 必须在服务端执行，客户端不能直接修改 Level
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogDragonOathAbilitySystem, Warning, TEXT("SetDOAbilityLevel: Must be called on server."));
		return false;
	}

	const FGameplayAbilitySpecHandle* HandlePtr = AbilityIdToSpecHandle.Find(AbilityId);
	if (!HandlePtr)
	{
		UE_LOG(LogDragonOathAbilitySystem, Warning, TEXT("SetDOAbilityLevel: AbilityId %s not found."), *AbilityId.ToString());
		return false;
	}

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(*HandlePtr);
	if (!Spec)
	{
		return false;
	}

	const int32 OldLevel = Spec->Level;

	// 1+ -> 0：先取消正在运行的技能，让 EndAbility 清理它产生的持续效果
	if (OldLevel > 0 && NewLevel == 0)
	{
		if (Spec->IsActive())
		{
			CancelAbility(Spec->Ability);
		}
	}

	// 修改技能等级并标记 Spec 脏，触发客户端同步与等级变化广播。
	Spec->Level = NewLevel;
	MarkAbilitySpecDirty(*Spec);

	// 0 -> 1+：OnSpawn 被动技能需要自动激活
	// OnGiveAbility 只在首次授予时调用，升级时不会触发，这里手动激活
	if (OldLevel == 0 && NewLevel > 0)
	{
		if (const UDOGameplayAbility* DOAbility = Cast<UDOGameplayAbility>(Spec->Ability))
		{
			if (DOAbility->GetActivationPolicy() == EDOAbilityActivationPolicy::OnSpawn)
			{
				TryActivateAbility(*HandlePtr);
			}
		}
	}

	// 广播等级变化（权威端）。客户端通过 GAS 复制收到新 Level 后，
	// 监听 AbilitySpecDirtiedCallbacks 并转 Message.UI.Skill.LevelChanged 刷新 UI。
	OnAbilityLevelChanged.Broadcast(AbilityId, OldLevel, NewLevel);

	return true;
}

void UDOAbilitySystemComponent::Server_RequestAbilityLevel_Implementation(FGameplayTag AbilityId, int32 NewLevel)
{
	// 完整权威校验：技能必须已授予
	const FGameplayAbilitySpecHandle* HandlePtr = AbilityIdToSpecHandle.Find(AbilityId);
	if (!HandlePtr)
	{
		UE_LOG(LogDragonOathAbilitySystem, Warning, TEXT("Server_RequestAbilityLevel: AbilityId %s not granted."), *AbilityId.ToString());
		return;
	}

	// 等级区间校验：1 <= NewLevel <= MaxLevel（MaxLevel 来自授予配置）
	const int32* MaxLevelPtr = AbilityIdToMaxLevel.Find(AbilityId);
	const int32 MaxLevel = MaxLevelPtr ? *MaxLevelPtr : 1;
	if (NewLevel <= 0 || NewLevel > MaxLevel)
	{
		UE_LOG(LogDragonOathAbilitySystem, Warning, TEXT("Server_RequestAbilityLevel: NewLevel %d out of range [1, %d] for %s."), NewLevel, MaxLevel, *AbilityId.ToString());
		return;
	}

	// 等级无变化则跳过，避免空转
	if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(*HandlePtr))
	{
		if (Spec->Level == NewLevel)
		{
			return;
		}
	}

	// TODO: 升级消耗与前置校验接入。
	// 升级所需资源（技能点等）不一定放在 GAS 内，后续由独立的资源/技能树系统（SkillTreeComponent 或外部服务）提供
	// 消耗计算与前置判定。届时在此处调用资源系统扣除，并在资源不足/前置未满足时直接 return 拒绝升级。
	// 当前版本先放行，仅做等级区间与已授予校验。

	SetDOAbilityLevel(AbilityId, NewLevel);
}

bool UDOAbilitySystemComponent::Server_RequestAbilityLevel_Validate(FGameplayTag AbilityId, int32 NewLevel)
{
	// 基础合法性（具体区间与资源由 _Implementation 权威判定）：Tag 有效、目标等级为正。
	return AbilityId.IsValid() && NewLevel > 0;
}

int32 UDOAbilitySystemComponent::GetDOAbilityLevel(FGameplayTag AbilityId) const
{
	const FGameplayAbilitySpecHandle* HandlePtr = AbilityIdToSpecHandle.Find(AbilityId);
	if (!HandlePtr)
	{
		return 0;
	}

	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(*HandlePtr);
	return Spec ? Spec->Level : 0;
}

void UDOAbilitySystemComponent::CancelAbilityById(FGameplayTag AbilityId)
{
	const FGameplayAbilitySpecHandle* HandlePtr = AbilityIdToSpecHandle.Find(AbilityId);
	if (!HandlePtr)
	{
		return;
	}

	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(*HandlePtr);
	if (Spec && Spec->IsActive())
	{
		CancelAbility(Spec->Ability);
	}
}

void UDOAbilitySystemComponent::ClearDOAbilities()
{
	for (auto& Pair : AbilityIdToSpecHandle)
	{
		ClearAbility(Pair.Value);
	}

	AbilityIdToSpecHandle.Reset();
	AbilityIdToMaxLevel.Reset();
}

int32 UDOAbilitySystemComponent::GetCharacterLevel() const
{
	if (const AActor* Avatar = GetAvatarActor())
	{
		if (const ADOCharacter* DOCharacter = Cast<ADOCharacter>(Avatar))
		{
			return DOCharacter->GetCharacterLevel();
		}
	}
	return 1;
}

void UDOAbilitySystemComponent::ApplyDamageToTarget(TSubclassOf<UGameplayEffect> DamageGEClass, AActor* Target,
	float SkillBaseDamage, float SkillDamageMultiplier, const FGameplayTagContainer& SourceTags, float Level)
{
	if (!DamageGEClass || !Target)
	{
		return;
	}

	// 通过 IAbilitySystemInterface 兼容玩家（ASC 在 PlayerState）和怪物（ASC 在自身）。
	UAbilitySystemComponent* TargetASC = nullptr;
	if (const IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target))
	{
		TargetASC = TargetASI->GetAbilitySystemComponent();
	}
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DamageGEClass, Level, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	// 技能参数通过 SetByCaller 传给伤害 GE 的 ExecutionCalculation。
	SpecHandle.Data->SetSetByCallerMagnitude(DragonOathGameplayTags::Data::Damage, SkillBaseDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(DragonOathGameplayTags::Data::DamageMultiplier, SkillDamageMultiplier);

	ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	// 吸血不在本函数处理：伤害 GE 施加在目标上，无法为来源直接回血。
	// 吸血改为在 UDOHealthSet::PostGameplayEffectExecute 内用真实伤害 LocalDamage 计算
	// （LifeStealRate * LocalDamage），仅服务端执行，数据正确且天然避免客户端双重回血。
}