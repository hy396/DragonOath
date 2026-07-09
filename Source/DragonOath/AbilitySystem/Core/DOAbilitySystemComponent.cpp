// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/DOGameplayAbility.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystemGlobals.h"
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

	// 让已激活技能能响应“松开释放”“蓄力松手”等玩法。
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
