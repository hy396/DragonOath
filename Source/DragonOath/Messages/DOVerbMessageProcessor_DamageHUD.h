// Copyright Epic Games, Inc. All Rights Reserved.
//
// DOVerbMessageProcessor_DamageHUD —— FDOVerbMessage 订阅方的参考实现（教学注释）。
//
// 文件性质：示例代码（reference implementation），**不**会被默认编译进项目。
// 启用方式：把这个文件改名为 DOVerbMessageProcessor_DamageHUD.h.in → DOVerbMessageProcessor_DamageHUD.h（去掉 .in），
//           或者直接把整个文件内容粘到新 .h/.cpp。
// 或者：直接复制里面的代码到自己工程的「HUD 处理器」组件里使用。
//
// 用途：演示如何继承 UGameplayMessageProcessor 基类订阅 Message.Combat.Damage.Applied，
// 在 BeginPlay/StartListening 里 RegisterListener，在 EndPlay 由基类统一兜底 Unregister。
// 实战时把这个类的 OnDamage 接到你的伤害数字 Widget / Niagara 飘字系统 / 受击音效。

#pragma once

#include "CoreMinimal.h"
#include "Messages/DOVerbMessage.h"
#include "Messages/GameplayMessageProcessor.h"

#include "DOVerbMessageProcessor_DamageHUD.generated.h"

/**
 * 参考实现：HUD 伤害数字 / 命中反馈处理器。
 *
 * 用法（在自己的项目里照搬）：
 *   1. 把这个类（或它的简化版）放到你的 HUD Actor 或 PlayerController 上作为 UActorComponent。
 *   2. BeginPlay 时基类自动调 StartListening()，里面 RegisterListener<FDOVerbMessage>(Message.Combat.Damage.Applied, ...)。
 *   3. 收到回调后，自己接：
 *        - HUD 飘字（Niagara / UMG）
 *        - 受击音效
 *        - 命中震屏
 *        - 写入伤害统计组件
 *      等等。
 *
 * 注意：这个文件目前处于「参考示例」状态，**没**在 ProjectSettings / Build.cs 里激活。
 *       启用时记得改 .Build.cs 加 UMG / Niagara（如果还没加）。
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class DRAGONOATH_API UDOVerbMessageProcessor_DamageHUD : public UGameplayMessageProcessor
{
	GENERATED_BODY()

public:
	// 继承自 UGameplayMessageProcessor：基类在 BeginPlay 时自动调 StartListening()，在 EndPlay 统一反注册。
	virtual void StartListening() override;

private:
	// 收到伤害事件后的回调。
	// Channel 是订阅时传入的 tag；Message 是发布方构造的 FDOVerbMessage。
	void OnDamage(FGameplayTag Channel, const FDOVerbMessage& Message);
};