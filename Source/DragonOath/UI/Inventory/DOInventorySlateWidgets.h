#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Input/DragAndDrop.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STileView.h"

struct FDOInventorySlotViewModel;
class UDOInventoryViewModel;
struct FSlateBrush;
class SOverlay;
class SMenuAnchor;

DECLARE_DELEGATE_OneParam(FDOOnInventorySlotSelected, const FGuid&);
DECLARE_DELEGATE_OneParam(FDOOnInventorySlotActivated, const FGuid&);
DECLARE_DELEGATE_ThreeParams(FDOOnInventorySlotDropped, const FGuid&, int32, bool);
DECLARE_DELEGATE_TwoParams(FDOOnInventoryContextAction, const FGuid&, uint8);
DECLARE_DELEGATE_TwoParams(FDOOnEquipmentSlotDropped, const FGuid&, const FGameplayTag&);
DECLARE_DELEGATE_OneParam(FDOOnEquipmentSlotClicked, const FGameplayTag&);
DECLARE_DELEGATE(FDOOnInventoryCloseRequested);

/** 物品右键菜单中的操作类型。数值会作为 Slate 菜单回调的轻量参数传递。 */
enum class EDOInventoryContextAction : uint8
{
	Use,
	Equip,
	AssignQuickBar1,
	AssignQuickBar2,
	AssignQuickBar3,
	AssignQuickBar4,
	Discard
};

/** Slate 拖拽载荷只携带实例 ID 和源槽位，不携带物品定义指针。 */
class FDOInventoryDragDropOperation : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FDOInventoryDragDropOperation, FDragDropOperation)

	FGuid InstanceId;
	int32 SourceSlotIndex = INDEX_NONE;
	int32 RequestedCount = 0;
	bool bSplitRequested = false;

};

/** 物品格右键菜单，只负责生成菜单按钮，不直接修改背包真实数据。 */
class SDOItemContextMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDOItemContextMenu) {}
		SLATE_ARGUMENT(TSharedPtr<FDOInventorySlotViewModel>, SlotViewModel)
		SLATE_ARGUMENT(TWeakObjectPtr<UDOInventoryViewModel>, ViewModel)
		SLATE_EVENT(FDOOnInventoryContextAction, OnAction)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> BuildActionButton(const FText& Label, EDOInventoryContextAction Action);

	TSharedPtr<FDOInventorySlotViewModel> SlotViewModel;
	TWeakObjectPtr<UDOInventoryViewModel> ViewModel;
	FDOOnInventoryContextAction OnAction;
};

/** 单个物品格，使用固定尺寸保证文字、数量和悬停状态不会改变网格布局。 */
class SDOInventorySlotWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDOInventorySlotWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FDOInventorySlotViewModel>, SlotViewModel)
		SLATE_ARGUMENT(TWeakObjectPtr<UDOInventoryViewModel>, ViewModel)
		SLATE_ARGUMENT(int32, SlotIndex)
		SLATE_EVENT(FDOOnInventorySlotSelected, OnSelected)
		SLATE_EVENT(FDOOnInventorySlotActivated, OnActivated)
		SLATE_EVENT(FDOOnInventorySlotDropped, OnDropped)
		SLATE_EVENT(FDOOnInventoryContextAction, OnContextAction)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

private:
	FSlateColor GetBorderColor() const;
	FText GetNameText() const;
	FText GetStackText() const;
	FText GetTooltipText() const;
	const FSlateBrush* GetIconBrush() const;
	void RequestIconLoad();
	bool IsSelected() const;
	TSharedRef<SWidget> BuildContextMenu();

	TSharedPtr<FDOInventorySlotViewModel> SlotViewModel;
	TWeakObjectPtr<UDOInventoryViewModel> ViewModel;
	int32 SlotIndex = INDEX_NONE;
	FDOOnInventorySlotSelected OnSelected;
	FDOOnInventorySlotActivated OnActivated;
	FDOOnInventorySlotDropped OnDropped;
	FDOOnInventoryContextAction OnContextAction;
	TSharedPtr<SMenuAnchor> ContextMenuAnchor;
	FSoftObjectPath RequestedIconPath;
	TSharedPtr<FSlateBrush> LoadedIconBrush;
};

