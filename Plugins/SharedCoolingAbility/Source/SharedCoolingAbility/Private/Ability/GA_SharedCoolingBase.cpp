// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GA_SharedCoolingBase.h"
#include "Interface/SharedCoolingInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect/GE_SharedCooling.h"
#include "Net/UnrealNetwork.h"

UGA_SharedCoolingBase::UGA_SharedCoolingBase()
{
	//Currently, only InstancedPerActor is supported for SharedCooling
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	EventNotifyPlicy = EEventNotifyPlicy::OnlyClient;
}

void UGA_SharedCoolingBase::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	ISharedCoolingInterface* SharedCoolingSystem = Cast<ISharedCoolingInterface>(ActorInfo->AbilitySystemComponent);
	check(SharedCoolingSystem && "AbilitySystemComponent must be inherited from the ISharedCoolingInterface class.");

	if (bEnableSharedCooling && SharedCoolingTime.Num() > 0)
	{
		TArray<FGameplayTag> SharedCoolingTags;
		SharedCoolingTime.GenerateKeyArray(SharedCoolingTags);
		for (const FGameplayTag& SharedCoolingTag : SharedCoolingTags)
		{
			if (*SharedCoolingTime.Find(SharedCoolingTag) > 0.f)
			{
				ValidSharedCoolingTag.AddTag(SharedCoolingTag);
				SharedCoolingSystem->RegisterSharedCoolingAbilities(SharedCoolingTag, Spec.Handle);
			}
		}
	}
	
	if (UGA_SharedCoolingBase* Ability = Cast<UGA_SharedCoolingBase>(Spec.GetPrimaryInstance()))
	{
		Ability->RegisterCoolTimeGERemoveCallback();
	}
}

void UGA_SharedCoolingBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	ISharedCoolingInterface* SharedCoolingSystem = Cast<ISharedCoolingInterface>(ActorInfo->AbilitySystemComponent);
	check(SharedCoolingSystem && "AbilitySystemComponent must be inherited from the ISharedCoolingSystemInterface class.");

	if (bEnableSharedCooling && ValidSharedCoolingTag.IsValid())
	{
		for (TArray<FGameplayTag>::TConstIterator Ite = ValidSharedCoolingTag.CreateConstIterator(); Ite; Ite++)
		{
			SharedCoolingSystem->CancelSharedCoolingAbilities(*Ite, Spec.Handle);
		}
	}
}

const FGameplayTagContainer* UGA_SharedCoolingBase::GetCooldownTags() const
{
	if (!bSelfDontSharedCoolRuningSwitch)
	{
		FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&ValidSharedCoolingTag);
		if (const FGameplayTagContainer* ParentTags = Super::GetCooldownTags())
		{
			MutableTags->AppendTags(*ParentTags);
		}
		return MutableTags;
	}

	return Super::GetCooldownTags();
}

void UGA_SharedCoolingBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//[apply default cooldown]
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		// apply default cooldown GameplayEffect  
		FActiveGameplayEffectHandle DefaultCoolHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CooldownGE, GetAbilityLevel(Handle, ActorInfo));
		 
		//change cooldown time callback
		if (FOnActiveGameplayEffectTimeChange* TimeChangeDelegate = GetAbilitySystemComponentFromActorInfo()->OnGameplayEffectTimeChangeDelegate(DefaultCoolHandle))
		{
			TimeChangeDelegate->AddWeakLambda(const_cast<UGA_SharedCoolingBase*>(this), [&](FActiveGameplayEffectHandle ActiveGameplayEffectHandle, float NewStartTime, float NewDuration)
				{
					const_cast<UGA_SharedCoolingBase*>(this)->RefreshSharedCoolAbilityTime();
				});
		}


		if (!bEnableSharedCooling && K2_HasAuthority() && DefaultCoolHandle.IsValid())
		{
			//remove cooldown callback
			if (FOnActiveGameplayEffectRemoved_Info* DefaultCool_Remove = GetAbilitySystemComponentFromActorInfo()->OnGameplayEffectRemoved_InfoDelegate(DefaultCoolHandle))
			{
				DefaultCool_Remove->AddWeakLambda(const_cast<UGA_SharedCoolingBase*>(this),[&, DefaultCoolHandle](const FGameplayEffectRemovalInfo& InGameplayEffectRemovalInfo)
					{
						//notify client widget cooldown end.
						ExecCoolingUpdateNotifyEvent(TAG_EVENT_COOLING_END, GetCurrentCoolingAssetTagByAGEHandle(DefaultCoolHandle), 0.f, 0.f);
					});
			}

			//notify client widget cooldown start.
			float Remaining, Duration = 0.f;
			GetCooldownTimeRemainingAndDuration(Handle, ActorInfo, Remaining, Duration);
			if (Duration > 0.f && Remaining > 0.f)
			{
				ExecCoolingUpdateNotifyEvent(TAG_EVENT_COOLING_START, GetCurrentCoolingAssetTagByAGEHandle(DefaultCoolHandle), Remaining, Duration);
			}
		}
	}

	//[apply shared cooldown]
	if (K2_HasAuthority() && bEnableSharedCooling && ValidSharedCoolingTag.IsValid())
	{
		ApplySharedCooldown(Handle, ActorInfo, ActivationInfo);
	}
}

