// Fill out your copyright notice in the Description page of Project Settings.

#include "DataType/SharedCoolingDataType.h"

FSharedCoolingNativeTags FSharedCoolingNativeTags::NativeTags;

USharedCoolingInfoObject* USharedCoolingInfoObject::GenerateSharedCoolingInfoObject(FGameplayAbilitySpecHandle InGASpecHandle, FGameplayTag InCoolingAssetTag, float InRemaining, float InDuration)
{
	USharedCoolingInfoObject* SharedCoolingInfoObject = NewObject<USharedCoolingInfoObject>(GetTransientPackage(), FName(TEXT("DynamicSharedCoolingInfoObject")));
	SharedCoolingInfoObject->SetSharedCoolingInfo(InGASpecHandle, InCoolingAssetTag,InRemaining, InDuration);
	return SharedCoolingInfoObject;
}

void USharedCoolingInfoObject::SetSharedCoolingInfo(FGameplayAbilitySpecHandle InGASpecHandle, FGameplayTag InCoolingAssetTag,float InRemaining, float InDuration)
{
	SharedCoolingInfo.GASpecHandle = InGASpecHandle;
	SharedCoolingInfo.CoolingAssetTag = InCoolingAssetTag;
	SharedCoolingInfo.Remaining = InRemaining;
	SharedCoolingInfo.Duration = InDuration;
}

FSharedCoolingInfo USharedCoolingInfoObject::GetSharedCoolingInfo() const
{
	return SharedCoolingInfo;
}
