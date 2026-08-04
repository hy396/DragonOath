#include "Player/DOPlayerState.h"

#include "AbilitySystem/Abilities/Core/DOAbilitySet.h"
#include "AbilitySystem/Abilities/Core/DOProfessionAbilityConfig.h"
#include "AbilitySystem/Attributes/DOHealthSet.h"
#include "AbilitySystem/Attributes/DOResourceSet.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Components/DOHealthComponent.h"
#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "DOLogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SaveGame/DOSaveGame.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOPlayerState)

ADOPlayerState::ADOPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UDOAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthSet = CreateDefaultSubobject<UDOHealthSet>(TEXT("HealthSet"));
	ResourceSet = CreateDefaultSubobject<UDOResourceSet>(TEXT("ResourceSet"));
	CombatSet = CreateDefaultSubobject<UDOCombatSet>(TEXT("CombatSet"));

	// 玩家死亡行为组件挂在 PlayerState（玩家 ASC 在 PlayerState，与 HealthSet 同生命周期）。
	// 玩家 Character 上的同名 HealthComponent 是冗余实例，由 ADOCharacter 兜底跳过注入。
	HealthComponent = CreateDefaultSubobject<UDOHealthComponent>(TEXT("HealthComponent"));
	InventoryComponent = CreateDefaultSubobject<UDOInventoryComponent>(TEXT("InventoryComponent"));
	EquipmentComponent = CreateDefaultSubobject<UDOEquipmentComponent>(TEXT("EquipmentComponent"));
	ItemQuickBarComponent = CreateDefaultSubobject<UDOItemQuickBarComponent>(TEXT("ItemQuickBarComponent"));
}

void ADOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADOPlayerState, ProfessionTag);
}

void ADOPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void ADOPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && bAutoSaveInventory)
	{
		SaveInventoryToSlot();
	}

	Super::EndPlay(EndPlayReason);
}

FString ADOPlayerState::GetInventorySaveSlotName() const
{
	const FString Prefix = InventorySaveSlotPrefix.IsEmpty() ? TEXT("DragonOath_Player") : InventorySaveSlotPrefix;
	return FString::Printf(TEXT("%s_%d"), *Prefix, FMath::Max(0, GetPlayerId()));
}

bool ADOPlayerState::SaveInventoryToSlot()
{
	if (!HasAuthority())
	{
		return false;
	}

	UDOSaveGame* SaveGame = UDOSaveGame::CaptureFromPlayerState(this);
	if (!SaveGame)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("SaveInventoryToSlot: 无法从 PlayerState 创建存档对象。"));
		return false;
	}

	const FString SlotName = GetInventorySaveSlotName();
	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	UE_LOG(LogDragonOath, Log, TEXT("SaveInventoryToSlot: Slot=%s Result=%s"), *SlotName, bSaved ? TEXT("Success") : TEXT("Failed"));
	return bSaved;
}

bool ADOPlayerState::LoadInventoryFromSlot()
{
	if (!HasAuthority() || !bInventoryPersistenceReady)
	{
		return false;
	}
	bInventoryLoadAttempted = true;

	const FString SlotName = GetInventorySaveSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogDragonOath, Verbose, TEXT("LoadInventoryFromSlot: Slot=%s 不存在，使用默认状态。"), *SlotName);
		return false;
	}

	UDOSaveGame* SaveGame = Cast<UDOSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveGame)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("LoadInventoryFromSlot: Slot=%s 加载失败或类型不匹配。"), *SlotName);
		return false;
	}

	const bool bRestored = SaveGame->RestoreToPlayerState(this);
	UE_LOG(LogDragonOath, Log, TEXT("LoadInventoryFromSlot: Slot=%s Result=%s"), *SlotName, bRestored ? TEXT("Success") : TEXT("Rejected"));
	return bRestored;
}

void ADOPlayerState::NotifyInventoryPersistenceReady()
{
	if (bInventoryPersistenceReady)
	{
		return;
	}

	bInventoryPersistenceReady = true;
	if (HasAuthority() && bAutoLoadInventory && !bInventoryLoadAttempted)
	{
		bInventoryLoadAttempted = true;
		LoadInventoryFromSlot();
	}
}

UAbilitySystemComponent* ADOPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDOAbilitySystemComponent* ADOPlayerState::GetDOAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDOHealthSet* ADOPlayerState::GetHealthSet() const
{
	return HealthSet;
}

UDOResourceSet* ADOPlayerState::GetResourceSet() const
{
	return ResourceSet;
}

UDOCombatSet* ADOPlayerState::GetCombatSet() const
{
	return CombatSet;
}

void ADOPlayerState::SetProfession(FGameplayTag NewProfession)
{
	if (!HasAuthority())
	{
		return;
	}

	if (ProfessionTag == NewProfession)
	{
		return;
	}

	// 清除旧职业技能
	if (bProfessionAbilitiesGranted)
	{
		AbilitySystemComponent->ClearDOAbilities();
		bProfessionAbilitiesGranted = false;
	}

	ProfessionTag = NewProfession;

	// 授予新职业技能
	GrantProfessionAbilities();
}

void ADOPlayerState::GrantProfessionAbilities()
{
	if (bProfessionAbilitiesGranted)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (!ProfessionTag.IsValid())
	{
		UE_LOG(LogDragonOath, Warning, TEXT("GrantProfessionAbilities: ProfessionTag is not set."));
		return;
	}

	if (!ProfessionAbilityConfig)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("GrantProfessionAbilities: ProfessionAbilityConfig is not set."));
		return;
	}

	const TObjectPtr<UDOAbilitySet>* AbilitySetPtr = ProfessionAbilityConfig->ProfessionAbilitySets.Find(ProfessionTag);
	if (!AbilitySetPtr || !*AbilitySetPtr)
	{
		UE_LOG(LogDragonOath, Warning, TEXT("GrantProfessionAbilities: No AbilitySet found for profession %s."), *ProfessionTag.ToString());
		return;
	}

	AbilitySystemComponent->GiveDOAbilitySet(*AbilitySetPtr);
	bProfessionAbilitiesGranted = true;

	UE_LOG(LogDragonOath, Log, TEXT("Granted profession abilities for %s."), *ProfessionTag.ToString());
}

void ADOPlayerState::EnsureProfessionSet()
{
	if (!HasAuthority())
	{
		// 客户端走的 InitializeAbilitySystem 也会进到这里，
		// 但能力由服务器经 Mixed 复制下发，不在此处授予。
		return;
	}

	if (!ProfessionTag.IsValid())
	{
		// 职业未设定：用兜底来源补上并授予技能
		const FGameplayTag Def = DefaultProfessionTag.IsValid()
			? DefaultProfessionTag
			: DragonOathGameplayTags::Profession::DragonFighter;
		SetProfession(Def);
	}
	else
	{
		// 职业已在（如重生存场景 ProfessionTag 随 PlayerState 保留），
		// 仅保证技能已授予，避免重复。
		GrantProfessionAbilities();
	}
}

void ADOPlayerState::OnRep_ProfessionTag()
{
	// 客户端可以在这里更新 UI、切换角色外观等
	// 后续可以委托给 UI 系统或 GameplayMessageRouter
}
