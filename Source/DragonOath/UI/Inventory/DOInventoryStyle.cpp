#include "UI/Inventory/DOInventoryStyle.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "UObject/SoftObjectPath.h"

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

	FSlateBrush MakeTextureBrush(UTexture2D* Texture, const FVector2D& ImageSize)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.SetResourceObject(Texture);
		Brush.ImageSize = ImageSize;
		return Brush;
	}

	UTexture2D* LoadInventoryTexture(const TCHAR* AssetPath)
	{
		return Cast<UTexture2D>(FSoftObjectPath(AssetPath).TryLoad());
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

	// 背包主题使用暖金、橙红和紫色强调，避免默认 Slate 灰蓝色让页面像调试面板。
	NewStyle->Set("DOInventory.Panel", new FSlateBrush(MakeBoxBrush(FLinearColor(0.12f, 0.045f, 0.018f, 0.98f), 0.08f)));
	NewStyle->Set("DOInventory.SubPanel", new FSlateBrush(MakeBoxBrush(FLinearColor(0.28f, 0.095f, 0.028f, 0.96f), 0.10f)));
	NewStyle->Set("DOInventory.Slot", new FSlateBrush(MakeBoxBrush(FLinearColor(0.46f, 0.18f, 0.035f, 1.0f), 0.12f)));
	NewStyle->Set("DOInventory.SlotSelected", new FSlateBrush(MakeBoxBrush(FLinearColor(0.98f, 0.56f, 0.08f, 1.0f), 0.12f)));
	NewStyle->Set("DOInventory.PlaceholderIcon", new FSlateBrush(MakeImageBrush(FLinearColor(0.48f, 0.22f, 0.08f, 1.0f))));

	// 这两张图属于少量常驻 UI 框架资源，可以在样式初始化时同步加载；物品图标仍在控件中异步加载。
	if (UTexture2D* PanelBackdrop = LoadInventoryTexture(TEXT("/Game/DragonOath/UI/Inventory/Frames/T_DO_Inventory_PanelBackdrop.T_DO_Inventory_PanelBackdrop")))
	{
		NewStyle->Set("DOInventory.ArtBackdrop", new FSlateBrush(MakeTextureBrush(PanelBackdrop, FVector2D(2048.0f, 1536.0f))));
	}
	else
	{
		NewStyle->Set("DOInventory.ArtBackdrop", new FSlateBrush(MakeBoxBrush(FLinearColor(0.12f, 0.045f, 0.018f, 1.0f), 0.08f)));
	}
	if (UTexture2D* EquipmentSlotFrame = LoadInventoryTexture(TEXT("/Game/DragonOath/UI/Inventory/Frames/T_DO_Inventory_EquipmentSlot_Horizontal.T_DO_Inventory_EquipmentSlot_Horizontal")))
	{
		NewStyle->Set("DOInventory.ArtEquipmentSlot", new FSlateBrush(MakeTextureBrush(EquipmentSlotFrame, FVector2D(1253.0f, 826.0f))));
	}
	else
	{
		NewStyle->Set("DOInventory.ArtEquipmentSlot", new FSlateBrush(MakeBoxBrush(FLinearColor(0.46f, 0.18f, 0.035f, 1.0f), 0.12f)));
	}

	FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
	ButtonStyle.Normal = MakeBoxBrush(FLinearColor(0.30f, 0.10f, 0.36f, 1.0f), 0.18f);
	ButtonStyle.Hovered = MakeBoxBrush(FLinearColor(0.52f, 0.18f, 0.46f, 1.0f), 0.18f);
	ButtonStyle.Pressed = MakeBoxBrush(FLinearColor(0.95f, 0.42f, 0.08f, 1.0f), 0.18f);
	ButtonStyle.Disabled = MakeBoxBrush(FLinearColor(0.22f, 0.09f, 0.07f, 0.72f), 0.18f);
	NewStyle->Set("DOInventory.Button", ButtonStyle);

	FTextBlockStyle TitleStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	TitleStyle.SetFont(FCoreStyle::Get().GetFontStyle("EmbossedText"));
	TitleStyle.SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.88f, 0.42f, 1.0f)));
	NewStyle->Set("DOInventory.TitleText", TitleStyle);

	FTextBlockStyle BodyStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	BodyStyle.SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.90f, 0.70f, 1.0f)));
	NewStyle->Set("DOInventory.BodyText", BodyStyle);

	return NewStyle;
}
