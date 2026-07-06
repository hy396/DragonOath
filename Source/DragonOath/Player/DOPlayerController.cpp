#include "Player/DOPlayerController.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlayerController)

ADOPlayerController::ADOPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ADOPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// Enhanced Input 会先把本帧输入事件派发给 Pawn。
	// 这里再统一处理 ASC 缓存的技能输入，顺序与 Lyra 保持一致。
	if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
	{
		DOASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

UDOAbilitySystemComponent* ADOPlayerController::GetDOAbilitySystemComponent() const
{
	return UDOAbilitySystemComponent::GetFromActor(GetPawn());
}
