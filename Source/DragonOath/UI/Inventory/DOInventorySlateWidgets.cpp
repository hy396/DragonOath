#include "UI/Inventory/DOInventorySlateWidgets.h"

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "UI/Inventory/DOInventoryStyle.h"
#include "UI/Inventory/DOInventoryViewModel.h"
#include "UI/Inventory/DOItemTooltipViewModel.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FSlateBrush* GetWhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	FSlateColor GetQualityColor(const FGameplayTag& Rarity)
	{
		const FString Name = Rarity.ToString();
		if (Name.EndsWith(TEXT("Legendary"))) return FSlateColor(FLinearColor(1.0f, 0.45f, 0.08f));
		if (Name.EndsWith(TEXT("Epic"))) return FSlateColor(FLinearColor(0.75f, 0.25f, 1.0f));
		if (Name.EndsWith(TEXT("Rare"))) return FSlateColor(FLinearColor(0.18f, 0.55f, 1.0f));
		if (Name.EndsWith(TEXT("Uncommon"))) return FSlateColor(FLinearColor(0.25f, 0.85f, 0.35f));
		return FSlateColor(FLinearColor(0.72f, 0.76f, 0.82f));
	}

}

void SDOItemContextMenu::Construct(const FArguments& InArgs)
{
	SlotViewModel = InArgs._SlotViewModel;
	ViewModel = InArgs._ViewModel;
	OnAction = InArgs._OnAction;

	TSharedRef<SVerticalBox> ActionBox = SNew(SVerticalBox);
	bool bHasAction = false;
	if (SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty && !SlotViewModel->bIsPending)
	{
		if (SlotViewModel->bIsUsable)
		{
			bHasAction = true;
			ActionBox->AddSlot().AutoHeight()[BuildActionButton(FText::FromString(TEXT("使用")), EDOInventoryContextAction::Use)];
		}
		if (SlotViewModel->EquipmentSlotTag.IsValid())
		{
			bHasAction = true;
			ActionBox->AddSlot().AutoHeight()[BuildActionButton(FText::FromString(TEXT("装备")), EDOInventoryContextAction::Equip)];
		}
		if (SlotViewModel->bIsUsable)
		{
			bHasAction = true;
			ActionBox->AddSlot().AutoHeight()[BuildActionButton(FText::FromString(TEXT("绑定快捷栏 1")), EDOInventoryContextAction::AssignQuickBar1)];
			ActionBox->AddSlot().AutoHeight()[BuildActionButton(FText::FromString(TEXT("绑定快捷栏 2")), EDOInventoryContextAction::AssignQuickBar2)];
			ActionBox->AddSlot().AutoHeight()[BuildActionButton(FText::FromString(TEXT("绑定快捷栏 3")), EDOInventoryContextAction::AssignQuickBar3)];
			ActionBox->AddSlot().AutoHeight()[BuildActionButton(FText::FromString(TEXT("绑定快捷栏 4")), EDOInventoryContextAction::AssignQuickBar4)];
		}
		if (SlotViewModel->bCanDiscard)
		{
			bHasAction = true;
			ActionBox->AddSlot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)[BuildActionButton(FText::FromString(TEXT("丢弃")), EDOInventoryContextAction::Discard)];
		}
	}

	if (!bHasAction)
	{
		ActionBox->AddSlot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("暂无可用操作")))];
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.SubPanel"))
		.BorderBackgroundColor(FLinearColor::White)
		.Padding(6.0f)
		[
			ActionBox
		]
	];
}

TSharedRef<SWidget> SDOItemContextMenu::BuildActionButton(const FText& Label, const EDOInventoryContextAction Action)
{
	return SNew(SButton)
		.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
		.OnClicked_Lambda([this, Action]
		{
			if (OnAction.IsBound() && SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty)
			{
				OnAction.Execute(SlotViewModel->GetInstanceId(), static_cast<uint8>(Action));
			}
			return FReply::Handled();
		})
		[
			SNew(STextBlock).Text(Label)
		];
}

void SDOInventorySlotWidget::Construct(const FArguments& InArgs)
{
	SlotViewModel = InArgs._SlotViewModel;
	ViewModel = InArgs._ViewModel;
	SlotIndex = InArgs._SlotIndex;
	OnSelected = InArgs._OnSelected;
	OnActivated = InArgs._OnActivated;
	OnDropped = InArgs._OnDropped;
	OnContextAction = InArgs._OnContextAction;
	RequestIconLoad();

	SAssignNew(ContextMenuAnchor, SMenuAnchor)
	.Placement(MenuPlacement_MenuRight)
	.OnGetMenuContent(this, &SDOInventorySlotWidget::BuildContextMenu)
	[
		SNew(SBox)
		.WidthOverride(82.0f)
		.HeightOverride(82.0f)
		[
			SNew(SBorder)
			.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.Slot"))
			.BorderBackgroundColor_Lambda([this] { return GetBorderColor(); })
			.Padding(4.0f)
			.ToolTipText(this, &SDOInventorySlotWidget::GetTooltipText)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image_Lambda([this] { return GetIconBrush(); })
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				.Padding(1.0f)
				[
					SNew(SBorder)
					.BorderImage(GetWhiteBrush())
					.BorderBackgroundColor(FLinearColor(0.12f, 0.035f, 0.01f, 0.82f))
					.Padding(2.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text_Lambda([this] { return GetNameText(); })
						.Font(FDOInventoryStyle::GetBodyTextStyle().Font)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FLinearColor(1.0f, 0.93f, 0.78f, 1.0f))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(STextBlock)
					.Text_Lambda([this] { return GetStackText(); })
					.Font(FDOInventoryStyle::GetBodyTextStyle().Font)
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.Visibility_Lambda([this]
					{
						return SlotViewModel.IsValid() && SlotViewModel->bIsPending
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.SubPanel"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.04f, 0.06f, 0.82f))
					.Padding(4.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("处理中…")))
						.ColorAndOpacity(FLinearColor(1.0f, 0.86f, 0.35f))
					]
				]
			]
		]
	];

	ChildSlot
	[
		ContextMenuAnchor.ToSharedRef()
	];
}

