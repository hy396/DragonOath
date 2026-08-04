#pragma once

#include "CommonActivatableWidget.h"
#include "Input/Events.h"

class SWidget;

#include "DOInventoryScreen.generated.h"

class SDOInventoryEquipmentPanel;
class UDOInventoryViewModel;
struct FUIInputConfig;

/**
 * CommonUI 页面外壳。
 *
 * 页面生命周期、Menu 输入模式和返回由 CommonUI 管理；背包主体完全由
 * SDOInventoryEquipmentPanel 及其子 Slate 控件构建。
 */
UCLASS(BlueprintType, Blueprintable)
class DRAGONOATH_API UDOInventoryScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UDOInventoryScreen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void HandleCloseRequested();

	UPROPERTY(Transient)
	TObjectPtr<UDOInventoryViewModel> ViewModel;

	TSharedPtr<SDOInventoryEquipmentPanel> InventoryPanel;
};
