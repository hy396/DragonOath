#include "UI/Inventory/DOItemQuickBarSlateWidgets.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "ItemSystem/QuickBar/DOItemQuickBarViewModel.h"
#include "Styling/CoreStyle.h"
#include "UI/Inventory/DOInventoryStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SDOItemQuickBarSlotWidget::Construct(const FArguments& InArgs)
{
	SlotViewModel = InArgs._SlotViewModel;
	SlotIndex = InArgs._SlotIndex;
	OnActivated = InArgs._OnActivated;
	RequestIconLoad();

	ChildSlot
	[
		SNew(SButton)
		.ButtonStyle(&FDOInventoryStyle::GetButtonStyle())
		.ContentPadding(4.0f)
		.IsEnabled_Lambda([this]
		{
			return SlotViewModel.IsValid() && SlotViewModel->bIsBound && SlotViewModel->StackCount > 0 && !SlotViewModel->bIsPending;
		})
		.OnClicked(this, &SDOItemQuickBarSlotWidget::HandleClicked)
		.ToolTipText(this, &SDOItemQuickBarSlotWidget::GetTooltipText)
		[
			SNew(SBox)
			.WidthOverride(72.0f)
			.HeightOverride(72.0f)
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
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Text(FText::AsNumber(SlotIndex + 1))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(STextBlock)
					.Text_Lambda([this] { return GetStackText(); })
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility_Lambda([this]
					{
						return SlotViewModel.IsValid() && SlotViewModel->bIsPending
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					.Text(FText::FromString(TEXT("…")))
					.ColorAndOpacity(FLinearColor(1.0f, 0.86f, 0.35f))
				]
			]
		]
	];
}

FReply SDOItemQuickBarSlotWidget::HandleClicked()
{
	if (OnActivated.IsBound())
	{
		OnActivated.Execute(SlotIndex);
	}
	return FReply::Handled();
}

FText SDOItemQuickBarSlotWidget::GetTooltipText() const
{
	if (!SlotViewModel.IsValid() || !SlotViewModel->bIsBound)
	{
		return FText::Format(FText::FromString(TEXT("快捷栏 {0}\n未绑定物品")), FText::AsNumber(SlotIndex + 1));
	}

	return FText::Format(
		FText::FromString(TEXT("快捷栏 {0}\n{1}\n数量：{2}")),
		FText::AsNumber(SlotIndex + 1),
		SlotViewModel->DisplayName,
		FText::AsNumber(SlotViewModel->StackCount));
}

FText SDOItemQuickBarSlotWidget::GetStackText() const
{
	if (!SlotViewModel.IsValid() || !SlotViewModel->bIsBound)
	{
		return FText::GetEmpty();
	}
	return FText::AsNumber(SlotViewModel->StackCount);
}

const FSlateBrush* SDOItemQuickBarSlotWidget::GetIconBrush() const
{
	return LoadedIconBrush.IsValid()
		? LoadedIconBrush.Get()
		: FDOInventoryStyle::GetBrush("DOInventory.PlaceholderIcon");
}

void SDOItemQuickBarSlotWidget::RequestIconLoad()
{
	LoadedIconBrush.Reset();
	RequestedIconPath.Reset();
	if (!SlotViewModel.IsValid() || !SlotViewModel->Icon.ToSoftObjectPath().IsValid())
	{
		return;
	}

	RequestedIconPath = SlotViewModel->Icon.ToSoftObjectPath();
	const FSoftObjectPath RequestedPath = RequestedIconPath;
	TWeakPtr<SDOItemQuickBarSlotWidget> WeakThis = SharedThis(this);
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		RequestedPath,
		FStreamableDelegate::CreateLambda([WeakThis, RequestedPath]
		{
			TSharedPtr<SDOItemQuickBarSlotWidget> Widget = WeakThis.Pin();
			if (!Widget.IsValid() || Widget->RequestedIconPath != RequestedPath)
			{
				return;
			}

			if (UTexture2D* Texture = Cast<UTexture2D>(RequestedPath.ResolveObject()))
			{
				Widget->LoadedIconBrush = MakeShared<FSlateBrush>();
				Widget->LoadedIconBrush->DrawAs = ESlateBrushDrawType::Image;
				Widget->LoadedIconBrush->SetResourceObject(Texture);
				Widget->LoadedIconBrush->ImageSize = FVector2D(52.0f, 52.0f);
			}
			Widget->Invalidate(EInvalidateWidgetReason::Paint);
		}));
}

void SDOItemQuickBarWidget::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;
	SAssignNew(SlotBox, SHorizontalBox);
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FDOInventoryStyle::GetBrush("DOInventory.SubPanel"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("物品快捷栏")))
				.Font(FCoreStyle::Get().GetFontStyle("EmbossedText"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SlotBox.ToSharedRef()
			]
		]
	];

	if (ViewModel.IsValid())
	{
		ViewModelChangedHandle = ViewModel->OnChanged().AddSP(SharedThis(this), &SDOItemQuickBarWidget::HandleViewModelChanged);
	}
	RebuildSlots();
}

SDOItemQuickBarWidget::~SDOItemQuickBarWidget()
{
	if (ViewModel.IsValid() && ViewModelChangedHandle.IsValid())
	{
		ViewModel->OnChanged().Remove(ViewModelChangedHandle);
	}
}

void SDOItemQuickBarWidget::HandleViewModelChanged()
{
	RebuildSlots();
}

void SDOItemQuickBarWidget::RebuildSlots()
{
	if (!SlotBox.IsValid())
	{
		return;
	}

	SlotBox->ClearChildren();
	if (!ViewModel.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FDOQuickBarSlotViewModel>>& Slots = ViewModel->GetSlots();
	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		SlotBox->AddSlot()
			.AutoWidth()
			.Padding(3.0f)
		[
			SNew(SDOItemQuickBarSlotWidget)
			.SlotViewModel(Slots[SlotIndex])
			.SlotIndex(SlotIndex)
			.OnActivated(FDOOnQuickBarSlotActivated::CreateUObject(ViewModel.Get(), &UDOItemQuickBarViewModel::RequestUseSlot))
		];
	}
}