TSharedRef<SWidget> SDOInventorySlotWidget::BuildContextMenu()
{
	return SNew(SDOItemContextMenu)
		.SlotViewModel(SlotViewModel)
		.ViewModel(ViewModel)
		.OnAction(OnContextAction);
}

bool SDOInventorySlotWidget::IsSelected() const
{
	return ViewModel.IsValid() && SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty && ViewModel->GetSelectedInstanceId() == SlotViewModel->GetInstanceId();
}

FSlateColor SDOInventorySlotWidget::GetBorderColor() const
{
	if (!SlotViewModel.IsValid() || SlotViewModel->bIsEmpty)
	{
		return FSlateColor(FLinearColor(0.34f, 0.13f, 0.035f, 0.88f));
	}
	if (SlotViewModel->bIsPending)
	{
		return FSlateColor(FLinearColor(1.0f, 0.78f, 0.18f, 1.0f));
	}
	if (IsSelected())
	{
		return FSlateColor(FLinearColor(1.0f, 0.72f, 0.18f, 1.0f));
	}
	return GetQualityColor(SlotViewModel->Rarity);
}

FText SDOInventorySlotWidget::GetNameText() const
{
	return SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty ? SlotViewModel->DisplayName : FText::GetEmpty();
}

FText SDOInventorySlotWidget::GetStackText() const
{
	if (!SlotViewModel.IsValid() || SlotViewModel->bIsEmpty || SlotViewModel->Item.StackCount <= 1)
	{
		return FText::GetEmpty();
	}
	return FText::AsNumber(SlotViewModel->Item.StackCount);
}

FText SDOInventorySlotWidget::GetTooltipText() const
{
	if (!SlotViewModel.IsValid() || SlotViewModel->bIsEmpty)
	{
		return FText::FromString(TEXT("空槽"));
	}
	return UDOItemTooltipViewModel::BuildForInventorySlot(*SlotViewModel, ViewModel.Get());
}

const FSlateBrush* SDOInventorySlotWidget::GetIconBrush() const
{
	return LoadedIconBrush.IsValid()
		? LoadedIconBrush.Get()
		: FDOInventoryStyle::GetBrush("DOInventory.PlaceholderIcon");
}

void SDOInventorySlotWidget::RequestIconLoad()
{
	LoadedIconBrush.Reset();
	RequestedIconPath.Reset();
	if (!SlotViewModel.IsValid() || SlotViewModel->bIsEmpty || !SlotViewModel->Icon.ToSoftObjectPath().IsValid())
	{
		return;
	}

	RequestedIconPath = SlotViewModel->Icon.ToSoftObjectPath();
	RequestedInstanceId = SlotViewModel->GetInstanceId();
	const FSoftObjectPath RequestedPath = RequestedIconPath;
	const FGuid RequestedId = RequestedInstanceId;
	TWeakPtr<SDOInventorySlotWidget> WeakThis = SharedThis(this);
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		RequestedPath,
		FStreamableDelegate::CreateLambda([WeakThis, RequestedPath, RequestedId]
		{
			TSharedPtr<SDOInventorySlotWidget> Widget = WeakThis.Pin();
			if (!Widget.IsValid() || Widget->RequestedIconPath != RequestedPath || Widget->RequestedInstanceId != RequestedId
				|| !Widget->SlotViewModel.IsValid() || Widget->SlotViewModel->GetInstanceId() != RequestedId)
			{
				return;
			}

			UTexture2D* Texture = Cast<UTexture2D>(RequestedPath.ResolveObject());
			if (Texture)
			{
				Widget->LoadedIconBrush = MakeShared<FSlateBrush>();
				Widget->LoadedIconBrush->DrawAs = ESlateBrushDrawType::Image;
				Widget->LoadedIconBrush->SetResourceObject(Texture);
				Widget->LoadedIconBrush->ImageSize = FVector2D(58.0f, 58.0f);
			}
			Widget->Invalidate(EInvalidateWidgetReason::Paint);
		}));
}

