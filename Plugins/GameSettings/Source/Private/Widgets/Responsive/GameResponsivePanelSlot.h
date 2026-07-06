// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "Components/PanelSlot.h"
#include "SGameResponsivePanel.h"

#include "GameResponsivePanelSlot.generated.h"

class UObject;

UCLASS()
class UGameResponsivePanelSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

public:


public:

	void BuildSlot(TSharedRef<SGameResponsivePanel> GameResponsivePanel);

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	SGameResponsivePanel::FSlot* Slot;
};
