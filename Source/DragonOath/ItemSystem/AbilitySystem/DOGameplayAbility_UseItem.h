#pragma once

#include "AbilitySystem/Abilities/Core/DOGameplayAbility.h"

#include "DOGameplayAbility_UseItem.generated.h"

/** 复杂消耗品使用流程的 C++ 基类。 */
UCLASS(Abstract, Blueprintable)
class DRAGONOATH_API UDOGameplayAbility_UseItem : public UDOGameplayAbility
{
	GENERATED_BODY()

public:
	UDOGameplayAbility_UseItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/**
	 * 在复杂流程的最终提交点调用。
	 * 只使用服务器生成的上下文，不接受蓝图传入数量、属性或冷却时间。
	 */
	UFUNCTION(BlueprintCallable, Category = "DO|物品")
	bool CommitItemUse();

	/** 获取服务器注入的物品使用上下文，供表现和目标选择逻辑读取。 */
	const class UDOItemUseContext* GetItemUseContext() const;
};
