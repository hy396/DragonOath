#include "Player/DOPlayerController.h"

#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "ItemSystem/Pickup/DOItemPickup.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "ItemSystem/QuickBar/DOItemQuickBarViewModel.h"
#include "CommonUIExtensions.h"
#include "Engine/GameViewportClient.h"
#include "UI/Inventory/DOItemQuickBarSlateWidgets.h"
#include "PrimaryGameLayout.h"
#include "Player/DOPlayerState.h"
#include "UI/Inventory/DOInventoryScreen.h"
#include "Widgets/SOverlay.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlayerController)

ADOPlayerController::ADOPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ADOPlayerController::BeginPlay()
{
	Super::BeginPlay();
	EnsureQuickBarHud();
}

void ADOPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (QuickBarViewportWidget.IsValid() && GetWorld() && GetWorld()->GetGameViewport())
	{
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(QuickBarViewportWidget.ToSharedRef());
	}
	QuickBarViewportWidget.Reset();
	QuickBarWidget.Reset();
	if (QuickBarViewModel)
	{
		QuickBarViewModel->Shutdown();
		QuickBarViewModel = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ADOPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	EnsureQuickBarHud();
}

void ADOPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// Enhanced Input 会先把本帧输入事件派发给 Pawn。
	// 这里再统一处理 ASC 缓存的技能输入，顺序与 Lyra 保持一致。
	if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
	{
		DOASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

bool ADOPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (IsLocalController() && Params.Event == IE_Pressed)
	{
		if (Params.Key == EKeys::I || Params.Key == EKeys::B)
		{
			ToggleInventoryScreen();
			return true;
		}

		// 背包页面打开时不响应快捷栏数字键，避免菜单操作误触发消耗品。
		if (!ActiveInventoryScreen || !ActiveInventoryScreen->IsActivated())
		{
			int32 QuickBarSlot = INDEX_NONE;
			if (Params.Key == EKeys::One) QuickBarSlot = 0;
			else if (Params.Key == EKeys::Two) QuickBarSlot = 1;
			else if (Params.Key == EKeys::Three) QuickBarSlot = 2;
			else if (Params.Key == EKeys::Four) QuickBarSlot = 3;

			if (QuickBarSlot != INDEX_NONE)
			{
				if (ADOPlayerState* DOPlayerState = GetPlayerState<ADOPlayerState>())
				{
					if (UDOItemQuickBarComponent* QuickBar = DOPlayerState->GetItemQuickBarComponent())
					{
						QuickBar->RequestUseSlot(QuickBarSlot, 0);
						return true;
					}
				}
			}
		}
	}

	return Super::InputKey(Params);
}

void ADOPlayerController::ToggleInventoryScreen()
{
	if (!IsLocalController())
	{
		return;
	}

	if (ActiveInventoryScreen && ActiveInventoryScreen->IsActivated())
	{
		// 从 CommonUI 层移除页面，避免反复按 I/B 时在 Menu 层累积失活实例。
		UCommonUIExtensions::PopContentFromLayer(ActiveInventoryScreen);
		ActiveInventoryScreen = nullptr;
		return;
	}
	ActiveInventoryScreen = nullptr;

	if (UPrimaryGameLayout* PrimaryLayout = UPrimaryGameLayout::GetPrimaryGameLayout(this))
	{
		UClass* WidgetClass = InventoryScreenClass ? InventoryScreenClass.Get() : UDOInventoryScreen::StaticClass();
		ActiveInventoryScreen = PrimaryLayout->PushWidgetToLayerStack<UDOInventoryScreen>(
			DragonOathGameplayTags::Message::UI::Layer::Menu,
			WidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ToggleInventoryScreen: PrimaryGameLayout 尚未创建，无法打开背包页面。"));
	}
}

void ADOPlayerController::DOAddInventoryItem(const FString& DefinitionName, const int32 Count)
{
	if (!HasAuthority() || DefinitionName.IsEmpty() || Count <= 0)
	{
		return;
	}

	ADOPlayerState* DOPlayerState = GetPlayerState<ADOPlayerState>();
	UDOInventoryComponent* Inventory = DOPlayerState ? DOPlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		return;
	}

	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("ItemDefinition")), FName(*DefinitionName));
	const FDOInventoryAddResult Result = Inventory->TryAddItem(DefinitionId, Count);
	UE_LOG(LogTemp, Log, TEXT("DOAddInventoryItem: Definition=%s Requested=%d Added=%d Remaining=%d Failure=%d"),
		*DefinitionName,
		Count,
		Result.AddedCount,
		Result.RemainingCount,
		static_cast<int32>(Result.FailureReason));
}

void ADOPlayerController::RequestPickupItem(ADOItemPickup* Pickup)
{
	if (!Pickup)
	{
		return;
	}

	if (HasAuthority())
	{
		Server_RequestPickup_Implementation(Pickup);
	}
	else
	{
		Server_RequestPickup(Pickup);
	}
}

void ADOPlayerController::Server_RequestPickup_Implementation(ADOItemPickup* Pickup)
{
	if (Pickup && GetPawn())
	{
		// Pickup 内部再次校验服务器权限、距离、物品定义和背包空间。
		Pickup->TryPickupForActor(GetPawn());
	}
}

void ADOPlayerController::EnsureQuickBarHud()
{
	if (!IsLocalController() || QuickBarWidget.IsValid())
	{
		return;
	}

	ADOPlayerState* DOPlayerState = GetPlayerState<ADOPlayerState>();
	if (!DOPlayerState || !GetWorld() || !GetWorld()->GetGameViewport())
	{
		return;
	}

	QuickBarViewModel = NewObject<UDOItemQuickBarViewModel>(this);
	QuickBarViewModel->Initialize(DOPlayerState);
	SAssignNew(QuickBarWidget, SDOItemQuickBarWidget)
		.ViewModel(QuickBarViewModel);

	SAssignNew(QuickBarViewportWidget, SOverlay)
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(0.0f, 0.0f, 0.0f, 28.0f)
	[
		QuickBarWidget.ToSharedRef()
	];

	GetWorld()->GetGameViewport()->AddViewportWidgetContent(QuickBarViewportWidget.ToSharedRef(), 20);
}

UDOAbilitySystemComponent* ADOPlayerController::GetDOAbilitySystemComponent() const
{
	return UDOAbilitySystemComponent::GetFromActor(GetPawn());
}