/** 装备槽拖放目标。装备外观不属于此控件，槽位只显示装备数据并发起装备请求。 */
class SDOEquipmentSlotWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDOEquipmentSlotWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UDOInventoryViewModel>, ViewModel)
		SLATE_ARGUMENT(FGameplayTag, SlotTag)
		SLATE_ARGUMENT(FText, DisplayName)
		SLATE_EVENT(FDOOnEquipmentSlotDropped, OnDropped)
		SLATE_EVENT(FDOOnEquipmentSlotClicked, OnClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

private:
	FSlateColor GetBorderColor() const;
	FText GetSlotText() const;
	FText GetTooltipText() const;

	TWeakObjectPtr<UDOInventoryViewModel> ViewModel;
	FGameplayTag SlotTag;
	FText DisplayName;
	FDOOnEquipmentSlotDropped OnDropped;
	FDOOnEquipmentSlotClicked OnClicked;
};

/** 背包与装备组合页面的 Slate 根面板。 */
class SDOInventoryEquipmentPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDOInventoryEquipmentPanel) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UDOInventoryViewModel>, ViewModel)
		SLATE_EVENT(FDOOnInventoryCloseRequested, OnCloseRequested)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SDOInventoryEquipmentPanel();
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** 将键盘焦点放到物品网格，保证手柄或键盘打开页面后有可操作的初始焦点。 */
	void FocusInitialWidget();

private:
	TSharedRef<SWidget> BuildTopBar();
	TSharedRef<SWidget> BuildCharacterPanel();
	TSharedRef<SWidget> BuildInventoryPanel();
	TSharedRef<SWidget> BuildActionBar();
	TSharedRef<SWidget> BuildEquipmentSlot(const FGameplayTag& SlotTag, const FText& DisplayName);
	TSharedRef<SWidget> BuildCategoryButton(const struct FDOInventoryCategoryOption& Category);

	TSharedRef<ITableRow> GenerateInventoryTile(TSharedPtr<FDOInventorySlotViewModel> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleTileSelectionChanged(TSharedPtr<FDOInventorySlotViewModel> Item, ESelectInfo::Type SelectInfo);
	void HandleViewModelChanged();
	void HandleSlotSelected(const FGuid& InstanceId);
	void HandleSlotActivated(const FGuid& InstanceId);
	void HandleSlotDropped(const FGuid& InstanceId, int32 TargetSlot, bool bSplitRequested);
	void HandleContextAction(const FGuid& InstanceId, uint8 ActionValue);
	void HandleEquipmentSlotDropped(const FGuid& InstanceId, const FGameplayTag& SlotTag);
	FReply HandlePreviousPage();
	FReply HandleNextPage();
	FReply HandleSort();
	FReply HandleDiscard();
	FReply HandleConfirmQuantity();
	FReply HandleCancelQuantity();
	FReply HandleActivateSelected();
	FReply HandleEquipSelected();
	FReply HandleAssignToQuickBar(int32 QuickBarSlot);
	FReply HandleCloseRequested();
	void HandleUnequip(const FGameplayTag& SlotTag);
	void RefreshTileView();

	FText GetPageText() const;
	FText GetCapacityText() const;
	FText GetSelectedNameText() const;
	FText GetSelectedDescriptionText() const;
	FText GetSelectedStackText() const;
	const FDOInventorySlotViewModel* GetSelectedSlot() const;
	FText GetCombatPowerText() const;
	FText GetGuardPowerText() const;
	FText GetAttributeSummaryText() const;
	const FSlateBrush* GetPreviewBrush() const;
	FText GetQuantityDialogTitle() const;
	EVisibility GetQuantityDialogVisibility() const;
	void OpenQuantityDialog(bool bSplit, const FGuid& InstanceId, int32 TargetSlot, int32 MaxCount);

	TWeakObjectPtr<UDOInventoryViewModel> ViewModel;
	FDOOnInventoryCloseRequested OnCloseRequested;
	TSharedPtr<STileView<TSharedPtr<FDOInventorySlotViewModel>>> InventoryTileView;
	TSharedPtr<SHorizontalBox> CategoryBox;
	TSharedPtr<SOverlay> RootOverlay;
	mutable TSharedPtr<FSlateBrush> PreviewBrush;
	FDelegateHandle ViewModelChangedHandle;
	bool bQuantityDialogVisible = false;
	bool bQuantityDialogIsSplit = false;
	FGuid QuantityDialogInstanceId;
	int32 QuantityDialogTargetSlot = INDEX_NONE;
	int32 QuantityDialogMaxCount = 1;
	int32 QuantityDialogCount = 1;
};