FReply SDOInventorySlotWidget::OnMouseButtonDown(const FGeometry& /*MyGeometry*/, const FPointerEvent& MouseEvent)
{
	if (SlotViewModel.IsValid() && SlotViewModel->bIsPending)
	{
		return FReply::Handled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty)
		{
			if (OnSelected.IsBound())
			{
				OnSelected.Execute(SlotViewModel->GetInstanceId());
			}
			if (ContextMenuAnchor.IsValid())
			{
				ContextMenuAnchor->SetIsOpen(true);
			}
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (OnSelected.IsBound() && SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty)
		{
			OnSelected.Execute(SlotViewModel->GetInstanceId());
		}
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply SDOInventorySlotWidget::OnMouseButtonDoubleClick(const FGeometry& /*MyGeometry*/, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && OnActivated.IsBound() && SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty && !SlotViewModel->bIsPending)
	{
		OnActivated.Execute(SlotViewModel->GetInstanceId());
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SDOInventorySlotWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if ((InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::Virtual_Gamepad_Accept.GetVirtualKey())
		&& SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty && !SlotViewModel->bIsPending && OnActivated.IsBound())
	{
		OnActivated.Execute(SlotViewModel->GetInstanceId());
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SDOInventorySlotWidget::OnDragDetected(const FGeometry& /*MyGeometry*/, const FPointerEvent& MouseEvent)
{
	if (!SlotViewModel.IsValid() || SlotViewModel->bIsEmpty || SlotViewModel->bIsPending)
	{
		return FReply::Unhandled();
	}

	TSharedRef<FDOInventoryDragDropOperation> DragOperation = MakeShared<FDOInventoryDragDropOperation>();
	DragOperation->InstanceId = SlotViewModel->GetInstanceId();
	DragOperation->SourceDomain = EDOItemOperationDomain::Inventory;
	DragOperation->SourceSlotIndex = SlotViewModel->GetSlotIndex();
	DragOperation->RequestedCount = SlotViewModel->Item.StackCount;
	DragOperation->bSplitRequested = MouseEvent.IsShiftDown() && SlotViewModel->Item.StackCount > 1;
	return FReply::Handled().BeginDragDrop(DragOperation);
}

FReply SDOInventorySlotWidget::OnDrop(const FGeometry& /*MyGeometry*/, const FDragDropEvent& DragDropEvent)
{
	const TSharedPtr<FDOInventoryDragDropOperation> DragOperation = DragDropEvent.GetOperationAs<FDOInventoryDragDropOperation>();
	if (DragOperation.IsValid() && OnDropped.IsBound())
	{
		OnDropped.Execute(DragOperation->InstanceId, SlotIndex, DragOperation->bSplitRequested);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SDOEquipmentSlotWidget::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;
	SlotViewModel = InArgs._SlotViewModel;
	SlotTag = InArgs._SlotTag;
	DisplayName = InArgs._DisplayName;
	OnDropped = InArgs._OnDropped;
	OnClicked = InArgs._OnClicked;
	RequestIconLoad();

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(104.0f)
		.HeightOverride(66.0f)
		[
			SNew(SBorder)
			.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.ArtEquipmentSlot"))
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(9.0f)
			.ToolTipText(this, &SDOEquipmentSlotWidget::GetTooltipText)
			[
				SNew(SOverlay)
				+ SOverlay::Slot().Padding(3.0f)
				[
					SNew(SBorder)
					.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.Slot"))
					.BorderBackgroundColor_Lambda([this] { return GetBorderColor(); })
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(42.0f)
					.HeightOverride(42.0f)
					[
						SNew(SImage).Image_Lambda([this] { return GetIconBrush(); })
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Padding(47.0f, 0.0f, 2.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this] { return GetSlotText(); })
					.Font(FDOInventoryStyle::GetBodyTextStyle().Font)
					.AutoWrapText(true)
				]
			]
		]
	];
}

FSlateColor SDOEquipmentSlotWidget::GetBorderColor() const
{
	if (SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty)
	{
		return GetQualityColor(SlotViewModel->Rarity);
	}
	return FSlateColor(FLinearColor(0.36f, 0.14f, 0.04f, 1.0f));
}

FText SDOEquipmentSlotWidget::GetSlotText() const
{
	if (SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty)
	{
		if (SlotViewModel.IsValid())
		{
			return FText::Format(
				FText::FromString(TEXT("{0}\n{1}")),
				DisplayName,
				SlotViewModel->ItemDisplayName.IsEmpty()
					? FText::FromName(SlotViewModel->Item.DefinitionId.PrimaryAssetName)
					: SlotViewModel->ItemDisplayName);
		}
	}
	return FText::Format(FText::FromString(TEXT("{0}\n空槽\n拖入对应部位装备")), DisplayName);
}

FText SDOEquipmentSlotWidget::GetTooltipText() const
{
	return UDOItemTooltipViewModel::BuildForEquipmentSlot(SlotTag, DisplayName, ViewModel.Get());
}

const FSlateBrush* SDOEquipmentSlotWidget::GetIconBrush() const
{
	const FSoftObjectPath CurrentPath = SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty
		? SlotViewModel->Icon.ToSoftObjectPath()
		: FSoftObjectPath();
	if (CurrentPath != RequestedIconPath)
	{
		const_cast<SDOEquipmentSlotWidget*>(this)->RequestIconLoad();
	}
	return LoadedIconBrush.IsValid()
		? LoadedIconBrush.Get()
		: FDOInventoryStyle::GetBrush("DOInventory.PlaceholderIcon");
}

void SDOEquipmentSlotWidget::RequestIconLoad()
{
	LoadedIconBrush.Reset();
	RequestedIconPath.Reset();
	if (!SlotViewModel.IsValid() || SlotViewModel->bIsEmpty || !SlotViewModel->Icon.ToSoftObjectPath().IsValid())
	{
		return;
	}

	RequestedIconPath = SlotViewModel->Icon.ToSoftObjectPath();
	const FSoftObjectPath RequestedPath = RequestedIconPath;
	TWeakPtr<SDOEquipmentSlotWidget> WeakThis = SharedThis(this);
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		RequestedPath,
		FStreamableDelegate::CreateLambda([WeakThis, RequestedPath]
		{
			TSharedPtr<SDOEquipmentSlotWidget> Widget = WeakThis.Pin();
			if (!Widget.IsValid() || Widget->RequestedIconPath != RequestedPath || !Widget->SlotViewModel.IsValid()
				|| Widget->SlotViewModel->Icon.ToSoftObjectPath() != RequestedPath)
			{
				return;
			}

			if (UTexture2D* Texture = Cast<UTexture2D>(RequestedPath.ResolveObject()))
			{
				Widget->LoadedIconBrush = MakeShared<FSlateBrush>();
				Widget->LoadedIconBrush->DrawAs = ESlateBrushDrawType::Image;
				Widget->LoadedIconBrush->SetResourceObject(Texture);
				Widget->LoadedIconBrush->ImageSize = FVector2D(42.0f, 42.0f);
			}
			Widget->Invalidate(EInvalidateWidgetReason::Paint);
		}));
}

FReply SDOEquipmentSlotWidget::OnMouseButtonDown(const FGeometry& /*MyGeometry*/, const FPointerEvent& MouseEvent)
{
	const bool bHasEquippedItem = SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty;
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bHasEquippedItem && OnClicked.IsBound())
	{
		OnClicked.Execute(SlotTag);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SDOEquipmentSlotWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const bool bHasEquippedItem = SlotViewModel.IsValid() && !SlotViewModel->bIsEmpty;
	if ((InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::Virtual_Gamepad_Accept.GetVirtualKey()) && bHasEquippedItem && OnClicked.IsBound())
	{
		OnClicked.Execute(SlotTag);
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SDOEquipmentSlotWidget::OnDrop(const FGeometry& /*MyGeometry*/, const FDragDropEvent& DragDropEvent)
{
	const TSharedPtr<FDOInventoryDragDropOperation> DragOperation = DragDropEvent.GetOperationAs<FDOInventoryDragDropOperation>();
	if (DragOperation.IsValid() && DragOperation->InstanceId.IsValid() && OnDropped.IsBound())
	{
		OnDropped.Execute(DragOperation->InstanceId, SlotTag);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SDOInventoryEquipmentPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;
	OnCloseRequested = InArgs._OnCloseRequested;

	SAssignNew(RootOverlay, SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SImage)
		.Image(FDOInventoryStyle::GetBrush("DOInventory.ArtBackdrop"))
	]
	+ SOverlay::Slot()
	[
		SNew(SBorder)
		.BorderImage(GetWhiteBrush())
		.BorderBackgroundColor(FLinearColor(0.98f, 0.66f, 0.18f, 1.0f))
		.Padding(2.0f)
		[
			SNew(SBorder)
			.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.Panel"))
			.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.66f))
			.Padding(18.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[BuildTopBar()]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 12.0f)[SNew(SSeparator).ColorAndOpacity(FLinearColor(0.96f, 0.52f, 0.12f, 0.85f))]
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.46f).Padding(0.0f, 0.0f, 12.0f, 0.0f)[BuildCharacterPanel()]
					+ SHorizontalBox::Slot().FillWidth(0.54f)[BuildInventoryPanel()]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 12.0f, 0.0f, 0.0f)[BuildActionBar()]
			]
		]
	]
	+ SOverlay::Slot()
	[
		// 数量确认期间覆盖整页，避免点击穿透到背包格或底部操作栏。
		SNew(SBorder)
		.Visibility_Lambda([this] { return GetQuantityDialogVisibility(); })
		.BorderImage(GetWhiteBrush())
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.62f))
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Visibility_Lambda([this] { return GetQuantityDialogVisibility(); })
		.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.SubPanel"))
		.BorderBackgroundColor(FLinearColor::White)
		.Padding(24.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(STextBlock).Text_Lambda([this] { return GetQuantityDialogTitle(); }).Font(FCoreStyle::Get().GetFontStyle("EmbossedText"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 14.0f)[
				SNew(SSpinBox<int32>)
				.MinValue(1)
				.MaxValue_Lambda([this] { return QuantityDialogMaxCount; })
				.Value_Lambda([this] { return QuantityDialogCount; })
				.OnValueChanged_Lambda([this](int32 NewValue) { QuantityDialogCount = FMath::Clamp(NewValue, 1, QuantityDialogMaxCount); })
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)[SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).Text(FText::FromString(TEXT("取消"))).OnClicked(this, &SDOInventoryEquipmentPanel::HandleCancelQuantity)]
				+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).Text(FText::FromString(TEXT("确认"))).OnClicked(this, &SDOInventoryEquipmentPanel::HandleConfirmQuantity)]
			]
		]
	];

	ChildSlot[RootOverlay.ToSharedRef()];

	if (ViewModel.IsValid())
	{
		ViewModelChangedHandle = ViewModel->OnChanged().AddSP(SharedThis(this), &SDOInventoryEquipmentPanel::HandleViewModelChanged);
	}
}

