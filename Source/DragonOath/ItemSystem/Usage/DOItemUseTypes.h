#pragma once

#include "CoreMinimal.h"

#include "DOItemUseTypes.generated.h"

/**
 * 服务器发起一次复杂道具使用时携带的临时上下文。
 *
 * 该对象不复制、不存档，只在本次 Ability/Event 流程中保存物品实例和定义标识，
 * 防止蓝图通过输入参数伪造物品效果或冷却数值。
 */
UCLASS(BlueprintType)
class DRAGONOATH_API UDOItemUseContext : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "DO|ItemUse")
	FGuid InstanceId;

	UPROPERTY(BlueprintReadOnly, Category = "DO|ItemUse")
	FPrimaryAssetId DefinitionId;
};
