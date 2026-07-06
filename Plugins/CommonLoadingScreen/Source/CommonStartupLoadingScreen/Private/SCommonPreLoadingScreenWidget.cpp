// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：加载屏管理插件，用于聚合加载状态并显示启动/切图/副本加载界面。

#include "SCommonPreLoadingScreenWidget.h"

#include "Widgets/Layout/SBorder.h"

class FReferenceCollector;

#define LOCTEXT_NAMESPACE "SCommonPreLoadingScreenWidget"

void SCommonPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0)
	];
}

void SCommonPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	//WidgetAssets.AddReferencedObjects(Collector);
}

FString SCommonPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SCommonPreLoadingScreenWidget");
}

#undef LOCTEXT_NAMESPACE
