#include "Player/DOPlayerCharacter.h"

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "DOLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "LyraInputComponent.h"
#include "Player/DOPlayerController.h"
#include "Player/DOPlayerState.h"
#include "SetlyGameplayTags.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlayerCharacter)

// namespace DragonOathInputTags
// {
// 	static const float LookYawRate = 300.0f;
// 	static const float LookPitchRate = 165.0f;
// }

ADOPlayerCharacter::ADOPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 角色可蹲伏，我好像并不需要这个东西
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// 角色旋转完全独立，不跟随摄像机（控制器）
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 关闭引擎自带的“自动朝向移动方向”，我们自己控制转身逻辑
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ADOPlayerCharacter::InitializeAbilitySystem()
{
	ADOPlayerState* DOPlayerState = GetPlayerState<ADOPlayerState>();
	if (!DOPlayerState)
	{
		return;
	}

	InitializeAbilitySystemComponent(DOPlayerState->GetDOAbilitySystemComponent(), DOPlayerState);

	// ASC 初始化后，服务端授予职业技能
	DOPlayerState->GrantProfessionAbilities();
}

void ADOPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

void ADOPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!InputConfig)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("%s has no InputConfig. Movement and abilities will not be bound."), *GetNameSafe(this));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetInputSubsystem();
	if (!InputSubsystem)
	{
		return;
	}

	ULyraInputComponent* LyraIC = Cast<ULyraInputComponent>(PlayerInputComponent);
	if (!LyraIC)
	{
		UE_LOG(LogDragonOath, Error, TEXT("%s expected ULyraInputComponent, got %s. Set DefaultInputComponentClass to /Script/Setly.LyraInputComponent."),
			*GetNameSafe(this), *GetNameSafe(PlayerInputComponent));
		return;
	}

	AddDefaultInputMappings(InputSubsystem);

	if (bInputBindingsAdded)
	{
		return;
	}

	// ULyraInputComponent 用 GameplayTag 找 InputAction，避免 C++ 直接依赖具体按键资源。
	LyraIC->AddInputMappings(InputConfig, InputSubsystem);
	LyraIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, AbilityInputBindHandles);

	// NativeInputActions 处理角色本身的输入；AbilityInputActions 只透传给 ASC。
	LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
	// 新增 Started 绑定（检测双击冲刺）
	LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Move, ETriggerEvent::Started, this, &ThisClass::Input_MoveStarted, /*bLogIfNotFound=*/ false);
	// LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
	// LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
	LyraIC->BindNativeAction(InputConfig, DragonOathGameplayTags::InputTag::Jump, ETriggerEvent::Started, this, &ThisClass::Input_Jump, /*bLogIfNotFound=*/ false);
	LyraIC->BindNativeAction(InputConfig, DragonOathGameplayTags::InputTag::Jump, ETriggerEvent::Completed, this, &ThisClass::Input_StopJumping, /*bLogIfNotFound=*/ false);
	LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
	CachedLyraInputComponent = LyraIC;
	bInputBindingsAdded = true;
}

void ADOPlayerCharacter::UnPossessed()
{
	RemoveDefaultInputMappings();
	Super::UnPossessed();
}

void ADOPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveDefaultInputMappings();
	Super::EndPlay(EndPlayReason);
}

void ADOPlayerCharacter::AddDefaultInputMappings(UEnhancedInputLocalPlayerSubsystem* InputSubsystem)
{
	if (!InputSubsystem || bDefaultInputMappingsAdded)
	{
		return;
	}

	if (bClearInputMappingsBeforeAdd)
	{
		// 只建议在玩家 Pawn 初始化时清空；如果 UI 或其他系统也在加 MappingContext，需要谨慎开启。
		InputSubsystem->ClearAllMappings();
	}

	for (const FDOInputMappingContextAndPriority& Mapping : DefaultInputMappings)
	{
		UInputMappingContext* MappingContext = Mapping.InputMapping.LoadSynchronous();
		if (!MappingContext)
		{
			continue;
		}

		if (Mapping.bRegisterWithSettings)
		{
			if (UEnhancedInputUserSettings* Settings = InputSubsystem->GetUserSettings())
			{
				Settings->RegisterInputMappingContext(MappingContext);
			}
		}

		FModifyContextOptions Options;
		Options.bIgnoreAllPressedKeysUntilRelease = false;
		InputSubsystem->AddMappingContext(MappingContext, Mapping.Priority, Options);
	}

	CachedInputSubsystem = InputSubsystem;
	bDefaultInputMappingsAdded = true;
}

