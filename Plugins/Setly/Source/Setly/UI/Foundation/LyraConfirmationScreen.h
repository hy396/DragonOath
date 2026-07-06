// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "Messaging/CommonGameDialog.h"

#include "Messaging/CommonMessagingSubsystem.h"
#include "LyraConfirmationScreen.generated.h"

class IWidgetCompilerLog;

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;
class ULyraButtonBase;

/**
 *	
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ULyraConfirmationScreen : public UCommonGameDialog
{
	GENERATED_BODY()
public:
	virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback) override;
	virtual void KillDialog() override;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeOnInitialized() override;
	virtual void CloseConfirmationWindow(ECommonMessagingResult Result);

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

private:

	UFUNCTION()
	FEventReply HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	void ScheduleBindPendingButtonInputActions();
	void BindPendingButtonInputActions();

	FCommonMessagingResultDelegate OnResultCallback;

private:
	struct FPendingButtonInputAction
	{
		TWeakObjectPtr<ULyraButtonBase> Button;
		FDataTableRowHandle InputAction;
	};

	TArray<FPendingButtonInputAction> PendingButtonInputActions;
	int32 ButtonInputBindingRetryCount = 0;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Title;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonBorder> Border_TapToCloseZone;

	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle CancelAction;
};
