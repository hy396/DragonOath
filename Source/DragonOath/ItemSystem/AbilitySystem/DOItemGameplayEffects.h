#pragma once

#include "GameplayEffect.h"

#include "DOItemGameplayEffects.generated.h"

/** 所有装备共用的无限时长属性 GE。实际数值由装备 Spec 的 SetByCaller 写入。 */
UCLASS()
class DRAGONOATH_API UDOEquipmentAttributeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDOEquipmentAttributeEffect();
};

/** 所有简单回复道具共用的瞬时 GE。 */
UCLASS()
class DRAGONOATH_API UDOItemInstantRestoreEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDOItemInstantRestoreEffect();
};

/** 所有限时属性道具共用的持续型 GE。 */
UCLASS()
class DRAGONOATH_API UDOItemTimedAttributeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDOItemTimedAttributeEffect();
};

/** 所有道具公共冷却共用的持续型 GE。 */
UCLASS()
class DRAGONOATH_API UDOItemCooldownEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDOItemCooldownEffect();
};
