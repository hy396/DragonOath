#pragma once

#include "Characters/DOCharacter.h"
#include "UObject/SoftObjectPtr.h"

#include "DOPlayerCharacter.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputComponent;
class UInputMappingContext;
class ULyraInputConfig;
class ULyraInputComponent;
class UDOInventoryPreviewComponent;
class UDOEquipmentPresentationComponent;
struct FGameplayTag;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FDOInputMappingContextAndPriority
{
	GENERATED_BODY()

	// Enhanced Input 的 MappingContext 资源，负责把键鼠/手柄按键映射到 InputAction。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DO|Input", meta = (AssetBundles = "Client", DisplayName = "输入映射上下文"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// 多个 MappingContext 同时存在时，优先级高的映射会先被处理。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DO|Input", meta = (DisplayName = "映射优先级"))
	int32 Priority = 0;

	// 注册到玩家输入设置后，后续才能支持按键重绑定和本地配置保存。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DO|Input", meta = (DisplayName = "注册到输入设置"))
	bool bRegisterWithSettings = true;
};

/**
 * 玩家控制的 DragonOath 角色。
 *
 * 输入分两层：
 * - NativeInputActions：移动、视角、跳跃、蹲伏等角色本体动作。
 * - AbilityInputActions：只把 GameplayTag 交给 ASC，由 ASC 决定触发哪个技能。
 */
UCLASS(Blueprintable)
class DRAGONOATH_API ADOPlayerCharacter : public ADOCharacter
{
	GENERATED_BODY()

public:
	ADOPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void OnRep_PlayerState() override;
	virtual void UnPossessed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UDOInventoryPreviewComponent* GetInventoryPreviewComponent() const { return InventoryPreviewComponent; }
	UDOEquipmentPresentationComponent* GetEquipmentPresentationComponent() const { return EquipmentPresentationComponent; }

protected:
	// 玩家角色的 ASC 由 PlayerState 持有，避免重生或换 Pawn 时丢失技能和属性。
	virtual void InitializeAbilitySystem() override;

	// 把默认 MappingContext 加到本地玩家的 Enhanced Input 子系统。
	void AddDefaultInputMappings(UEnhancedInputLocalPlayerSubsystem* InputSubsystem);

	// Pawn 失去控制或销毁时移除映射，避免输入绑定残留到下一个 Pawn。
	void RemoveDefaultInputMappings();

	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_StopJumping(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);

protected:
	// Setly/Lyra 风格的输入配置：GameplayTag 到 InputAction 的数据表式映射。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Input", meta = (DisplayName = "输入配置"))
	TObjectPtr<ULyraInputConfig> InputConfig;

	// 角色默认启用的 Enhanced Input 映射，例如通用移动、战斗、UI 模式等。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Input", meta = (DisplayName = "默认输入映射"))
	TArray<FDOInputMappingContextAndPriority> DefaultInputMappings;

	// 开启后会先清空所有 MappingContext。只有在确认没有 UI/载具等其它系统映射时再使用。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Input", meta = (DisplayName = "添加前清空输入映射"))
	bool bClearInputMappingsBeforeAdd = false;

	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> CachedInputSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<ULyraInputComponent> CachedLyraInputComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DO|UI", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDOInventoryPreviewComponent> InventoryPreviewComponent;

	/** Pawn 级公开装备外观摘要，供远端客户端表现系统读取。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DO|Equipment", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDOEquipmentPresentationComponent> EquipmentPresentationComponent;

	TArray<uint32> AbilityInputBindHandles;
	bool bDefaultInputMappingsAdded = false;
	bool bInputBindingsAdded = false;

};