SDOInventoryEquipmentPanel::~SDOInventoryEquipmentPanel()
{
	if (ViewModel.IsValid() && ViewModelChangedHandle.IsValid())
	{
		ViewModel->OnChanged().Remove(ViewModelChangedHandle);
	}
}

FReply SDOInventoryEquipmentPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (bQuantityDialogVisible && InKeyEvent.GetKey() == EKeys::Escape)
	{
		bQuantityDialogVisible = false;
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SDOInventoryEquipmentPanel::FocusInitialWidget()
{
	if (InventoryTileView.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetKeyboardFocus(InventoryTileView.ToSharedRef(), EFocusCause::SetDirectly);
		if (ViewModel.IsValid())
		{
			for (const TSharedPtr<FDOInventorySlotViewModel>& Slot : ViewModel->GetVisibleSlots())
			{
				if (Slot.IsValid() && !Slot->bIsEmpty)
				{
					ViewModel->SelectInstance(Slot->GetInstanceId());
					InventoryTileView->SetSelection(Slot, ESelectInfo::Direct);
					break;
				}
			}
		}
	}
}

TSharedRef<SWidget> SDOInventoryEquipmentPanel::BuildTopBar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("角色背包")))
				.TextStyle(&FDOInventoryStyle::GetTitleTextStyle())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("装备、物品与属性")))
				.TextStyle(&FDOInventoryStyle::GetBodyTextStyle())
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
		[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.ToolTipText(FText::FromString(TEXT("关闭背包（Esc）")))
			.Text(FText::FromString(TEXT("X")))
			.OnClicked(this, &SDOInventoryEquipmentPanel::HandleCloseRequested)
		];
}

