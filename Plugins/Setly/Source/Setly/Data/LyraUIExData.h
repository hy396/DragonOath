// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UIExtensionSystem.h"

#include "LyraUIExData.generated.h"

class UUserWidget;

USTRUCT(BlueprintType)
struct SETLY_API FContentToLayerWidgetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	FGameplayTag LayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TSubclassOf<UCommonActivatableWidget> WidgetClass;
};

USTRUCT(BlueprintType)
struct SETLY_API FRegisterExtensionAsWidgetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	FGameplayTag SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	int32 Priority = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct SETLY_API FRegisterExtensionAsWidgetForContextEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	FGameplayTag SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TObjectPtr<UObject> ContextObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	int32 Priority = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct SETLY_API FRegisterExtensionPointEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	FGameplayTag SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	EUIExtensionPointMatch ExtensionPointTagMatchType = EUIExtensionPointMatch::ExactMatch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TArray<TSubclassOf<UObject>> AllowedDataClasses;
};

USTRUCT(BlueprintType)
struct SETLY_API FRegisterExtensionPointContextEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	FGameplayTag SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TObjectPtr<UObject> ContextObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	EUIExtensionPointMatch ExtensionPointTagMatchType = EUIExtensionPointMatch::ExactMatch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TArray<TSubclassOf<UObject>> AllowedDataClasses;
};

USTRUCT(BlueprintType)
struct SETLY_API FRegisterExtensionAsDataEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	FGameplayTag SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TObjectPtr<UObject> ContextObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	TObjectPtr<UObject> Data = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extensions")
	int32 Priority = INDEX_NONE;
};

UCLASS(BlueprintType)
class SETLY_API ULyraUIExData : public UDataAsset
{
	GENERATED_BODY()

public:
	ULyraUIExData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Extension")
	TArray<FRegisterExtensionAsWidgetEntry> WidgetEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Extension")
	TArray<FRegisterExtensionAsWidgetForContextEntry> WidgetForContexEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Extension")
	TArray<FRegisterExtensionPointEntry> PointEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Extension")
	TArray<FRegisterExtensionPointContextEntry> PointContextEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Extension")
	TArray<FRegisterExtensionAsDataEntry> DataExtensions;
};
