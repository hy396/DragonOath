#include "UI/Inventory/DOInventoryScreen.h"

#include "Input/CommonUIInputTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Async/TaskGraphInterfaces.h"
#include "Player/DOPlayerState.h"
#include "UI/Inventory/DOInventorySlateWidgets.h"
#include "UI/Inventory/DOInventoryViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOInventoryScreen)

UDOInventoryScreen::UDOInventoryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

TOptional<FUIInputConfig> UDOInventoryScreen::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

UWidget* UDOInventoryScreen::NativeGetDesiredFocusTarget() const
{
	// CommonUI 需要一个 UObject Widget 作为焦点恢复目标；真正的键盘焦点随后交给 Slate 网格。
	return const_cast<UDOInventoryScreen*>(this);
}

TSharedRef<SWidget> UDOInventoryScreen::RebuildWidget()
{
	if (!ViewModel)
	{
		ViewModel = NewObject<UDOInventoryViewModel>(this);
	}

	SAssignNew(InventoryPanel, SDOInventoryEquipmentPanel)
		.ViewModel(ViewModel)
		.OnCloseRequested(FDOOnInventoryCloseRequested::CreateUObject(this, &UDOInventoryScreen::HandleCloseRequested));
	return InventoryPanel.ToSharedRef();
}

void UDOInventoryScreen::ReleaseSlateResources(const bool bReleaseChildren)
{
	InventoryPanel.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UDOInventoryScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!ViewModel)
	{
		ViewModel = NewObject<UDOInventoryViewModel>(this);
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		ViewModel->Initialize(PlayerController->GetPlayerState<ADOPlayerState>());
	}

	if (InventoryPanel.IsValid() && FSlateApplication::IsInitialized())
	{
		InventoryPanel->FocusInitialWidget();
	}
}

void UDOInventoryScreen::NativeOnDeactivated()
{
	if (ViewModel)
	{
		ViewModel->Shutdown();
	}
	Super::NativeOnDeactivated();

	// CommonUI 在页面关闭时会把输入模式恢复成页面激活前的状态，
	// 该过程可能把 bShowMouseCursor 重置为 false，导致鼠标消失。
	// 本项目鼠标要求常驻，因此在页面失活后延迟一帧强制重新显示鼠标，
	// 避开 CommonUI 输入模式恢复的时序竞争。
	if (APlayerController* PC = GetOwningPlayer())
	{
		TWeakObjectPtr<APlayerController> WeakPC(PC);
		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[WeakPC]()
			{
				if (APlayerController* SafePC = WeakPC.Get())
				{
					SafePC->bShowMouseCursor = true;
					SafePC->DefaultMouseCursor = EMouseCursor::Default;
					SafePC->CurrentMouseCursor = EMouseCursor::Default;
				}
			},
			TStatId(), nullptr, ENamedThreads::GameThread);
	}
}

void UDOInventoryScreen::HandleCloseRequested()
{
	DeactivateWidget();
}

FReply UDOInventoryScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		DeactivateWidget();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