TSharedRef<SWidget> SDOInventoryEquipmentPanel::BuildCharacterPanel()
{
	TSharedRef<SVerticalBox> LeftEquipmentSlots = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> RightEquipmentSlots = SNew(SVerticalBox);
	if (ViewModel.IsValid())
	{
		int32 SlotIndex = 0;
		for (const TSharedPtr<FDOEquipmentSlotViewModel>& Slot : ViewModel->GetEquipmentSlots())
		{
			if (Slot.IsValid())
			{
				if (SlotIndex % 2 == 0)
				{
					LeftEquipmentSlots->AddSlot().AutoHeight().Padding(2.0f)[BuildEquipmentSlot(Slot->SlotTag, Slot->DisplayName)];
				}
				else
				{
					RightEquipmentSlots->AddSlot().AutoHeight().Padding(2.0f)[BuildEquipmentSlot(Slot->SlotTag, Slot->DisplayName)];
				}
				++SlotIndex;
			}
		}
	}

	return SNew(SBorder)
		.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.SubPanel"))
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.62f))
		.Padding(14.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("角色状态")))
				.TextStyle(&FDOInventoryStyle::GetTitleTextStyle())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f)[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)[SNew(STextBlock).Text_Lambda([this] { return GetCombatPowerText(); }).ColorAndOpacity(FLinearColor(1.0f, 0.78f, 0.18f))]
				+ SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text_Lambda([this] { return GetGuardPowerText(); }).ColorAndOpacity(FLinearColor(1.0f, 0.68f, 0.34f))]
			]
			+ SVerticalBox::Slot().FillHeight(1.0f).MinHeight(280.0f).Padding(0.0f, 4.0f)[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[LeftEquipmentSlots]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8.0f, 0.0f)[
					SNew(SBorder)
					.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.Panel"))
					.BorderBackgroundColor(FLinearColor(1.0f, 0.55f, 0.12f, 0.95f))
					.Padding(8.0f)
					[
						SNew(SImage).Image_Lambda([this] { return GetPreviewBrush(); })
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[RightEquipmentSlots]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)[
				SNew(SBorder)
				.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.Slot"))
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(8.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]
					{
						return FText::Format(FText::FromString(TEXT("当前选择：{0}    数量：{1}")), GetSelectedNameText(), GetSelectedStackText());
					})
					.TextStyle(&FDOInventoryStyle::GetBodyTextStyle())
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 0.0f)[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("角色属性")))
				.TextStyle(&FDOInventoryStyle::GetTitleTextStyle())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)[
				SNew(STextBlock).Text_Lambda([this] { return GetAttributeSummaryText(); }).TextStyle(&FDOInventoryStyle::GetBodyTextStyle()).AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> SDOInventoryEquipmentPanel::BuildEquipmentSlot(const FGameplayTag& SlotTag, const FText& DisplayName)
{
	return SNew(SDOEquipmentSlotWidget)
		.ViewModel(ViewModel)
		.SlotViewModel(ViewModel.IsValid() ? ViewModel->GetOrCreateEquipmentSlot(SlotTag) : nullptr)
		.SlotTag(SlotTag)
		.DisplayName(DisplayName)
		.OnDropped(FDOOnEquipmentSlotDropped::CreateSP(this, &SDOInventoryEquipmentPanel::HandleEquipmentSlotDropped))
		.OnClicked(FDOOnEquipmentSlotClicked::CreateSP(this, &SDOInventoryEquipmentPanel::HandleUnequip));
}

TSharedRef<SWidget> SDOInventoryEquipmentPanel::BuildInventoryPanel()
{
	SAssignNew(CategoryBox, SVerticalBox);
	if (ViewModel.IsValid())
	{
		for (const FDOInventoryCategoryOption& Category : ViewModel->GetCategories())
		{
			CategoryBox->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)[BuildCategoryButton(Category)];
		}
	}

	SAssignNew(InventoryTileView, STileView<TSharedPtr<FDOInventorySlotViewModel>>)
		.ListItemsSource(ViewModel.IsValid() ? &ViewModel->GetVisibleSlotsMutable() : nullptr)
		// 保留 TileView 的单选状态，让键盘/手柄方向键能在格子之间移动焦点。
		.SelectionMode(ESelectionMode::Single)
		.ItemWidth(84.0f)
		.ItemHeight(84.0f)
		.OnSelectionChanged(this, &SDOInventoryEquipmentPanel::HandleTileSelectionChanged)
		.OnGenerateTile(this, &SDOInventoryEquipmentPanel::GenerateInventoryTile);

	return SNew(SBorder)
		.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.SubPanel"))
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.62f))
		.Padding(14.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("物品背包"))).TextStyle(&FDOInventoryStyle::GetTitleTextStyle())]
				+ SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text_Lambda([this] { return GetCapacityText(); }).TextStyle(&FDOInventoryStyle::GetBodyTextStyle())]
			]
			+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 10.0f)[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 10.0f, 0.0f)[
					SNew(SScrollBox)
					.Orientation(Orient_Vertical)
					+ SScrollBox::Slot()[CategoryBox.ToSharedRef()]
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f)[InventoryTileView.ToSharedRef()]
			]
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)[SNew(STextBlock).Text_Lambda([this] { return GetPageText(); }).TextStyle(&FDOInventoryStyle::GetBodyTextStyle())]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
					SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).Text(FText::FromString(TEXT("<"))).OnClicked(this, &SDOInventoryEquipmentPanel::HandlePreviousPage)
				]
				+ SHorizontalBox::Slot().AutoWidth()[
					SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).Text(FText::FromString(TEXT(">"))).OnClicked(this, &SDOInventoryEquipmentPanel::HandleNextPage)
				]
			]
		];
}

TSharedRef<SWidget> SDOInventoryEquipmentPanel::BuildCategoryButton(const FDOInventoryCategoryOption& Category)
{
	return SNew(SButton)
		.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
		.ButtonColorAndOpacity_Lambda([this, Category]
		{
			return ViewModel.IsValid() && ViewModel->GetCurrentFilter() == Category.FilterTag
				? FLinearColor(0.82f, 0.38f, 0.95f, 1.0f)
				: FLinearColor(0.40f, 0.16f, 0.44f, 1.0f);
		})
		.OnClicked_Lambda([this, Category]
		{
			if (ViewModel.IsValid()) ViewModel->SetFilter(Category.FilterTag);
			return FReply::Handled();
		})
		[
			SNew(SBox)
			.WidthOverride(96.0f)
			.MinDesiredHeight(28.0f)
			.Padding(FMargin(10.0f, 3.0f))
			[
				SNew(STextBlock)
				.Text(Category.DisplayName)
				.Justification(ETextJustify::Center)
				.TextStyle(&FDOInventoryStyle::GetBodyTextStyle())
			]
		];
}

