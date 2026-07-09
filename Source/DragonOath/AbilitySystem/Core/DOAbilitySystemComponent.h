// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DOAbilitySystemComponent.generated.h"

/**
 * DragonOath 的 AbilitySystemComponent。
 *
 * ASC 是 GAS 的核心运行时对象：保存已授予技能、GameplayEffect、GameplayTag 和属性聚合结果。
 * 本项目在这里集中处理“输入标签 -> AbilitySpecHandle -> TryActivateAbility”的转换。
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

protected:
	void TryActivateAbilitiesOnInput();
	bool DoesAbilitySpecMatchInputTag(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& InputTag) const;

protected:
	// 单帧输入缓存：每次 ProcessAbilityInput 结束后都会清空。
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// 持续输入缓存：按键或按钮保持按下时一直保留。
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