void UGA_SharedCoolingBase::ApplySharedCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//shared cooling skill
	if (ISharedCoolingInterface* SharedCoolingSystem = Cast<ISharedCoolingInterface>(ActorInfo->AbilitySystemComponent.Get()))
	{
		for (TArray<FGameplayTag>::TConstIterator TagIte = ValidSharedCoolingTag.CreateConstIterator(); TagIte; TagIte++)
		{
			if (SharedCoolingTime.Contains(*TagIte))
			{
				float Duration = *SharedCoolingTime.Find(*TagIte);
				if (Duration > 0.f)
				{
					FGameplayEffectSpecHandle SharedCoolingSpecHandle = MakeOutgoingGameplayEffectSpec(UGE_SharedCooling::StaticClass(), GetAbilityLevel());
					if (FGameplayEffectSpec* Spec = SharedCoolingSpecHandle.Data.Get())
					{
						Spec->SetDuration(Duration, true);

						Spec->DynamicGrantedTags.AddTag(*TagIte);
#if ENGINE_MAJOR_VERSION == 5 
						Spec->AddDynamicAssetTag(*TagIte);

#else
						Spec->DynamicAssetTags.AddTag(*TagIte);
#endif
					}
					FActiveGameplayEffectHandle AGEHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SharedCoolingSpecHandle);

					if (FOnActiveGameplayEffectTimeChange* TimeChangeDelegate = GetAbilitySystemComponentFromActorInfo()->OnGameplayEffectTimeChangeDelegate(AGEHandle))
					{
						TimeChangeDelegate->AddWeakLambda(const_cast<UGA_SharedCoolingBase*>(this), [&, SharedCoolingSystem](FActiveGameplayEffectHandle ActiveGameplayEffectHandle, float NewStartTime, float NewDuration)
							{
								if (SharedCoolingSystem)
								{
									SharedCoolingSystem->NotifyAllSharedAbilityRefreshCoolTime(ValidSharedCoolingTag, GetCurrentAbilitySpecHandle());
								}
							});
					}
				}
			}
		}
		//They are not restricted(限制) by shared cooling.
		if (bSelfActivateDontSharedCoolDefaultConfig)
		{
			bSelfDontSharedCoolRuningSwitch = true;
		}
		SharedCoolingSystem->NotifyAllSharedAbilityRefreshCoolTime(ValidSharedCoolingTag, GetCurrentAbilitySpecHandle());
	}
	else
	{
		check(SharedCoolingSystem && "AbilitySystemComponent must be inherited from the ISharedCoolingSystemInterface class.");
	}
}

void UGA_SharedCoolingBase::RefreshSharedCoolAbilityTime(FGameplayAbilitySpecHandle InstigatorHandle /*= FGameplayAbilitySpecHandle()*/)
{
	float Duration;
	float Remaining;
	FGameplayTag Tag;

	if (bSelfActivateDontSharedCoolDefaultConfig && InstigatorHandle.IsValid())
	{
		if (InstigatorHandle != GetCurrentAbilitySpecHandle())
		{
			bSelfDontSharedCoolRuningSwitch = false;
		}
	}

	RegisterCoolTimeGERemoveCallback();

	GetCooldownTimeRemainingAndDurationAndTag(Tag, Remaining, Duration);
	if (Remaining > 0.f && Duration > 0.f)
	{
		ExecCoolingUpdateNotifyEvent(TAG_EVENT_COOLING_START, Tag/*GetCurrentCoolingAssetTagByAGEHandle(MaxRemainingCoolTimeAGEHandle)*/, Remaining, Duration);
	}
}

void UGA_SharedCoolingBase::ExecCoolingUpdateNotifyEvent(FGameplayTag EventTag, FGameplayTag CoolingAssetTag, float Remaining, float Duration) const
{
	if (EventNotifyPlicy == EEventNotifyPlicy::OnlyClient)
	{
		Client_SendCoolingUpdateNotifyEvent(EventTag, CoolingAssetTag, Remaining, Duration);
	}
	else if (EventNotifyPlicy == EEventNotifyPlicy::OnlyServer)
	{
		SendCoolingUpdateNotifyEvent(EventTag, CoolingAssetTag, Remaining, Duration);
	}
	else
	{
		SendCoolingUpdateNotifyEvent(EventTag, CoolingAssetTag, Remaining, Duration);
		Client_SendCoolingUpdateNotifyEvent(EventTag, CoolingAssetTag, Remaining, Duration);
	}
}

