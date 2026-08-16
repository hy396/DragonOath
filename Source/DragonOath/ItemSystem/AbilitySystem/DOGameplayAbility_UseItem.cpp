#include "ItemSystem/AbilitySystem/DOGameplayAbility_UseItem.h"

#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Usage/DOItemUseTypes.h"
#include "Player/DOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOGameplayAbility_UseItem)

UDOGameplayAbility_UseItem::UDOGameplayAbility_UseItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 道具使用由服务器背包请求启动，不走客户端预测；每次使用单独创建实例，避免上下文互相覆盖。
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

const UDOItemUseContext* UDOGameplayAbility_UseItem::GetItemUseContext() const
{
	if (const UDOItemUseContext* EventContext = Cast<UDOItemUseContext>(CurrentEventData.OptionalObject.Get()))
	{
		return EventContext;
	}

	if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UDOItemUseContext>(Spec->SourceObject.Get());
	}

	return nullptr;
}

bool UDOGameplayAbility_UseItem::CommitItemUse()
{
	const UDOItemUseContext* Context = GetItemUseContext();
	const ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwningActorFromActorInfo());
	UDOInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Context || !Inventory)
	{
		return false;
	}

	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	return Inventory->CommitConsumableUse(Context->InstanceId, Context->DefinitionId, FailureReason, Context->ClientOperationId);
}

void UDOGameplayAbility_UseItem::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	if (bWasCancelled)
	{
		const UDOItemUseContext* Context = GetItemUseContext();
		const ADOPlayerState* PlayerState = Cast<ADOPlayerState>(GetOwningActorFromActorInfo());
		if (Context && Context->ClientOperationId > 0 && PlayerState)
		{
			if (UDOInventoryComponent* Inventory = PlayerState->GetInventoryComponent())
			{
				Inventory->CancelConsumableUse(Context->ClientOperationId);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
