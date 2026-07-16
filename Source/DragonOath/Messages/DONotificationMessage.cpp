// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messages/DONotificationMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DONotificationMessage)

// DragonOath 的"加一条通知"消息频道 tag 定义。
// 监听方（如成就 / 击杀流 / 拾取流的 UI 组件）用这个 tag 来过滤接收的通知类型。
UE_DEFINE_GAMEPLAY_TAG(TAG_DO_AddNotification_Message, "Message.UI.Notification.Added");