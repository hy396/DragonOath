#pragma once

#include "GameFramework/PlayerController.h"
#include "SetlyPlayerController.h"
#include "DOPlayerController.generated.h"

class UDOAbilitySystemComponent;
class UDOInventoryScreen;
class UDOItemQuickBarViewModel;
class ADOItemPickup;
class SDOItemQuickBarWidget;
class SOverlay;

/**
 * 玩家控制器。
 *
 * Enhanced Input 的事件先在 Pawn 上收集，本类在每帧输入后处理 ASC 缓存。
 * 这样 Pressed、Held、Released 会按统一顺序进入技能系统，便于做预测和输入复制。
 */
UCLASS()
class DRAGONOATH_API ADOPlayerController : public ASetlyPlayerController
{
	GENERATED_BODY()

public:
	ADOPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostProcessInput(float DeltaTime, bool bGamePaused) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	/** 在没有依赖额外输入资产的情况下提供 I/B 背包快捷键。 */
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

	/** 通过 CommonUI Menu 层打开或关闭原生 Slate 背包页面。 */
	UFUNCTION(BlueprintCallable, Category = "DO|UI")
	void ToggleInventoryScreen();

	/** 编辑器/开发联机测试用：由服务器给当前玩家添加一个 ItemDefinition。 */
	UFUNCTION(Exec)
	void DOAddInventoryItem(const FString& DefinitionName, int32 Count = 1);

	/** 请求服务器让当前 Pawn 拾取指定掉落物，服务器不信任客户端传入的控制器或 Pawn。 */
	UFUNCTION(BlueprintCallable, Category = "DO|Inventory|Pickup")
	void RequestPickupItem(ADOItemPickup* Pickup);

	protected:
	UDOAbilitySystemComponent* GetDOAbilitySystemComponent() const;
	void EnsureQuickBarHud();

	UFUNCTION(Server, Reliable)
	void Server_RequestPickup(ADOItemPickup* Pickup);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|UI", meta = (DisplayName = "背包界面类"))
	TSubclassOf<UDOInventoryScreen> InventoryScreenClass;

	UPROPERTY(Transient)
	TObjectPtr<UDOInventoryScreen> ActiveInventoryScreen;

	UPROPERTY(Transient)
	TObjectPtr<UDOItemQuickBarViewModel> QuickBarViewModel;

	TSharedPtr<SDOItemQuickBarWidget> QuickBarWidget;
	TSharedPtr<SOverlay> QuickBarViewportWidget;
};
