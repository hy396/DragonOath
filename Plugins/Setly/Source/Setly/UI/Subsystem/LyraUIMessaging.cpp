// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraUIMessaging.h"

#include "Messaging/CommonGameDialog.h"
#include "NativeGameplayTags.h"
#include "CommonLocalPlayer.h"
#include "PrimaryGameLayout.h"
#include "SetlyLogChannels.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraUIMessaging)

class FSubsystemCollectionBase;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MODAL, "UI.Layer.Modal");

void ULyraUIMessaging::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConfirmationDialogClassPtr = ConfirmationDialogClass.LoadSynchronous();
	ErrorDialogClassPtr = ErrorDialogClass.LoadSynchronous();

	UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging Initialize ConfirmationDialogClass=%s Loaded=%s ErrorDialogClass=%s Loaded=%s"),
		*ConfirmationDialogClass.ToString(),
		*GetNameSafe(ConfirmationDialogClassPtr),
		*ErrorDialogClass.ToString(),
		*GetNameSafe(ErrorDialogClassPtr));

	if (!ConfirmationDialogClassPtr)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging missing ConfirmationDialogClass. Configure [/Script/Setly.LyraUIMessaging] ConfirmationDialogClass in DefaultGame.ini."));
	}
	if (!ErrorDialogClassPtr)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging missing ErrorDialogClass. Configure [/Script/Setly.LyraUIMessaging] ErrorDialogClass in DefaultGame.ini."));
	}
}

void ULyraUIMessaging::ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging ShowConfirmation Descriptor=%s DialogClass=%s"),
		*GetNameSafe(DialogDescriptor),
		*GetNameSafe(ConfirmationDialogClassPtr));

	if (!DialogDescriptor)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowConfirmation failed: DialogDescriptor is null."));
		ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
		return;
	}
	if (!ConfirmationDialogClassPtr)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowConfirmation failed: ConfirmationDialogClass is null."));
		ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
		return;
	}

	if (UCommonLocalPlayer* LocalPlayer = GetLocalPlayer<UCommonLocalPlayer>())
	{
		if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
		{
			UCommonActivatableWidgetContainerBase* ModalLayer = RootLayout->GetLayerWidget(TAG_UI_LAYER_MODAL);
			UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging ShowConfirmation root layout. LocalPlayer=%s RootLayout=%s ModalLayer=%s ModalLayerClass=%s"),
				*GetNameSafe(LocalPlayer),
				*GetNameSafe(RootLayout),
				*GetNameSafe(ModalLayer),
				*GetNameSafe(ModalLayer ? ModalLayer->GetClass() : nullptr));

			if (!ModalLayer)
			{
				UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowConfirmation failed: RootLayout=%s has no UI.Layer.Modal layer."), *GetNameSafe(RootLayout));
				ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
				return;
			}

			UCommonGameDialog* Dialog = RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(TAG_UI_LAYER_MODAL, ConfirmationDialogClassPtr, [DialogDescriptor, ResultCallback](UCommonGameDialog& Dialog) {
				Dialog.SetupDialog(DialogDescriptor, ResultCallback);
			});

			UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging ShowConfirmation pushed dialog. Dialog=%s DialogClass=%s ModalLayer=%s"),
				*GetNameSafe(Dialog),
				*GetNameSafe(Dialog ? Dialog->GetClass() : nullptr),
				*GetNameSafe(ModalLayer));
			return;
		}

		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowConfirmation failed: RootLayout is null for LocalPlayer=%s."), *GetNameSafe(LocalPlayer));
		ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
		return;
	}

	UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowConfirmation failed: LocalPlayer is null."));
	ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
}

void ULyraUIMessaging::ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging ShowError Descriptor=%s DialogClass=%s"),
		*GetNameSafe(DialogDescriptor),
		*GetNameSafe(ErrorDialogClassPtr));

	if (!DialogDescriptor)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowError failed: DialogDescriptor is null."));
		ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
		return;
	}
	if (!ErrorDialogClassPtr)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowError failed: ErrorDialogClass is null."));
		ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
		return;
	}

	if (UCommonLocalPlayer* LocalPlayer = GetLocalPlayer<UCommonLocalPlayer>())
	{
		if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
		{
			UCommonActivatableWidgetContainerBase* ModalLayer = RootLayout->GetLayerWidget(TAG_UI_LAYER_MODAL);
			UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging ShowError root layout. LocalPlayer=%s RootLayout=%s ModalLayer=%s ModalLayerClass=%s"),
				*GetNameSafe(LocalPlayer),
				*GetNameSafe(RootLayout),
				*GetNameSafe(ModalLayer),
				*GetNameSafe(ModalLayer ? ModalLayer->GetClass() : nullptr));

			if (!ModalLayer)
			{
				UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowError failed: RootLayout=%s has no UI.Layer.Modal layer."), *GetNameSafe(RootLayout));
				ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
				return;
			}

			UCommonGameDialog* Dialog = RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(TAG_UI_LAYER_MODAL, ErrorDialogClassPtr, [DialogDescriptor, ResultCallback](UCommonGameDialog& Dialog) {
				Dialog.SetupDialog(DialogDescriptor, ResultCallback);
			});

			UE_LOG(LogLyra, Log, TEXT("LyraUIMessaging ShowError pushed dialog. Dialog=%s DialogClass=%s ModalLayer=%s"),
				*GetNameSafe(Dialog),
				*GetNameSafe(Dialog ? Dialog->GetClass() : nullptr),
				*GetNameSafe(ModalLayer));
			return;
		}

		UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowError failed: RootLayout is null for LocalPlayer=%s."), *GetNameSafe(LocalPlayer));
		ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
		return;
	}

	UE_LOG(LogLyra, Warning, TEXT("LyraUIMessaging ShowError failed: LocalPlayer is null."));
	ResultCallback.ExecuteIfBound(ECommonMessagingResult::Unknown);
}
