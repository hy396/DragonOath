#pragma once

#include "GameFramework/PlayerController.h"

#include "DOPlayerController.generated.h"

class UDOAbilitySystemComponent;

/**
 * 玩家控制器。
 *
 * Enhanced Input 的事件先在 Pawn 上收集，本类在每帧输入后处理 ASC 缓存。
 * 这样 Pressed、Held、Released 会按统一顺序进入技能系统，便于做预测和输入复制。
 */
UCLASS()
class DRAGONOATH_API ADOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADOPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostProcessInput(float DeltaTime, bool bGamePaused) override;

protected:
	UDOAbilitySystemComponent* GetDOAbilitySystemComponent() const;
};
