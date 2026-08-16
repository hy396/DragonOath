#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "DOEquipmentPresentationTypes.generated.h"

class UDOEquipmentPresentationComponent;
struct FDOEquipmentPublicList;
// TODO: 装备是装备，并不需要复制到其他客户端上给他们看，装备只是数值上的提升，不会改变外观
// 装备了什么东西只有在其他玩家查看当前玩家的个人面板的时候才会展示出来，否则不会给其他玩家知道浪费网络带宽。
/** 复制给所有相关客户端的轻量装备外观摘要。 */
USTRUCT(BlueprintType)
struct DRAGONOATH_API FDOEquipmentPublicEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/** 装备槽位；槽位本身不包含私有物品实例身份。 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotTag;

	/** 由表现注册表解析的公开外观 ID。 */
	UPROPERTY(BlueprintReadOnly)
	FName AppearanceId;

	/** 外观变体标签，例如武器形态或服饰版本。 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag VariantTag;

	/** 公开染色参数；不承载装备属性。 */
	UPROPERTY(BlueprintReadOnly)
	FLinearColor Tint = FLinearColor::White;

	/** 同槽位外观变更版本，供异步资源加载结果做过期检查。 */
	UPROPERTY(BlueprintReadOnly)
	int32 VisualRevision = 0;

	void PostReplicatedAdd(const FDOEquipmentPublicList& /*Serializer*/) {}
	void PostReplicatedChange(const FDOEquipmentPublicList& /*Serializer*/) {}
	void PreReplicatedRemove(const FDOEquipmentPublicList& /*Serializer*/) {}
};

/** Pawn 上公开装备摘要的 FastArray 容器。 */
USTRUCT()
struct DRAGONOATH_API FDOEquipmentPublicList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDOEquipmentPublicEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UDOEquipmentPresentationComponent> OwnerComponent = nullptr;

	TArray<FGameplayTag> PendingChangedSlotTags;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FDOEquipmentPublicEntry, FDOEquipmentPublicList>(Entries, DeltaParams, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);
};

template<>
struct TStructOpsTypeTraits<FDOEquipmentPublicList> : public TStructOpsTypeTraitsBase2<FDOEquipmentPublicList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