TSharedRef<SWidget> SDOInventoryEquipmentPanel::BuildActionBar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)[SNew(STextBlock).Text_Lambda([this] { return GetSelectedDescriptionText(); }).TextStyle(&FDOInventoryStyle::GetBodyTextStyle()).AutoWrapText(true)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this]
			{
				const FDOInventorySlotViewModel* Slot = GetSelectedSlot();
				return Slot && !Slot->bIsPending && Slot->bIsUsable;
			})
			.ToolTipText(FText::FromString(TEXT("选择消耗品后使用。")))
			.Text(FText::FromString(TEXT("使用")))
			.OnClicked(this, &SDOInventoryEquipmentPanel::HandleActivateSelected)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this]
			{
				const FDOInventorySlotViewModel* Slot = GetSelectedSlot();
				return Slot && !Slot->bIsPending && Slot->EquipmentSlotTag.IsValid();
			})
			.ToolTipText(FText::FromString(TEXT("选择装备后穿戴。")))
			.Text(FText::FromString(TEXT("穿戴")))
			.OnClicked(this, &SDOInventoryEquipmentPanel::HandleEquipSelected)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this] { const FDOInventorySlotViewModel* Slot = GetSelectedSlot(); return Slot && !Slot->bIsPending && Slot->bIsUsable; })
			.ToolTipText(FText::FromString(TEXT("选择消耗品后绑定到快捷栏 1。")))
			.Text(FText::FromString(TEXT("绑定1")))
			.OnClicked_Lambda([this] { return HandleAssignToQuickBar(0); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this] { const FDOInventorySlotViewModel* Slot = GetSelectedSlot(); return Slot && !Slot->bIsPending && Slot->bIsUsable; })
			.ToolTipText(FText::FromString(TEXT("选择消耗品后绑定到快捷栏 2。")))
			.Text(FText::FromString(TEXT("绑定2")))
			.OnClicked_Lambda([this] { return HandleAssignToQuickBar(1); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this] { const FDOInventorySlotViewModel* Slot = GetSelectedSlot(); return Slot && !Slot->bIsPending && Slot->bIsUsable; })
			.ToolTipText(FText::FromString(TEXT("选择消耗品后绑定到快捷栏 3。")))
			.Text(FText::FromString(TEXT("绑定3")))
			.OnClicked_Lambda([this] { return HandleAssignToQuickBar(2); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this] { const FDOInventorySlotViewModel* Slot = GetSelectedSlot(); return Slot && !Slot->bIsPending && Slot->bIsUsable; })
			.ToolTipText(FText::FromString(TEXT("选择消耗品后绑定到快捷栏 4。")))
			.Text(FText::FromString(TEXT("绑定4")))
			.OnClicked_Lambda([this] { return HandleAssignToQuickBar(3); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).Text(FText::FromString(TEXT("整理"))).OnClicked(this, &SDOInventoryEquipmentPanel::HandleSort)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[
			SNew(SButton)
			.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
			.IsEnabled_Lambda([this]
			{
				const FDOInventorySlotViewModel* Slot = GetSelectedSlot();
				return Slot && !Slot->bIsPending && Slot->bCanDiscard;
			})
			.ToolTipText(FText::FromString(TEXT("选择可丢弃物品后丢弃。")))
			.Text(FText::FromString(TEXT("丢弃")))
			.OnClicked(this, &SDOInventoryEquipmentPanel::HandleDiscard)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).IsEnabled(false).ToolTipText(FText::FromString(TEXT("出售功能将在商店交互系统接入后启用。"))).Text(FText::FromString(TEXT("出售")))]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).IsEnabled(false).ToolTipText(FText::FromString(TEXT("仓库功能将在仓库交互系统接入后启用。"))).Text(FText::FromString(TEXT("仓库")))]
		+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).ButtonStyle(&FDOInventoryStyle::GetButtonStyle()).IsEnabled(false).ToolTipText(FText::FromString(TEXT("修理功能将在耐久和修理系统接入后启用。"))).Text(FText::FromString(TEXT("修理")))];
}

TSharedRef<ITableRow> SDOInventoryEquipmentPanel::GenerateInventoryTile(TSharedPtr<FDOInventorySlotViewModel> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const int32 SlotIndex = Item.IsValid() ? Item->GetSlotIndex() : INDEX_NONE;
	return SNew(STableRow<TSharedPtr<FDOInventorySlotViewModel>>, OwnerTable)
		[
			SNew(SDOInventorySlotWidget)
			.SlotViewModel(Item)
			.ViewModel(ViewModel)
			.SlotIndex(SlotIndex)
			.OnSelected(FDOOnInventorySlotSelected::CreateSP(this, &SDOInventoryEquipmentPanel::HandleSlotSelected))
			.OnActivated(FDOOnInventorySlotActivated::CreateSP(this, &SDOInventoryEquipmentPanel::HandleSlotActivated))
			.OnDropped(FDOOnInventorySlotDropped::CreateSP(this, &SDOInventoryEquipmentPanel::HandleSlotDropped))
			.OnContextAction(FDOOnInventoryContextAction::CreateSP(this, &SDOInventoryEquipmentPanel::HandleContextAction))
		];
}

void SDOInventoryEquipmentPanel::HandleTileSelectionChanged(TSharedPtr<FDOInventorySlotViewModel> Item, ESelectInfo::Type /*SelectInfo*/)
{
	if (ViewModel.IsValid() && Item.IsValid() && !Item->bIsEmpty)
	{
		ViewModel->SelectInstance(Item->GetInstanceId());
	}
}

void SDOInventoryEquipmentPanel::HandleViewModelChanged()
{
	RefreshTileView();
}

