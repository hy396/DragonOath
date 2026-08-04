#include "UI/Inventory/DOInventoryStyle.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FDOInventoryStyle::StyleSet;

namespace
{
	FSlateBrush MakeBoxBrush(const FLinearColor& Color, const float Margin = 0.12f)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(Color);
		Brush.Margin = FMargin(Margin);
		return Brush;
	}

	FSlateBrush MakeImageBrush(const FLinearColor& Color)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.TintColor = FSlateColor(Color);
		return Brush;
	}
}

void FDOInventoryStyle::Initialize()
{
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = CreateStyleSet();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FDOInventoryStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		StyleSet.Reset();
	}
}

const ISlateStyle& FDOInventoryStyle::Get()
{
	Initialize();
	return *StyleSet;
}

const FSlateBrush* FDOInventoryStyle::GetBrush(const FName BrushName)
{
	return Get().GetBrush(BrushName);
}

const FButtonStyle& FDOInventoryStyle::GetButtonStyle()
{
	return Get().GetWidgetStyle<FButtonStyle>("DOInventory.Button");
}

const FTextBlockStyle& FDOInventoryStyle::GetTitleTextStyle()
{
	return Get().GetWidgetStyle<FTextBlockStyle>("DOInventory.TitleText");
}

const FTextBlockStyle& FDOInventoryStyle::GetBodyTextStyle()
{
	return Get().GetWidgetStyle<FTextBlockStyle>("DOInventory.BodyText");
}

TSharedRef<FSlateStyleSet> FDOInventoryStyle::CreateStyleSet()
{
	TSharedRef<FSlateStyleSet> NewStyle = MakeShared<FSlateStyleSet>("DOInventoryStyle");

	NewStyle->Set("DOInventory.Panel", new FSlateBrush(MakeBoxBrush(FLinearColor(0.025f, 0.035f, 0.055f, 0.96f))));
	NewStyle->Set("DOInventory.SubPanel", new FSlateBrush(MakeBoxBrush(FLinearColor(0.055f, 0.07f, 0.10f, 0.92f))));
	NewStyle->Set("DOInventory.Slot", new FSlateBrush(MakeBoxBrush(FLinearColor(0.08f, 0.10f, 0.14f, 0.92f))));
	NewStyle->Set("DOInventory.SlotSelected", new FSlateBrush(MakeBoxBrush(FLinearColor(0.10f, 0.58f, 0.58f, 1.0f))));
	NewStyle->Set("DOInventory.PlaceholderIcon", new FSlateBrush(MakeImageBrush(FLinearColor(0.20f, 0.24f, 0.30f, 1.0f))));

	FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
	ButtonStyle.Normal = MakeBoxBrush(FLinearColor(0.09f, 0.11f, 0.15f, 1.0f));
	ButtonStyle.Hovered = MakeBoxBrush(FLinearColor(0.14f, 0.22f, 0.27f, 1.0f));
	ButtonStyle.Pressed = MakeBoxBrush(FLinearColor(0.08f, 0.45f, 0.46f, 1.0f));
	ButtonStyle.Disabled = MakeBoxBrush(FLinearColor(0.05f, 0.06f, 0.08f, 0.65f));
	NewStyle->Set("DOInventory.Button", ButtonStyle);

	FTextBlockStyle TitleStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	TitleStyle.SetFont(FCoreStyle::Get().GetFontStyle("EmbossedText"));
	TitleStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.94f, 1.0f, 1.0f)));
	NewStyle->Set("DOInventory.TitleText", TitleStyle);

	FTextBlockStyle BodyStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	BodyStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.78f, 0.86f, 1.0f)));
	NewStyle->Set("DOInventory.BodyText", BodyStyle);

	return NewStyle;
}
