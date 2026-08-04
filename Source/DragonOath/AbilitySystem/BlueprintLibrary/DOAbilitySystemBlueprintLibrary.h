// Copyright DragonOath. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DOAbilitySystemBlueprintLibrary.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;

/** GameplayEffect 标签查询的匹配方式。 */
UENUM(BlueprintType)
enum class EDOTagsQueryCondition : uint8
{
	/** 命中任意一个标签即可。 */
	MatchAny UMETA(DisplayName = "匹配任意标签"),

	/** 必须同时命中全部标签。 */
	MatchAll UMETA(DisplayName = "匹配全部标签"),
};

/**
 * DragonOath 的 GAS 蓝图辅助函数库。
 *
 * 这里集中放置项目实际使用的通用 GAS 查询、技能实例访问和冷却时间修改接口，
 * 避免玩法蓝图直接依赖 SharedCoolingAbility 插件的工具函数库。
 */
UCLASS()
class DRAGONOATH_API UDOAbilitySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 将 ActiveGameplayEffectHandle 转换为调试字符串。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO ToString (ActiveGameplayEffectHandle)", BlueprintAutocast), Category = "DO|AbilitySystem")
	static FString Conv_ActiveGameplayEffectHandleToString(const FActiveGameplayEffectHandle& Handle);

	/** 比较两个 AbilitySpecHandle 是否指向同一个技能授予记录。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO Equal (GameplayAbilitySpecHandle)", CompactNodeTitle = "==", Keywords = "== equal"), Category = "DO|AbilitySystem")
	static bool EqualEqual_GameplayAbilitySpecHandle(const FGameplayAbilitySpecHandle& A, const FGameplayAbilitySpecHandle& B);

	/** 将 GameplayAbilitySpecHandle 转换为调试字符串。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO ToString (GameplayAbilitySpecHandle)", BlueprintAutocast), Category = "DO|AbilitySystem")
	static FString Conv_GameplayAbilitySpecHandleToString(const FGameplayAbilitySpecHandle& Handle);

	/**
	 * 在服务器上向 ASC 授予一个技能。
	 *
	 * 需要输入标签授予时，应优先使用 UDOAbilitySystemComponent::GiveDOAbility，
	 * 这里保留通用接口用于少量原型或工具蓝图。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "DO Give Ability"), Category = "DO|AbilitySystem")
	static FGameplayAbilitySpecHandle GiveAbility(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1, int32 InputID = -1);

	/** 尝试通过 SpecHandle 激活技能，支持客户端预测和远程激活。 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DO TryActivateAbilityByHandle"), Category = "DO|AbilitySystem")
	static bool TryActivateAbilityByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& AbilityToActivate, bool bAllowRemoteActivation = true);

	/** 在服务器上清除技能；可选择等技能结束后再移除。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "DO Clear Ability"), Category = "DO|AbilitySystem")
	static void ClearAbility(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle, bool bRemoveAbilityOnEnd);

	/**
	 * 修改指定 GameplayEffect 类实例的剩余时间。
	 * ModifiedIncrement 为负数时减少剩余时间，为正数时延长剩余时间。
	 * 当减少量超过剩余时间时，GameplayEffect 会由 GAS 正常结束。
	 *
	 * @return 实际修改成功的 ActiveGameplayEffect 数量。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "DO ModifyGameplayEffectRemainingTimeByClass"), Category = "DO|AbilitySystem")
	static int32 ModifyGameplayEffectRemainingTimeByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayEffect> EffectClass, float ModifiedIncrement);

	/**
	 * 修改指定 ActiveGameplayEffect 的剩余时间。
	 * 该操作本质上是调整 GameplayEffect 的开始时间，因此会由 GAS 负责复制和生命周期处理。
	 *
	 * @return Handle 有效且修改成功时返回 true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "DO ModifyGameplayEffectRemainingTimeByHandle"), Category = "DO|AbilitySystem")
	static bool ModifyGameplayEffectRemainingTimeByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle, float ModifiedIncrement);

	/**
	 * 修改带有指定效果标签的全部 GameplayEffect 实例的剩余时间。
	 * MatchAny 会匹配任意一个标签，MatchAll 要求全部标签都存在。
	 *
	 * @return 实际修改成功的 ActiveGameplayEffect 数量。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "DO ModifyGameplayEffectRemainingTimeByTags"), Category = "DO|AbilitySystem")
	static int32 ModifyGameplayEffectRemainingTimeByTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& Tags, float ModifiedIncrement, EDOTagsQueryCondition QueryCondition);

	/** 获取指定 GameplayEffect 的总持续时间。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO GetGameplayEffectDuration"), Category = "DO|AbilitySystem")
	static float GetGameplayEffectDuration(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle);

	/** 获取指定 GameplayEffect 的开始时间和总持续时间。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO GetGameplayEffectStartTimeAndDuration"), Category = "DO|AbilitySystem")
	static void GetGameplayEffectStartTimeAndDuration(UAbilitySystemComponent* AbilitySystemComponent, const FActiveGameplayEffectHandle& Handle, float& StartEffectTime, float& Duration);

	/** 根据技能授予句柄查询当前冷却剩余时间和总时长。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO GetAbilityCooldownTimeRemainingAndDurationByHandle"), Category = "DO|AbilitySystem")
	static void GetAbilityCooldownTimeRemainingAndDurationByHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle, float& TimeRemaining, float& CooldownDuration);

	/** 根据技能实例查询当前冷却剩余时间和总时长。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO GetAbilityCooldownTimeRemainingAndDurationByAbility"), Category = "DO|AbilitySystem")
	static void GetAbilityCooldownTimeRemainingAndDurationByAbility(UAbilitySystemComponent* AbilitySystemComponent, UGameplayAbility* Ability, float& TimeRemaining, float& CooldownDuration);

	/** 根据技能类获取该 ASC 上的主技能实例。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO GetPrimaryAbilityInstanceFromClass"), Category = "DO|AbilitySystem")
	static UGameplayAbility* GetPrimaryAbilityInstanceFromClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass);

	/** 根据技能授予句柄获取该 ASC 上的主技能实例。 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "DO GetPrimaryAbilityInstanceFromHandle"), Category = "DO|AbilitySystem")
	static UGameplayAbility* GetPrimaryAbilityInstanceFromHandle(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle);
};