void ADOPlayerCharacter::RemoveDefaultInputMappings()
{
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = CachedInputSubsystem;
	if (!InputSubsystem || !bDefaultInputMappingsAdded)
	{
		return;
	}

	for (const FDOInputMappingContextAndPriority& Mapping : DefaultInputMappings)
	{
		if (const UInputMappingContext* MappingContext = Mapping.InputMapping.Get())
		{
			InputSubsystem->RemoveMappingContext(MappingContext);
		}
	}

	if (CachedLyraInputComponent)
	{
		CachedLyraInputComponent->RemoveBinds(AbilityInputBindHandles);
	}

	CachedInputSubsystem = nullptr;
	CachedLyraInputComponent = nullptr;
	bDefaultInputMappingsAdded = false;
	bInputBindingsAdded = false;
}

UEnhancedInputLocalPlayerSubsystem* ADOPlayerCharacter::GetInputSubsystem() const
{
	const ADOPlayerController* DOController = Cast<ADOPlayerController>(GetController());
	if (!DOController)
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = DOController->GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
}

void ADOPlayerCharacter::Input_AbilityInputTagPressed(const FGameplayTag InputTag)
{
	// 角色只负责把输入标签交给 ASC，不在这里判断具体释放哪个技能。
	if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
	{
		DOASC->AbilityInputTagPressed(InputTag);
	}
}

void ADOPlayerCharacter::Input_AbilityInputTagReleased(const FGameplayTag InputTag)
{
	if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
	{
		DOASC->AbilityInputTagReleased(InputTag);
	}
}

void ADOPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	// W/S：沿世界纵深方向（X轴）移动，永远不改变角色朝向
	if (FMath::Abs(Value.Y) > KINDA_SMALL_NUMBER)
	{
		AddMovementInput(FVector::RightVector, Value.Y);
	}

	// A/D：沿世界左右方向（Y轴）移动 + 角色转向移动方向
	if (FMath::Abs(Value.X) > KINDA_SMALL_NUMBER)
	{
		AddMovementInput(FVector::ForwardVector, Value.X);

		const float TargetYaw = Value.X > 0.0f ? 0.0f : 180.0f;
		const float CurrentYaw = GetActorRotation().Yaw;
		// 核心优化1：计算最小角度差，自动处理-180/180°环绕问题
		const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw));
		
		// 核心优化2：只有角度差超过阈值时，才真正调用SetActorRotation
		if (YawDelta > 0.1f)
		{
			SetActorRotation(FRotator(0.0f, TargetYaw, 0.0f));
		}
		const FRotator TargetRot(0.0f, TargetYaw, 0.0f);
		// 平滑转身；想要原版瞬转翻面效果，直接删掉插值，改为 SetActorRotation(TargetRot)
		SetActorRotation(TargetRot);

	}
}

void ADOPlayerCharacter::Input_Jump(const FInputActionValue& /*InputActionValue*/)
{
	Jump();
}

void ADOPlayerCharacter::Input_StopJumping(const FInputActionValue& /*InputActionValue*/)
{
	StopJumping();
}

void ADOPlayerCharacter::Input_Crouch(const FInputActionValue& /*InputActionValue*/)
{
	if (bIsCrouched || GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (GetCharacterMovement()->IsMovingOnGround())
	{
		Crouch();
	}
}

void ADOPlayerCharacter::Input_MoveStarted(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	// 只检测横向输入（A/D），忽略纯纵向（W/S）
	if (FMath::Abs(Value.X) < KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float CurrentDirection = FMath::Sign(Value.X);
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();

	// 同方向连续两次按下，且间隔在阈值内
	if (CurrentDirection == LastMovePressDirection
		&& LastMovePressTime > 0.0f
		&& (CurrentTime - LastMovePressTime) <= DoubleTapThreshold)
	{
		// 触发冲刺（方向由技能内部通过角色朝向获取）
		TryStartDash();

		// 重置检测状态，避免三连击重复触发
		LastMovePressTime = -1.0f;
		LastMovePressDirection = 0.0f;
	}
	else
	{
		LastMovePressTime = CurrentTime;
		LastMovePressDirection = CurrentDirection;
	}
}

void ADOPlayerCharacter::TryStartDash()
{
	UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent();
	if (!DOASC)
	{
		return;
	}

	// 通过 Tag 查找冲刺技能并激活
	// 冲刺技能的 AbilityTags 中应包含 Ability.Movement.Dash
	FGameplayTagContainer DashTags;
	DashTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Movement.Dash")));

	DOASC->TryActivateAbilitiesByTag(DashTags);
}
