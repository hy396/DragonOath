// Copyright Epic Games, Inc. All Rights Reserved.
//
// DOVerbMessageProcessor_DamageHUD 的 .cpp —— 订阅方的参考实现（教学注释）。
// 启用方式见同名 .h 顶部说明。

#include "Messages/DOVerbMessageProcessor_DamageHUD.h"

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOVerbMessageProcessor_DamageHUD)

void UDOVerbMessageProcessor_DamageHUD::StartListening()
{
	// 基类 UGameplayMessageProcessor 会在 BeginPlay 自动调到这里。
	// 我们只需把感兴趣的频道 + 回调注册到 GameplayMessageSubsystem，并把返回的 handle 交给基类托管。
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);

	// ============================================================
	// 关键点 1：模板参数 <FDOVerbMessage> 决定 payload 类型。
	//         这里与发布方 DOHealthSet.cpp 里 BroadcastMessage 的类型必须严格一致。
	// 关键点 2：第二个参数是 tag —— 这里订阅的是 "Message.Combat.Damage.Applied"。
	//         订阅方只接收此 tag 频道（默认 ExactMatch），不会误收 Elimination 之类的事件。
	//         若想监听「所有 Combat 子事件」，把第一个参数换成
	//         DragonOathGameplayTags::Message::Combat::Combat
	//         并加 EGameplayMessageMatch::PartialMatch（如果你的 API 版本支持）。
	// 关键点 3：第三个参数 this + &Class::OnDamage 是「成员函数监听」重载，
	//         GameplayMessageSubsystem 内部会用弱引用保护 this，Actor 销毁后不会触发野回调。
	//         **不要**用裸 lambda 捕获 this。
	// 关键点 4：AddListenerHandle(...) 把 handle 存到基类的 ListenerHandles 数组里，
	//         EndPlay 时基类统一遍历 UnregisterListener —— 防止 GC 后仍触发回调。
	// ============================================================
	AddListenerHandle(
		MessageSubsystem.RegisterListener<FDOVerbMessage>(
			DragonOathGameplayTags::Message::Combat::DamageApplied,
			this,
			&UDOVerbMessageProcessor_DamageHUD::OnDamage
		)
	);

	// 可以继续 RegisterListener(...) 订阅其它频道（例如 EliminationFired 做击杀横幅）：
	/*
	AddListenerHandle(
		MessageSubsystem.RegisterListener<FDOVerbMessage>(
			DragonOathGameplayTags::Message::Combat::EliminationFired,
			this,
			&UDOVerbMessageProcessor_DamageHUD::OnElimination
		)
	);
	*/
}

void UDOVerbMessageProcessor_DamageHUD::OnDamage(FGameplayTag Channel, const FDOVerbMessage& Message)
{
	// 这一行是「确认通路打通」的最快办法：先打日志，确认能收到消息，再接业务逻辑。
	// 实际业务：HUD 飘字 / Niagara / 音效 / 震屏 / 统计组件写入。
	UE_LOG(LogTemp, Display,
		TEXT("[DO|DamageHUD] Received FDOVerbMessage: Channel=%s, Instigator=%s, Target=%s, Magnitude=%.1f"),
		*Channel.ToString(),
		*GetNameSafe(Message.Instigator),
		*GetNameSafe(Message.Target),
		Message.Magnitude);

	// TODO(项目方)：这里接真正的表现层逻辑。例如：
	//   - if (UMyHUDWidget* HUD = GetHUDWidget()) { HUD->ShowDamageNumber(Message.Magnitude, Message.Target); }
	//   - if (UNiagaraSystem* HitVFX = SelectHitVFX(Message.ContextTags)) { SpawnNiagaraAt(HitVFX, Message.Target); }
	//   - if (USoundBase* HitSFX = SelectHitSFX(Message.ContextTags)) { PlaySound(HitSFX, Message.Target); }
}

/*
// 击杀横幅订阅方示例（如果启用了上面的 OnElimination）：
void UDOVerbMessageProcessor_DamageHUD::OnElimination(FGameplayTag Channel, const FDOVerbMessage& Message)
{
	UE_LOG(LogTemp, Display,
		TEXT("[DO|EliminationHUD] 击杀事件: Killer=%s, Victim=%s, FinalDamage=%.1f"),
		*GetNameSafe(Message.Instigator),
		*GetNameSafe(Message.Target),
		Message.Magnitude);

	// TODO: 接击杀流横幅 / 连杀计数器 / 助攻提示。
}
*/