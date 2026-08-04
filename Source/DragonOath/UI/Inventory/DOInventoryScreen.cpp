#include "UI/Inventory/DOInventoryScreen.h"

#include "Input/CommonUIInputTypes.h"
#include "Framework/Application/SlateApplication.h"
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
