// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "SharedCoolingDataType.generated.h"

#define TAG_EVENT_COOLING_START  FGameplayTag::RequestGameplayTag("Event.Cooling.Start")		//冷却开始
#define TAG_EVENT_COOLING_END    FGameplayTag::RequestGameplayTag("Event.Cooling.End")		//冷却结束


UENUM(BlueprintType)
enum class EEventNotifyPlicy : uint8 
{
    OnlyClient        UMETA(DisplayName = "只通知客户端"),
    OnlyServer        UMETA(DisplayName = "只通知服务端"),
	AllBoth			  UMETA(DisplayName = "服务单与服务端"),
};
USTRUCT(BlueprintType)
struct SHAREDCOOLINGABILITY_API FSharedCoolingInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "SharedCooling")
		FGameplayAbilitySpecHandle GASpecHandle;

	UPROPERTY(BlueprintReadOnly, Category = "SharedCooling")
		FGameplayTag CoolingAssetTag;

	UPROPERTY(BlueprintReadOnly, Category = "SharedCooling")
		float Remaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "SharedCooling")
		float Duration = 0.f;
};

UCLASS(BlueprintType)
class USharedCoolingInfoObject : public UObject
{
	GENERATED_BODY()
public:
	static USharedCoolingInfoObject* GenerateSharedCoolingInfoObject(FGameplayAbilitySpecHandle InGASpecHandle, FGameplayTag InCoolingAssetTag, float InRemaining, float InDuration);

	void SetSharedCoolingInfo(FGameplayAbilitySpecHandle InGASpecHandle, FGameplayTag InCoolingTag, float InRemaining, float InDuration);
	UFUNCTION(BlueprintPure, Category = "SharedCooling")
	FSharedCoolingInfo GetSharedCoolingInfo() const;
private:
	FSharedCoolingInfo SharedCoolingInfo;
};

//Register Add tag
struct SHAREDCOOLINGABILITY_API FSharedCoolingNativeTags : public FGameplayTagNativeAdder
{
	/*
		有些不喜欢用FGameplayTag::RequestGameplayTag去访问Tag。
		所以提供直接方式访问:
			FSharedCoolingNativeTags::Get().CoolingStart
			FSharedCoolingNativeTags::Get().CoolingEnd
	*/
	FGameplayTag CoolingStart;
	FGameplayTag CoolingEnd;

	virtual void AddTags() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

		CoolingStart = Manager.AddNativeGameplayTag(TEXT("Event.Cooling.Start"));
		CoolingEnd = Manager.AddNativeGameplayTag(TEXT("Event.Cooling.End"));
	}

	FORCEINLINE static const FSharedCoolingNativeTags& Get() { return NativeTags; }
private:
	static FSharedCoolingNativeTags NativeTags;
};

