// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：用户、平台和在线会话辅助插件，后续登录、房间、匹配阶段再深入接入。

#include "CommonUserTypes.h"
#include "OnlineError.h"

void FOnlineResultInformation::FromOnlineError(const FOnlineErrorType& InOnlineError)
{
#if COMMONUSER_OSSV1
	bWasSuccessful = InOnlineError.WasSuccessful();
	ErrorId = InOnlineError.GetErrorCode();
	ErrorText = InOnlineError.GetErrorMessage();
#else
	bWasSuccessful = InOnlineError != UE::Online::Errors::Success();
	ErrorId = InOnlineError.GetErrorId();
	ErrorText = InOnlineError.GetText();
#endif
}
