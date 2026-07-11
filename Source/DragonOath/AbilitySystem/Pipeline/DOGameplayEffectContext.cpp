// DragonOath custom GameplayEffectContext.

#include "DOGameplayEffectContext.h"

bool FDOGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// ================================================================
	// 自定义网络序列化
	// ================================================================
	//
	// 【工作原理】
	// FArchive 同时处理读（从网络接收）和写（发送到网络）
	// Ar << Value 会根据当前是读还是写自动选择方向
	// Ar.IsSaving() = 正在序列化（发送端）
	// Ar.IsLoading() = 正在反序列化（接收端）
	//
	// 【性能优化】使用位字段压缩bool值
	// 多个bool打包成一个uint8，节省带宽
	// 因为网络上每个字段至少占1字节，4个bool单独发需要4字节
	// 打包后只需要1字节

	// 先调用父类序列化（处理Instigator、HitResult等）
	Super::NetSerialize(Ar, Map, bOutSuccess);

	// ================================================================
	// 使用位掩码压缩 —— 减少网络开销
	// ================================================================

	uint8 RepBits = 0;
	if (Ar.IsSaving())
	{
		// 序列化：构建位掩码
		if (bIsCriticalHit)             RepBits |= 1 << 0;
		if (bIsBlockedHit)              RepBits |= 1 << 1;
		if (HitBoneName != NAME_None)   RepBits |= 1 << 2;
		if (!DamageDirection.IsZero())  RepBits |= 1 << 3;
		if (DamageElementTag.IsValid()) RepBits |= 1 << 4;
		if (DamageMultiplier != 1.0f)   RepBits |= 1 << 5;
	}

	Ar.SerializeBits(&RepBits, 6); // 只序列化6位

	// 根据位掩码有条件地序列化各字段
	// 这样没有设置的字段完全不占带宽

	if (RepBits & (1 << 0))
	{
		Ar << bIsCriticalHit;
	}

	if (RepBits & (1 << 1))
	{
		Ar << bIsBlockedHit;
	}

	if (RepBits & (1 << 2))
	{
		Ar << HitBoneName;
	}

	if (RepBits & (1 << 3))
	{
		// FVector用NetQuantize压缩精度，进一步节省带宽
		// 不需要0.001cm精度，1cm就够了
		DamageDirection.NetSerialize(Ar, Map, bOutSuccess);
	}

	if (RepBits & (1 << 4))
	{
		Ar << DamageElementTag;
	}

	if (RepBits & (1 << 5))
	{
		Ar << DamageMultiplier;
	}

	bOutSuccess = true;
	return true;
}

// ================================================================
// 【使用示例】在GA中设置自定义EffectContext
// ================================================================
//
// void UMyAbility::ActivateAbility(...)
// {
//     UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
//
//     // 创建 EffectContext（会自动创建 FDOGameplayEffectContext，因为我们重写了 Globals）
//     FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
//
//     // 拿到自定义Context
//     FDOGameplayEffectContext* CustomContext =
//         static_cast<FDOGameplayEffectContext*>(ContextHandle.Get());
//
//     // 设置自定义数据
//     CustomContext->SetIsCriticalHit(true);
//     CustomContext->SetHitBoneName("head");
//     CustomContext->SetDamageMultiplier(2.0f);
//     CustomContext->SetDamageDirection(ShootDirection);
//     CustomContext->SetDamageElementTag(
//         FGameplayTag::RequestGameplayTag("Element.Fire"));
//
//     // 创建GE Spec时传入这个Context
//     FGameplayEffectSpecHandle SpecHandle =
//         ASC->MakeOutgoingSpec(DamageEffect, Level, ContextHandle);
//
//     // 应用GE —— Context会随GE一路传递到ExecCalc和GameplayCue
//     ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
// }
//
// ================================================================
// 【使用示例】在ExecCalc中读取自定义Context
// ================================================================
//
// void UMyExecCalc::Execute_Implementation(
//     const FGameplayEffectCustomExecutionParameters& ExecutionParams,
//     FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
// {
//     const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
//
//     const FDOGameplayEffectContext* Context =
//         static_cast<const FDOGameplayEffectContext*>(
//             Spec.GetEffectContext().Get());
//
//     if (Context)
//     {
//         if (Context->IsCriticalHit())
//         {
//             FinalDamage *= 2.0f;
//         }
//
//         FinalDamage *= Context->GetDamageMultiplier();
//     }
// }
//
// ================================================================
// 【使用示例】在GameplayCue中读取自定义Context
// ================================================================
//
// void AMyGameplayCueNotify::HandleGameplayCue(
//     AActor* Target,
//     EGameplayCueEvent::Type EventType,
//     const FGameplayCueParameters& Parameters)
// {
//     const FDOGameplayEffectContext* Context =
//         static_cast<const FDOGameplayEffectContext*>(
//             Parameters.EffectContext.Get());
//
//     if (Context && Context->IsCriticalHit())
//     {
//         // 播放暴击特效
//         SpawnCriticalHitVFX(Parameters.Location);
//     }
//     else
//     {
//         // 播放普通命中特效
//         SpawnNormalHitVFX(Parameters.Location);
//     }
// }