void UGA_SharedCoolingBase::SendCoolingUpdateNotifyEvent(FGameplayTag EventTag, FGameplayTag CoolingAssetTag, float Remaining, float Duration) const
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEventData GameplayEventData;
		GameplayEventData.EventTag = EventTag;
		GameplayEventData.OptionalObject = USharedCoolingInfoObject::GenerateSharedCoolingInfoObject(GetCurrentAbilitySpecHandle(), CoolingAssetTag, Remaining, Duration);
		FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(EventTag, &GameplayEventData);
	}
}

void UGA_SharedCoolingBase::Client_SendCoolingUpdateNotifyEvent_Implementation(FGameplayTag EventTag, FGameplayTag CoolingAssetTag, float Remaining, float Duration) const
{
	SendCoolingUpdateNotifyEvent(EventTag, CoolingAssetTag, Remaining, Duration);
}


void UGA_SharedCoolingBase::RegisterCoolTimeGERemoveCallback() const
{
	// Find the AGEHandle with the largest remaining value from default cooling and shared cooling.
	FActiveGameplayEffectHandle NewCoolTimeAGEHandle = GetCurrentCooldownTimeActiveGameplayEffectHandle();
	if (MaxRemainingCoolTimeAGEHandle != NewCoolTimeAGEHandle)
	{
		if (FOnActiveGameplayEffectRemoved_Info* OldActiveSharedCoold_Remove = GetAbilitySystemComponentFromActorInfo()->OnGameplayEffectRemoved_InfoDelegate(MaxRemainingCoolTimeAGEHandle))
		{
			OldActiveSharedCoold_Remove->Remove(MaxRemainingCoolTime_RemoveDelegate);
		}
		// The AGEHandle with the maximum remaining time is saved,and listen for the callback when the cooling ends.
		MaxRemainingCoolTimeAGEHandle = NewCoolTimeAGEHandle;
		if (FOnActiveGameplayEffectRemoved_Info* NewActiveSharedCoold_Remove = GetAbilitySystemComponentFromActorInfo()->OnGameplayEffectRemoved_InfoDelegate(NewCoolTimeAGEHandle))
		{
			MaxRemainingCoolTime_RemoveDelegate = NewActiveSharedCoold_Remove->AddWeakLambda(const_cast<UGA_SharedCoolingBase*>(this), [&](const FGameplayEffectRemovalInfo& InGameplayEffectRemovalInfo)
				{
					ExecCoolingUpdateNotifyEvent(TAG_EVENT_COOLING_END, GetCurrentCoolingAssetTagByAGEHandle(MaxRemainingCoolTimeAGEHandle), 0.f, 0.f);
				});
		}
	}
}

FActiveGameplayEffectHandle UGA_SharedCoolingBase::GetCurrentCooldownTimeActiveGameplayEffectHandle() const
{
	const FGameplayTagContainer* CooldownTags = GetCooldownTags();
	if (CooldownTags && CooldownTags->Num() > 0)
	{
		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
		TArray<FActiveGameplayEffectHandle> AllCoolingTimeGEHandle = GetAbilitySystemComponentFromActorInfo()->GetActiveEffects(Query);

		float MaxRemaining = 0;
		float index = -1;
		float CurrentTime = GetWorld()->GetTimeSeconds();
		for (int i = 0; i < AllCoolingTimeGEHandle.Num(); i++)
		{
			float StartEffectTime;
			float Duration;
			GetAbilitySystemComponentFromActorInfo()->GetGameplayEffectStartTimeAndDuration(AllCoolingTimeGEHandle[i], StartEffectTime, Duration);
			float Elapsed = CurrentTime - StartEffectTime;
			float Remaining = Duration - Elapsed;

			if (Remaining > MaxRemaining)
			{
				MaxRemaining = Remaining;
				index = i;
			}
		}
		return index > -1 ? AllCoolingTimeGEHandle[index] : FActiveGameplayEffectHandle{};
	}
	return FActiveGameplayEffectHandle{};
}

FGameplayTag UGA_SharedCoolingBase::GetCurrentCoolingAssetTagByAGEHandle(const FActiveGameplayEffectHandle& ActiveGameplayEffectHandle) const
{
	if(const FActiveGameplayEffect* ActiveGameplayEffect = GetAbilitySystemComponentFromActorInfo()->GetActiveGameplayEffect(ActiveGameplayEffectHandle))
	{
		FGameplayTagContainer TagContainer;
		ActiveGameplayEffect->Spec.GetAllGrantedTags(TagContainer);
		
		return TagContainer.GetByIndex(0);
	}
	return FGameplayTag();
}

void UGA_SharedCoolingBase::GetCooldownTimeRemainingAndDurationAndTag(FGameplayTag& CoolingAssetTag, float& TimeRemaining, float& CooldownDuration) const
{
	CoolingAssetTag = GetCurrentCoolingAssetTagByAGEHandle(MaxRemainingCoolTimeAGEHandle);
	GetCooldownTimeRemainingAndDuration(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), TimeRemaining, CooldownDuration);
}

void UGA_SharedCoolingBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGA_SharedCoolingBase, bSelfDontSharedCoolRuningSwitch);
}