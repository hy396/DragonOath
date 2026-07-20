// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Interface/SharedCoolingInterface.h"
#include "DataType/SharedCoolingDataType.h"
#include "GA_SharedCoolingBase.generated.h"


UCLASS()
class SHAREDCOOLINGABILITY_API UGA_SharedCoolingBase : public UGameplayAbility
{
	GENERATED_BODY()
	friend void ISharedCoolingInterface::NotifyAllSharedAbilityRefreshCoolTime(FGameplayTagContainer , FGameplayAbilitySpecHandle);
public:
	UGA_SharedCoolingBase();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplySharedCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

private:

	/* enable the function of configuring shared cooling.*/
	UPROPERTY(EditDefaultsOnly, Category = "SharedCooling", meta = (AllowPrivateAccess = "true"))
	bool bEnableSharedCooling;
	/*
		Share the cooling tag and time, Apply shared cooling to all GA with this Tag.

		By default, self will also be affected by the shared cooldown effect.
		If you want to activate self without being affected by shared cooling, @see "bSelfActivateDontSharedCoolDefaultConfig" .
	*/
	UPROPERTY(EditDefaultsOnly, Category = "SharedCooling", meta = (AllowPrivateAccess = "true",EditCondition = "bEnableSharedCooling"))
	TMap<FGameplayTag,float> SharedCoolingTime;

	/* when activated by itself, it is not subject to the restriction of shared cooldown.*/
	UPROPERTY(EditDefaultsOnly, Category = "SharedCooling", meta = (AllowPrivateAccess = "true",EditCondition = "bEnableSharedCooling"))
	bool bSelfActivateDontSharedCoolDefaultConfig;	

	/* Notification policy for cooling events.*/
	UPROPERTY(EditDefaultsOnly, Category = "SharedCooling", meta = (AllowPrivateAccess = "true",EditCondition = "bEnableSharedCooling"))
	EEventNotifyPlicy EventNotifyPlicy;

	/* Use "Replicated" to support "LocalPredicted" */
	UPROPERTY(Replicated)
	mutable bool bSelfDontSharedCoolRuningSwitch;
	mutable FActiveGameplayEffectHandle MaxRemainingCoolTimeAGEHandle;
	mutable FDelegateHandle MaxRemainingCoolTime_RemoveDelegate;
	FGameplayTagContainer ValidSharedCoolingTag;

	
	void ExecCoolingUpdateNotifyEvent(FGameplayTag EventTag, FGameplayTag CoolingAssetTag, float Remaining, float Duration)const;
	void SendCoolingUpdateNotifyEvent(FGameplayTag EventTag, FGameplayTag CoolingAssetTag, float Remaining, float Duration)const;
	UFUNCTION(Client, Reliable)
	void Client_SendCoolingUpdateNotifyEvent(FGameplayTag EventTag, FGameplayTag CoolingAssetTag, float Remaining, float Duration)const;
	void RegisterCoolTimeGERemoveCallback() const;
	void RefreshSharedCoolAbilityTime(FGameplayAbilitySpecHandle InstigatorHandle = FGameplayAbilitySpecHandle());
	FActiveGameplayEffectHandle GetCurrentCooldownTimeActiveGameplayEffectHandle() const;
	FGameplayTag GetCurrentCoolingAssetTagByAGEHandle(const FActiveGameplayEffectHandle& ActiveGameplayEffectHandle) const;
	UFUNCTION(BlueprintPure, Category = "SharedCooling")
	void GetCooldownTimeRemainingAndDurationAndTag(FGameplayTag& CoolingAssetTag,float& TimeRemaining, float& CooldownDuration) const;
};
