// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraConfirmationScreen.h"

#if WITH_EDITOR
#include "CommonInputSettings.h"
#include "Editor/WidgetCompilerLog.h"
#endif

#include "CommonBorder.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "Engine/World.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIActionRouterBase.h"
#include "LyraButtonBase.h"
#include "SetlyLogChannels.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraConfirmationScreen)

namespace
{
	constexpr int32 MaxButtonInputBindingRetries = 10;
}

void ULyraConfirmationScreen::SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback)
{
	Super::SetupDialog(Descriptor, ResultCallback);

	PendingButtonInputActions.Reset();
	ButtonInputBindingRetryCount = 0;

	Text_Title->SetText(Descriptor->Header);
	RichText_Description->SetText(Descriptor->Body);

	EntryBox_Buttons->Reset<ULyraButtonBase>([](ULyraButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	for (const FConfirmationDialogAction& Action : Descriptor->ButtonActions)
	{
		FDataTableRowHandle ActionRow;

		switch(Action.Result)
		{
			case ECommonMessagingResult::Confirmed:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
				break;
			case ECommonMessagingResult::Declined:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
				break;
			case ECommonMessagingResult::Cancelled:
				ActionRow = CancelAction;
				break;
			default:
				ensure(false);
				continue;
		}

		ULyraButtonBase* Button = EntryBox_Buttons->CreateEntry<ULyraButtonBase>();
		PendingButtonInputActions.Add({ Button, ActionRow });
		Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
		Button->SetButtonText(Action.OptionalDisplayText);
	}

	OnResultCallback = ResultCallback;

	UE_LOG(LogLyra, Log, TEXT("LyraConfirmationScreen setup. Dialog=%s PendingButtons=%d IsActivated=%s"),
		*GetNameSafe(this),
		PendingButtonInputActions.Num(),
		IsActivated() ? TEXT("true") : TEXT("false"));

	if (IsActivated())
	{
		ScheduleBindPendingButtonInputActions();
	}
}

void ULyraConfirmationScreen::KillDialog()
{
	Super::KillDialog();
}

void ULyraConfirmationScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	UE_LOG(LogLyra, Log, TEXT("LyraConfirmationScreen activated. Dialog=%s PendingButtons=%d"),
		*GetNameSafe(this),
		PendingButtonInputActions.Num());

	ScheduleBindPendingButtonInputActions();
}

void ULyraConfirmationScreen::NativeOnDeactivated()
{
	PendingButtonInputActions.Reset();
	ButtonInputBindingRetryCount = 0;

	Super::NativeOnDeactivated();
}

void ULyraConfirmationScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Border_TapToCloseZone->OnMouseButtonDownEvent.BindDynamic(this, &ULyraConfirmationScreen::HandleTapToCloseZoneMouseButtonDown);
}

void ULyraConfirmationScreen::ScheduleBindPendingButtonInputActions()
{
	if (PendingButtonInputActions.IsEmpty() || IsDesignTime())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			BindPendingButtonInputActions();
		}));
		return;
	}

	BindPendingButtonInputActions();
}

void ULyraConfirmationScreen::BindPendingButtonInputActions()
{
	if (PendingButtonInputActions.IsEmpty() || !IsActivated())
	{
		return;
	}

	UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this);
	const bool bTreePending = ActionRouter && ActionRouter->IsPendingTreeChange();
	const bool bInActiveRoot = ActionRouter && ActionRouter->IsWidgetInActiveRoot(this);

	if ((!ActionRouter || bTreePending || !bInActiveRoot) && ButtonInputBindingRetryCount < MaxButtonInputBindingRetries)
	{
		++ButtonInputBindingRetryCount;
		UE_LOG(LogLyra, Verbose, TEXT("LyraConfirmationScreen waiting for CommonUI tree. Dialog=%s Retry=%d Router=%s TreePending=%s InActiveRoot=%s"),
			*GetNameSafe(this),
			ButtonInputBindingRetryCount,
			*GetNameSafe(ActionRouter),
			bTreePending ? TEXT("true") : TEXT("false"),
			bInActiveRoot ? TEXT("true") : TEXT("false"));
		ScheduleBindPendingButtonInputActions();
		return;
	}

	if (!ActionRouter || bTreePending || !bInActiveRoot)
	{
		UE_LOG(LogLyra, Warning, TEXT("LyraConfirmationScreen skipped button input binding. Dialog=%s Router=%s TreePending=%s InActiveRoot=%s PendingButtons=%d"),
			*GetNameSafe(this),
			*GetNameSafe(ActionRouter),
			bTreePending ? TEXT("true") : TEXT("false"),
			bInActiveRoot ? TEXT("true") : TEXT("false"),
			PendingButtonInputActions.Num());
		PendingButtonInputActions.Reset();
		return;
	}

	for (const FPendingButtonInputAction& PendingAction : PendingButtonInputActions)
	{
		if (ULyraButtonBase* Button = PendingAction.Button.Get())
		{
			Button->SetTriggeringInputAction(PendingAction.InputAction);
			UE_LOG(LogLyra, Verbose, TEXT("LyraConfirmationScreen bound button input. Dialog=%s Button=%s Action=%s"),
				*GetNameSafe(this),
				*GetNameSafe(Button),
				*PendingAction.InputAction.RowName.ToString());
		}
	}

	UE_LOG(LogLyra, Log, TEXT("LyraConfirmationScreen bound pending button input actions. Dialog=%s Count=%d"),
		*GetNameSafe(this),
		PendingButtonInputActions.Num());

	PendingButtonInputActions.Reset();
	ButtonInputBindingRetryCount = 0;
}

void ULyraConfirmationScreen::CloseConfirmationWindow(ECommonMessagingResult Result)
{
	DeactivateWidget();
	OnResultCallback.ExecuteIfBound(Result);
}

FEventReply ULyraConfirmationScreen::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();

	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		CloseConfirmationWindow(ECommonMessagingResult::Declined);
		Reply.NativeReply = FReply::Handled();
	}

	return Reply;
}

#if WITH_EDITOR
void ULyraConfirmationScreen::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	if (CancelAction.IsNull())
	{
		CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} has unset property: CancelAction.")), FText::FromString(GetName())));
	}
}
#endif
