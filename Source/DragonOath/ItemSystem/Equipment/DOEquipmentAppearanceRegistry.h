#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

#include "DOEquipmentAppearanceRegistry.generated.h"

class AActor;
class UMaterialInterface;
class USkeletalMesh;
class UStaticMesh;

/**
 * A client-readable appearance mapping.  It contains presentation assets only;
 * no item instance identity or gameplay authority state is stored here.
 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOEquipmentAppearanceEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (DisplayName = "外观标识"))
	FName AppearanceId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (Categories = "Equipment.Appearance.Variant", DisplayName = "外观变体"))
	FGameplayTag VariantTag;

	/** Optional actor class for complex equipment visuals. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Actor", meta = (DisplayName = "外观表现 Actor 类"))
	TSoftClassPtr<AActor> VisualActorClass;

	/** Optional mesh assets for simple mesh-based presentation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Mesh", meta = (DisplayName = "骨骼网格"))
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Mesh", meta = (DisplayName = "静态网格"))
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Mesh", meta = (DisplayName = "材质"))
	TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Transform", meta = (DisplayName = "挂接插槽"))
	FName AttachSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance|Transform", meta = (DisplayName = "相对变换"))
	FTransform RelativeTransform = FTransform::Identity;

	bool Matches(const FName InAppearanceId, const FGameplayTag& InVariantTag) const;
};

/**
 * Primary asset registry for public equipment presentation.
 * The registry is static data and can be loaded asynchronously by clients.
 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOEquipmentAppearanceRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	const FDOEquipmentAppearanceEntry* FindAppearance(const FName InAppearanceId, const FGameplayTag& InVariantTag) const;

	UFUNCTION(BlueprintPure, Category = "DO|Equipment|Appearance")
	bool ResolveAppearance(FName InAppearanceId, FGameplayTag InVariantTag, FDOEquipmentAppearanceEntry& OutAppearance) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (TitleProperty = "AppearanceId", DisplayName = "外观条目"))
	TArray<FDOEquipmentAppearanceEntry> Entries;
};
