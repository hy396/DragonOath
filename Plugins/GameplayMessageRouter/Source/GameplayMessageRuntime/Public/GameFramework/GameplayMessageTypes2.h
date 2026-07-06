// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameplayMessageTypes2.generated.h"

class UGameplayMessageRouter;

/**
 * 消息监听器的匹配规则
 *
 * 消息频道是 GameplayTag（层级化字符串，如 "UI.Inventory.ItemAdded"），
 * 监听器可以选择"只响应完全相同的 Tag"还是"也响应子 Tag"。
 */
UENUM(BlueprintType)
enum class EGameplayMessageMatch : uint8
{
	// 精确匹配——只接收频道完全相同的消息
	// 例：监听 "A.B" 时只会收到对 "A.B" 的广播，不会收到 "A.B.C"
	ExactMatch,

	// 部分匹配——接收以监听频道为根的所有子频道消息
	// 例：监听 "A.B" 时，对 "A.B" 和 "A.B.C" 的广播都会收到
	PartialMatch
};
/**
 * 注册监听器时的高级参数
 *
 * 当你需要更灵活的行为（如设置匹配规则 + 绑定成员函数）时，
 * 构造一个这个结构体传给 RegisterListener，而不是直接传 Lambda。
 */
template<typename FMessageStructType>
struct FGameplayMessageListenerParams
{
	/** 匹配规则。默认只响应完全相同的频道 Tag */
	EGameplayMessageMatch MatchType = EGameplayMessageMatch::ExactMatch;

	/** 消息到达时触发的回调（按值形式绑定，最灵活） */
	TFunction<void(FGameplayTag, const FMessageStructType&)> OnMessageReceivedCallback;

	/**
	 * 辅助方法：把成员函数（弱引用持有目标对象）绑定到 OnMessageReceivedCallback
	 *
	 * @param Object    目标对象指针（内部以 WeakObjectPtr 持有，销毁后自动失效）
	 * @param Function  成员函数指针
	 */
	template<typename TOwner = UObject>
	void SetMessageReceivedCallback(TOwner* Object, void(TOwner::* Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		OnMessageReceivedCallback = [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		{
			if (TOwner* StrongObject = WeakObject.Get())
			{
				(StrongObject->*Function)(Channel, Payload);
			}
		};
	}
};