void SDOInventoryEquipmentPanel::RefreshTileView()
{
	if (InventoryTileView.IsValid())
	{
		if (ViewModel.IsValid() && ViewModel->ConsumeVisibleSlotStructureChange())
		{
			InventoryTileView->RequestListRefresh();
		}
		else
		{
			InventoryTileView->Invalidate(EInvalidateWidgetReason::Paint);
		}
	}
}

void SDOInventoryEquipmentPanel::HandleSlotSelected(const FGuid& InstanceId)
{
	if (ViewModel.IsValid()) ViewModel->SelectInstance(InstanceId);
}

void SDOInventoryEquipmentPanel::HandleSlotActivated(const FGuid& InstanceId)
{
	if (ViewModel.IsValid())
	{
		ViewModel->SelectInstance(InstanceId);
		const FDOInventorySlotViewModel* Slot = ViewModel->FindVisibleSlot(InstanceId);
		if (!Slot || Slot->bIsPending)
		{
			return;
		}
		if (Slot->bIsUsable)
		{
			ViewModel->RequestActivateSelected();
		}
		else
		{
			ViewModel->RequestEquipSelected();
		}
	}
}

void SDOInventoryEquipmentPanel::HandleSlotDropped(const FGuid& InstanceId, const int32 TargetSlot, const bool bSplitRequested)
{
	if (ViewModel.IsValid() && TargetSlot != INDEX_NONE)
	{
		const FDOInventorySlotViewModel* Source = ViewModel->FindVisibleSlot(InstanceId);
		if (!Source || Source->bIsPending)
		{
			return;
		}
		if (bSplitRequested && Source->Item.StackCount > 1)
		{
			OpenQuantityDialog(true, InstanceId, TargetSlot, Source->Item.StackCount - 1);
		}
		else
		{
			ViewModel->RequestMoveOrEquip(InstanceId, Source ? Source->GetSlotIndex() : INDEX_NONE, TargetSlot);
		}
	}
}

void SDOInventoryEquipmentPanel::HandleContextAction(const FGuid& InstanceId, const uint8 ActionValue)
{
	if (!ViewModel.IsValid() || !InstanceId.IsValid())
	{
		return;
	}

	const FDOInventorySlotViewModel* Slot = ViewModel->FindVisibleSlot(InstanceId);
	if (!Slot || Slot->bIsEmpty || Slot->bIsPending)
	{
		return;
	}

	ViewModel->SelectInstance(InstanceId);
	switch (static_cast<EDOInventoryContextAction>(ActionValue))
	{
	case EDOInventoryContextAction::Use:
		ViewModel->RequestActivateSelected();
		break;
	case EDOInventoryContextAction::Equip:
		ViewModel->RequestEquipSelected();
		break;
	case EDOInventoryContextAction::AssignQuickBar1:
		ViewModel->RequestAssignSelectedToQuickBar(0);
		break;
	case EDOInventoryContextAction::AssignQuickBar2:
		ViewModel->RequestAssignSelectedToQuickBar(1);
		break;
	case EDOInventoryContextAction::AssignQuickBar3:
		ViewModel->RequestAssignSelectedToQuickBar(2);
		break;
	case EDOInventoryContextAction::AssignQuickBar4:
		ViewModel->RequestAssignSelectedToQuickBar(3);
		break;
	case EDOInventoryContextAction::Discard:
		HandleDiscard();
		break;
	default:
		break;
	}
}

void SDOInventoryEquipmentPanel::HandleEquipmentSlotDropped(const FGuid& InstanceId, const FGameplayTag& SlotTag)
{
	if (!ViewModel.IsValid() || !InstanceId.IsValid() || !SlotTag.IsValid())
	{
		return;
	}

	const FDOInventorySlotViewModel* Source = ViewModel->FindVisibleSlot(InstanceId);
	if (Source && !Source->bIsEmpty && !Source->bIsPending && Source->EquipmentSlotTag == SlotTag)
	{
		// 目标槽只负责发起装备请求，最终部位、等级和权限仍由服务器校验。
		ViewModel->RequestEquipInstance(InstanceId);
	}
}

