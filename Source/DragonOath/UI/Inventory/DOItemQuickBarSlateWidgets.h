#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FDOQuickBarSlotViewModel;
class UDOItemQuickBarViewModel;
struct FSlateBrush;

DECLARE_DELEGATE_OneParam(FDOOnQuickBarSlotActivated, int32);

/** 单个快捷栏槽位，保留按键提示、图标和当前数量。 */
class SDOItemQuickBarSlotWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDOItemQuickBarSlotWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FDOQuickBarSlotViewModel>, SlotViewModel)
		SLATE_ARGUMENT(int32, SlotIndex)
		SLATE_EVENT(FDOOnQuickBarSlotActivated, OnActivated)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply HandleClicked();
	FText GetTooltipText() const;
	FText GetStackText() const;
	const FSlateBrush* GetIconBrush() const;
	void RequestIconLoad();

	TSharedPtr<FDOQuickBarSlotViewModel> SlotViewModel;
	int32 SlotIndex = INDEX_NONE;
	FDOOnQuickBarSlotActivated OnActivated;
	FSoftObjectPath RequestedIconPath;
	TSharedPtr<FSlateBrush> LoadedIconBrush;
};

/** 战斗 HUD 的四格原生 Slate 快捷栏。 */
class SDOItemQuickBarWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDOItemQuickBarWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UDOItemQuickBarViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SDOItemQuickBarWidget();

private:
	void HandleViewModelChanged();
	void RebuildSlots();

	TWeakObjectPtr<UDOItemQuickBarViewModel> ViewModel;
	TSharedPtr<SHorizontalBox> SlotBox;
	FDelegateHandle ViewModelChangedHandle;
};