FReply SDOInventoryEquipmentPanel::HandlePreviousPage()
{
	if (ViewModel.IsValid()) ViewModel->SetPage(ViewModel->GetCurrentPage() - 1);
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleNextPage()
{
	if (ViewModel.IsValid()) ViewModel->SetPage(ViewModel->GetCurrentPage() + 1);
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleSort()
{
	if (ViewModel.IsValid()) ViewModel->RequestSort();
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleDiscard()
{
	if (ViewModel.IsValid())
	{
		const FGuid SelectedId = ViewModel->GetSelectedInstanceId();
		if (const FDOInventorySlotViewModel* Slot = ViewModel->FindVisibleSlot(SelectedId))
		{
			if (!Slot->bIsEmpty)
			{
				OpenQuantityDialog(false, SelectedId, INDEX_NONE, Slot->Item.StackCount);
			}
		}
	}
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleConfirmQuantity()
{
	if (ViewModel.IsValid() && bQuantityDialogVisible)
	{
		if (bQuantityDialogIsSplit)
		{
			ViewModel->RequestSplitStack(QuantityDialogInstanceId, QuantityDialogTargetSlot, QuantityDialogCount);
		}
		else
		{
			ViewModel->SelectInstance(QuantityDialogInstanceId);
			ViewModel->RequestDiscardSelectedCount(QuantityDialogCount);
		}
	}

	bQuantityDialogVisible = false;
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleCancelQuantity()
{
	bQuantityDialogVisible = false;
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleActivateSelected()
{
	if (ViewModel.IsValid()) ViewModel->RequestActivateSelected();
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleEquipSelected()
{
	if (ViewModel.IsValid()) ViewModel->RequestEquipSelected();
	return FReply::Handled();
}

FReply SDOInventoryEquipmentPanel::HandleAssignToQuickBar(const int32 QuickBarSlot)
{
	if (ViewModel.IsValid()) ViewModel->RequestAssignSelectedToQuickBar(QuickBarSlot);
	return FReply::Handled();
}

void SDOInventoryEquipmentPanel::HandleUnequip(const FGameplayTag& SlotTag)
{
	if (ViewModel.IsValid()) ViewModel->RequestUnequip(SlotTag);
}

FText SDOInventoryEquipmentPanel::GetPageText() const
{
	return ViewModel.IsValid()
		? FText::Format(FText::FromString(TEXT("第 {0} / {1} 页")), FText::AsNumber(ViewModel->GetCurrentPage() + 1), FText::AsNumber(ViewModel->GetPageCount()))
		: FText::FromString(TEXT("第 1 / 1 页"));
}

FText SDOInventoryEquipmentPanel::GetCapacityText() const
{
	if (!ViewModel.IsValid() || !ViewModel->GetInventoryComponent())
	{
		return FText::FromString(TEXT("0 / 0 格"));
	}

	return FText::Format(
		FText::FromString(TEXT("{0} / {1} 格")),
		FText::AsNumber(ViewModel->GetInventoryComponent()->GetUsedSlotCount()),
		FText::AsNumber(ViewModel->GetInventoryComponent()->GetCapacity()));
}

FReply SDOInventoryEquipmentPanel::HandleCloseRequested()
{
	if (OnCloseRequested.IsBound())
	{
		OnCloseRequested.Execute();
	}
	return FReply::Handled();
}

FText SDOInventoryEquipmentPanel::GetSelectedNameText() const
{
	return ViewModel.IsValid() ? ViewModel->GetSelectedDisplayName() : FText::GetEmpty();
}

FText SDOInventoryEquipmentPanel::GetSelectedDescriptionText() const
{
	return ViewModel.IsValid() ? ViewModel->GetSelectedDescription() : FText::GetEmpty();
}

FText SDOInventoryEquipmentPanel::GetSelectedStackText() const
{
	return ViewModel.IsValid() ? FText::AsNumber(ViewModel->GetSelectedStackCount()) : FText::GetEmpty();
}

const FDOInventorySlotViewModel* SDOInventoryEquipmentPanel::GetSelectedSlot() const
{
	return ViewModel.IsValid() ? ViewModel->FindVisibleSlot(ViewModel->GetSelectedInstanceId()) : nullptr;
}

FText SDOInventoryEquipmentPanel::GetCombatPowerText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::FromString(TEXT("战力：0"));
	}
	return FText::Format(
		FText::FromString(TEXT("战力：{0}")),
		FText::AsNumber(ViewModel->GetAttributeSnapshot().CombatPower));
}

FText SDOInventoryEquipmentPanel::GetGuardPowerText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::FromString(TEXT("守护力：0"));
	}
	return FText::Format(
		FText::FromString(TEXT("守护力：{0}")),
		FText::AsNumber(ViewModel->GetAttributeSnapshot().GuardPower));
}

FText SDOInventoryEquipmentPanel::GetAttributeSummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	const FDOInventoryAttributeSnapshot& Attributes = ViewModel->GetAttributeSnapshot();
	return FText::Format(
		FText::FromString(TEXT("攻击 {0}    防御 {1}\n生命上限 {2}    法力上限 {3}\n暴击 {4}    命中 {5}\n闪避 {6}    攻速 {7}\n移速 {8}    吸血 {9}")),
		FText::AsNumber(Attributes.AttackPower),
		FText::AsNumber(Attributes.DefensePower),
		FText::AsNumber(Attributes.MaxHealth),
		FText::AsNumber(Attributes.MaxMana),
		FText::AsNumber(Attributes.CriticalRating),
		FText::AsNumber(Attributes.HitRating),
		FText::AsNumber(Attributes.EvasionRating),
		FText::AsNumber(Attributes.AttackSpeed),
		FText::AsNumber(Attributes.MoveSpeed),
		FText::AsNumber(Attributes.LifeStealRate));
}

const FSlateBrush* SDOInventoryEquipmentPanel::GetPreviewBrush() const
{
	if (!PreviewBrush.IsValid())
	{
		PreviewBrush = MakeShared<FSlateBrush>();
		PreviewBrush->DrawAs = ESlateBrushDrawType::Image;
		PreviewBrush->SetImageSize(FVector2D(300.0f, 300.0f));
	}

	if (ViewModel.IsValid())
	{
		if (UTextureRenderTarget2D* RenderTarget = ViewModel->GetPreviewRenderTarget())
		{
			if (PreviewBrush->GetResourceObject() != RenderTarget)
			{
				PreviewBrush->SetResourceObject(RenderTarget);
			}
			return PreviewBrush.Get();
		}
	}

	return FDOInventoryStyle::GetBrush("DOInventory.PlaceholderIcon");
}

FText SDOInventoryEquipmentPanel::GetQuantityDialogTitle() const
{
	return bQuantityDialogIsSplit
		? FText::FromString(TEXT("拆分堆栈"))
		: FText::FromString(TEXT("丢弃数量"));
}

EVisibility SDOInventoryEquipmentPanel::GetQuantityDialogVisibility() const
{
	return bQuantityDialogVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

void SDOInventoryEquipmentPanel::OpenQuantityDialog(const bool bSplit, const FGuid& InstanceId, const int32 TargetSlot, const int32 MaxCount)
{
	if (!InstanceId.IsValid() || MaxCount <= 0)
	{
		return;
	}

	bQuantityDialogVisible = true;
	bQuantityDialogIsSplit = bSplit;
	QuantityDialogInstanceId = InstanceId;
	QuantityDialogTargetSlot = TargetSlot;
	QuantityDialogMaxCount = MaxCount;
	QuantityDialogCount = bSplit ? FMath::Max(1, MaxCount / 2) : MaxCount;
}
