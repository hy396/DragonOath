#include "ItemSystem/Inventory/DOInventoryComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "AbilitySystem/Abilities/Core/DOAbilitySet.h"
#include "AbilitySystem/Attributes/DOCombatSet.h"
#include "AbilitySystem/Core/DOAbilitySystemComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/Equipment/DOEquipmentInstance.h"
#include "ItemSystem/AbilitySystem/DOItemGameplayEffects.h"
#include "ItemSystem/AbilitySystem/DOItemEffectSpecBuilder.h"
#include "ItemSystem/Core/DOItemDefinition.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Misc/DataValidation.h"
#include "ItemSystem/QuickBar/DOItemQuickBarComponent.h"
#include "ItemSystem/Usage/DOItemUseEffects.h"
#include "ItemSystem/Inventory/DOInventoryMutation.h"
#include "ItemSystem/Inventory/DOInventorySortConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DOPlayerState.h"
#include "SaveGame/DOSaveGame.h"
#include "AssetRegistry/AssetBundleData.h"
#include "UI/Inventory/DOCombatRatingConfig.h"
#include "ItemSystem/Tests/DOTestGameplayAbility.h"
#include "GameplayEffect/GE_SharedCooling.h"

namespace
{
	/** 为需要真实 World/组件注册状态的 GAS 测试创建临时游戏世界。 */
	struct FDOAutomationWorld
	{
		UWorld* World = nullptr;

		FDOAutomationWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (!World || !GEngine)
			{
				return;
			}

			FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			WorldContext.SetCurrentWorld(World);
			FURL URL;
			World->InitializeActorsForPlay(URL);
			World->BeginPlay();
		}

		~FDOAutomationWorld()
		{
			if (!World)
			{
				return;
			}

			World->EndPlay(EEndPlayReason::Quit);
			if (GEngine)
			{
				GEngine->DestroyWorldContext(World);
			}
			World->DestroyWorld(false);
		}

		ADOPlayerState* SpawnPlayerState() const
		{
			if (!World)
			{
				return nullptr;
			}

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			return World->SpawnActor<ADOPlayerState>(ADOPlayerState::StaticClass(), FTransform::Identity, SpawnParameters);
		}
	};

	/** 为自动化测试注册一个瞬态 ItemDefinition，避免依赖 Content 里的具体资产。 */
	UDOItemDefinition* CreateTestDefinition(const TCHAR* Name, const int32 MaxStackSize)
	{
		UObject* Outer = GetTransientPackage();
		const FName ObjectName = MakeUniqueObjectName(Outer, UDOItemDefinition::StaticClass(), FName(Name));
		UDOItemDefinition* Definition = NewObject<UDOItemDefinition>(Outer, ObjectName);
		Definition->DisplayName = FText::FromString(Name);
		Definition->MaxStackSize = MaxStackSize;

		// 项目中的 ItemDefinition 类型来自磁盘扫描，测试不能把它再次注册为 DynamicAsset。
		// 使用独立类型只影响测试注册，不改变运行时 ItemDefinition 的正式类型。
		const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), ObjectName);
		FAssetBundleData BundleData;
		BundleData.AddBundleAssetTruncated(TEXT("Test"), FSoftObjectPath(Definition));
		UAssetManager::Get().AddDynamicAsset(DefinitionId, FSoftObjectPath(Definition), BundleData);
		return Definition;
	}

	UDOInventoryComponent* CreateTestInventory(APlayerState*& OutOwner, const int32 Capacity)
	{
		OutOwner = NewObject<APlayerState>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), APlayerState::StaticClass(), TEXT("DOInventoryTestOwner")));
		UDOInventoryComponent* Inventory = NewObject<UDOInventoryComponent>(OutOwner);
		const TArray<FDOItemInstanceRecord> EmptyItems;
		Inventory->RestoreInventorySnapshot(EmptyItems, Capacity);
		return Inventory;
	}

	FDOItemInstanceRecord MakeTestItem(const FPrimaryAssetId& DefinitionId, const int32 SlotIndex, const int32 StackCount)
	{
		FDOItemInstanceRecord Item;
		Item.InstanceId = FGuid::NewGuid();
		Item.DefinitionId = DefinitionId;
		Item.SlotIndex = SlotIndex;
		Item.StackCount = StackCount;
		return Item;
	}

	UDOItemDefinition* CreateTestEquipmentDefinition(const TCHAR* Name, const FGameplayTag& EquipmentSlotTag)
	{
		UDOItemDefinition* Definition = CreateTestDefinition(Name, 1);
		Definition->ItemType = DragonOathGameplayTags::Item::Type::Equipment;

		UDOItemFragment_Equipment* EquipmentFragment = NewObject<UDOItemFragment_Equipment>(Definition);
		EquipmentFragment->EquipmentSlotTag = EquipmentSlotTag;
		EquipmentFragment->AttributeModifiers.AttackPower = 10.0f;
		Definition->Fragments.Add(EquipmentFragment);
		return Definition;
	}

	UDOItemDefinition* CreateTestConsumableDefinition(const TCHAR* Name)
	{
		UDOItemDefinition* Definition = CreateTestDefinition(Name, 20);
		Definition->ItemType = DragonOathGameplayTags::Item::Type::Consumable;

		UDOItemFragment_Consumable* ConsumableFragment = NewObject<UDOItemFragment_Consumable>(Definition);
		ConsumableFragment->UseGameplayEffect = UDOItemHealthPotionEffect::StaticClass();
		Definition->Fragments.Add(ConsumableFragment);
		return Definition;
	}

	void InitializeTestAbilitySystem(ADOPlayerState* PlayerState)
	{
		if (PlayerState)
		{
			UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent();
			// 测试对象没有经过 Pawn Possess 流程，这里显式提供最小 ActorInfo，
			// 让即时 GameplayEffect 和装备属性 GE 可以走真实 GAS 路径。
			if (ASC && PlayerState->GetWorld())
			{
				// 测试世界中的 PlayerState 可能尚未自动注册其默认子组件，先显式补齐注册。
				if (!ASC->IsRegistered())
				{
					ASC->RegisterComponentWithWorld(PlayerState->GetWorld());
				}

				// UE 5.8 的 InitAbilityActorInfo 要求 ASC 先拥有有效的
				// AbilityActorInfo。测试世界不一定会自动调用 InitializeComponent，
				// 所以这里补齐与正常 Actor 生命周期等价的初始化步骤。
				if (!ASC->HasBeenInitialized())
				{
					ASC->InitializeComponent();
				}
				ASC->InitAbilityActorInfo(PlayerState, PlayerState);
			}
		}
	}

	void AddInventoryRules(UDOItemDefinition* Definition, const bool bUnique)
	{
		if (!Definition)
		{
			return;
		}

		UDOItemFragment_Inventory* Fragment = NewObject<UDOItemFragment_Inventory>(Definition);
		Fragment->bUnique = bUnique;
		Definition->Fragments.Add(Fragment);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOItemSaveArchiveRoundTripTest,
	"DragonOath.Inventory.SaveGameArchiveRoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOItemSaveArchiveRoundTripTest::RunTest(const FString& /*Parameters*/)
{
	UDOSaveGame* SourceSave = NewObject<UDOSaveGame>();
	SourceSave->InventoryCapacity = 72;

	FDOItemInstanceRecord SourceItem;
	SourceItem.InstanceId = FGuid::NewGuid();
	SourceItem.DefinitionId = FPrimaryAssetId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), TEXT("DO_Test_SaveArchive"));
	SourceItem.StackCount = 9;
	SourceItem.SlotIndex = 17;
	SourceItem.UpgradeLevel = 4;
	SourceItem.CurrentDurability = 63;
	FDOItemAffixRoll& SourceAffix = SourceItem.Affixes.AddDefaulted_GetRef();
	SourceAffix.AffixTag = DragonOathGameplayTags::Data::Equipment::AttackPower;
	SourceAffix.Magnitude = 12.5f;
	SourceSave->InventoryItems.Add(SourceItem);

	TArray<uint8> SaveBytes;
	if (!TestTrue(TEXT("SaveGame 可以序列化到内存"), UGameplayStatics::SaveGameToMemory(SourceSave, SaveBytes)))
	{
		return false;
	}

	UDOSaveGame* LoadedSave = Cast<UDOSaveGame>(UGameplayStatics::LoadGameFromMemory(SaveBytes));
	if (!TestNotNull(TEXT("SaveGame 可以从内存反序列化"), LoadedSave)
		|| !TestEqual(TEXT("反序列化后保留物品数量"), LoadedSave->InventoryItems.Num(), 1))
	{
		return false;
	}

	const FDOItemInstanceRecord& LoadedItem = LoadedSave->InventoryItems[0];
	TestEqual(TEXT("保留 InstanceId"), LoadedItem.InstanceId, SourceItem.InstanceId);
	TestEqual(TEXT("保留 DefinitionId"), LoadedItem.DefinitionId, SourceItem.DefinitionId);
	TestEqual(TEXT("保留 StackCount"), LoadedItem.StackCount, SourceItem.StackCount);
	TestEqual(TEXT("保留 SlotIndex"), LoadedItem.SlotIndex, SourceItem.SlotIndex);
	TestEqual(TEXT("保留 UpgradeLevel"), LoadedItem.UpgradeLevel, SourceItem.UpgradeLevel);
	TestEqual(TEXT("保留 CurrentDurability"), LoadedItem.CurrentDurability, SourceItem.CurrentDurability);
	if (TestEqual(TEXT("保留词缀数量"), LoadedItem.Affixes.Num(), 1))
	{
		TestEqual(TEXT("保留 AffixTag"), LoadedItem.Affixes[0].AffixTag, SourceAffix.AffixTag);
		TestEqual(TEXT("保留词缀 Magnitude"), LoadedItem.Affixes[0].Magnitude, SourceAffix.Magnitude);
	}

	const FString SlotName = FString::Printf(TEXT("DO_Automation_ItemArchive_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
	TestTrue(TEXT("SaveGame 可以写入隔离测试槽位"), UGameplayStatics::SaveGameToSlot(SourceSave, SlotName, 0));
	UDOSaveGame* SlotLoadedSave = Cast<UDOSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (TestNotNull(TEXT("SaveGame 可以从隔离测试槽位读取"), SlotLoadedSave) && SlotLoadedSave->InventoryItems.Num() == 1)
	{
		TestEqual(TEXT("真实槽位往返保留 InstanceId"), SlotLoadedSave->InventoryItems[0].InstanceId, SourceItem.InstanceId);
		TestEqual(TEXT("真实槽位往返保留词缀"), SlotLoadedSave->InventoryItems[0].Affixes.Num(), 1);
	}
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		TestTrue(TEXT("隔离测试槽位已清理"), UGameplayStatics::DeleteGameInSlot(SlotName, 0));
	}

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryStackAndCapacityTest,
	"DragonOath.Inventory.StackAndCapacity",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryStackAndCapacityTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* Definition = CreateTestDefinition(TEXT("DO_Test_Stackable"), 5);
	APlayerState* Owner = nullptr;
	UDOInventoryComponent* Inventory = CreateTestInventory(Owner, 2);
	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), Definition->GetFName());

	const FDOInventoryAddResult FirstAdd = Inventory->TryAddItem(DefinitionId, 7);
	TestEqual(TEXT("第一次添加数量正确"), FirstAdd.AddedCount, 7);
	TestEqual(TEXT("第一次添加没有剩余"), FirstAdd.RemainingCount, 0);
	TestEqual(TEXT("第一次添加生成两个堆栈"), Inventory->GetUsedSlotCount(), 2);

	const FDOInventoryAddResult SecondAdd = Inventory->TryAddItem(DefinitionId, 4);
	TestEqual(TEXT("第二次添加只填充可用空间"), SecondAdd.AddedCount, 3);
	TestEqual(TEXT("背包满时返回剩余数量"), SecondAdd.RemainingCount, 1);

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("堆栈数量保持容量上限"), Items.Num(), 2);
	TestEqual(TEXT("第一个堆栈已填满"), Items[0].StackCount, 5);
	TestEqual(TEXT("第二个堆栈已填满"), Items[1].StackCount, 5);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryMutationInvariantTest,
	"DragonOath.Inventory.MutationInvariant",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryMutationInvariantTest::RunTest(const FString& /*Parameters*/)
{
	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), TEXT("DO_Test_Mutation"));
	const FDOItemInstanceRecord FirstItem = MakeTestItem(DefinitionId, 0, 1);
	const FDOItemInstanceRecord SecondItem = MakeTestItem(DefinitionId, 2, 1);

	FDOInventoryMutationChangeSet ChangeSet;
	TestTrue(TEXT("合法快照通过纯算法槽位唯一性校验"), FDOInventoryMutation::ValidateUniqueSlots({ FirstItem, SecondItem }, 4, &ChangeSet));
	TestEqual(TEXT("变更集包含每个实例一次"), ChangeSet.ChangedInstanceIds.Num(), 2);

	FDOItemInstanceRecord DuplicateSlot = SecondItem;
	DuplicateSlot.SlotIndex = FirstItem.SlotIndex;
	TestFalse(TEXT("重复槽位被纯算法拒绝"), FDOInventoryMutation::ValidateUniqueSlots({ FirstItem, DuplicateSlot }, 4));
	TestFalse(TEXT("超容量快照被纯算法拒绝"), FDOInventoryMutation::ValidateUniqueSlots({ FirstItem, SecondItem }, 1));
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryRevisionAndNoOpTest,
	"DragonOath.Inventory.RevisionAndNoOp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryRevisionAndNoOpTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* Definition = CreateTestDefinition(TEXT("DO_Test_Revision"), 10);
	APlayerState* Owner = nullptr;
	UDOInventoryComponent* Inventory = CreateTestInventory(Owner, 4);
	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), Definition->GetFName());

	const int32 RevisionBeforeAdd = Inventory->GetRevision();
	TestEqual(TEXT("一次添加事务只递增一次 Revision"), Inventory->TryAddItem(DefinitionId, 1).AddedCount, 1);
	TestEqual(TEXT("添加事务 Revision 增量为一"), Inventory->GetRevision(), RevisionBeforeAdd + 1);

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("Revision 测试拥有一个实例"), Items.Num(), 1))
	{
		return false;
	}

	const int32 RevisionBeforeNoOp = Inventory->GetRevision();
	Inventory->RequestMoveItem(Items[0].InstanceId, Items[0].SlotIndex, Items[0].SlotIndex, 0, 7001);
	TestEqual(TEXT("移动到原槽位是 NoOp 且不递增 Revision"), Inventory->GetRevision(), RevisionBeforeNoOp);

	const int32 RevisionBeforeRemove = Inventory->GetRevision();
	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	TestTrue(TEXT("删除最后一个实例成功"), Inventory->TryConsumeItem(Items[0].InstanceId, 1, FailureReason));
	TestEqual(TEXT("删除事务 Revision 只递增一次"), Inventory->GetRevision(), RevisionBeforeRemove + 1);
	TestEqual(TEXT("删除最后一个实例后库存为空"), Inventory->GetUsedSlotCount(), 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryConsumeAndSnapshotTest,
	"DragonOath.Inventory.ConsumeAndSnapshot",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryConsumeAndSnapshotTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* Definition = CreateTestDefinition(TEXT("DO_Test_Consume"), 20);
	APlayerState* Owner = nullptr;
	UDOInventoryComponent* Inventory = CreateTestInventory(Owner, 4);
	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), Definition->GetFName());

	const FDOItemInstanceRecord FirstItem = MakeTestItem(DefinitionId, 0, 3);
	const FDOItemInstanceRecord SecondItem = MakeTestItem(DefinitionId, 2, 2);
	const TArray<FDOItemInstanceRecord> ValidItems{ FirstItem, SecondItem };
	if (!TestTrue(TEXT("合法快照可以恢复"), Inventory->RestoreInventorySnapshot(ValidItems, 4)))
	{
		return false;
	}

	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	TestTrue(TEXT("按 DefinitionId 消耗成功"), Inventory->TryConsumeByDefinition(DefinitionId, 1, FailureReason));

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("消耗后仍保留两个堆栈"), Items.Num(), 2))
	{
		return false;
	}
	TestEqual(TEXT("优先消耗槽位靠前的堆栈"), Items[0].StackCount, 2);
	TestEqual(TEXT("其他堆栈保持不变"), Items[1].StackCount, 2);

	FDOItemInstanceRecord DuplicateSlot = MakeTestItem(DefinitionId, 0, 1);
	const TArray<FDOItemInstanceRecord> DuplicateSlotItems{ FirstItem, DuplicateSlot };
	TestFalse(TEXT("重复槽位快照被拒绝"), Inventory->ValidateInventorySnapshot(DuplicateSlotItems, 4));

	FDOItemInstanceRecord DuplicateId = SecondItem;
	DuplicateId.InstanceId = FirstItem.InstanceId;
	DuplicateId.SlotIndex = 3;
	const TArray<FDOItemInstanceRecord> DuplicateIdItems{ FirstItem, DuplicateId };
	TestFalse(TEXT("重复 InstanceId 快照被拒绝"), Inventory->ValidateInventorySnapshot(DuplicateIdItems, 4));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryMoveSplitSortDiscardTest,
	"DragonOath.Inventory.MoveSplitSortDiscard",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryMoveSplitSortDiscardTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* Definition = CreateTestDefinition(TEXT("DO_Test_MoveSplit"), 10);
	APlayerState* Owner = nullptr;
	UDOInventoryComponent* Inventory = CreateTestInventory(Owner, 4);
	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), Definition->GetFName());

	const FDOInventoryAddResult AddResult = Inventory->TryAddItem(DefinitionId, 8);
	TestEqual(TEXT("拆分测试物品添加成功"), AddResult.AddedCount, 8);

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("拆分前只有一个堆栈"), Items.Num(), 1))
	{
		return false;
	}

	const FGuid SourceId = Items[0].InstanceId;
	Inventory->RequestMoveItem(SourceId, 1, 3, 0, 99);
	Inventory->GetInventorySnapshot(Items);
	const FDOItemInstanceRecord* ItemAfterInvalidMove = Inventory->FindItemByInstanceId(SourceId);
	TestNotNull(TEXT("源槽位不匹配的移动请求被拒绝"), ItemAfterInvalidMove);
	if (ItemAfterInvalidMove)
	{
		TestEqual(TEXT("源槽位不匹配时物品位置不变"), ItemAfterInvalidMove->SlotIndex, 0);
	}

	Inventory->RequestSplitStack(SourceId, 0, 2, 3, 1);
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("拆分后生成两个堆栈"), Items.Num(), 2);
	const FDOItemInstanceRecord* SourceAfterSplit = Inventory->FindItemByInstanceId(SourceId);
	TestNotNull(TEXT("拆分后源实例仍存在"), SourceAfterSplit);
	if (!SourceAfterSplit)
	{
		return false;
	}
	TestEqual(TEXT("源堆栈数量正确"), SourceAfterSplit->StackCount, 5);

	FGuid SplitId;
	for (const FDOItemInstanceRecord& Item : Items)
	{
		if (Item.InstanceId != SourceId)
		{
			SplitId = Item.InstanceId;
		}
	}
	TestTrue(TEXT("拆分实例获得新的 InstanceId"), SplitId.IsValid() && SplitId != SourceId);

	Inventory->RequestMoveItem(SplitId, 2, 0, 2, 2);
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("合并后仍有两个堆栈"), Items.Num(), 2);
	const FDOItemInstanceRecord* SourceAfterMerge = Inventory->FindItemByInstanceId(SourceId);
	const FDOItemInstanceRecord* SplitAfterMerge = Inventory->FindItemByInstanceId(SplitId);
	TestNotNull(TEXT("合并后源实例存在"), SourceAfterMerge);
	TestNotNull(TEXT("合并后拆分实例存在"), SplitAfterMerge);
	if (SourceAfterMerge && SplitAfterMerge)
	{
		TestEqual(TEXT("目标堆栈合并数量正确"), SourceAfterMerge->StackCount, 7);
		TestEqual(TEXT("源堆栈剩余数量正确"), SplitAfterMerge->StackCount, 1);
	}

	Inventory->RequestDiscardItem(SplitId, 1, 3);
	TestNull(TEXT("丢弃最后一个堆栈后实例被删除"), Inventory->FindItemByInstanceId(SplitId));

	UDOItemDefinition* UniqueDefinition = CreateTestDefinition(TEXT("DO_Test_Unique"), 1);
	AddInventoryRules(UniqueDefinition, true);
	const FPrimaryAssetId UniqueDefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), UniqueDefinition->GetFName());
	TestEqual(TEXT("唯一物品第一次添加成功"), Inventory->TryAddItem(UniqueDefinitionId, 1).AddedCount, 1);
	const FDOInventoryAddResult DuplicateUniqueResult = Inventory->TryAddItem(UniqueDefinitionId, 1);
	TestEqual(TEXT("唯一物品重复添加不会增加数量"), DuplicateUniqueResult.AddedCount, 0);
	TestEqual(TEXT("唯一物品重复添加返回禁止原因"), DuplicateUniqueResult.FailureReason, EDOInventoryFailureReason::NotAllowed);
	const FDOInventoryAddResult UniqueStackResult = Inventory->TryAddItem(UniqueDefinitionId, 2);
	TestEqual(TEXT("唯一物品不能一次添加多个实例"), UniqueStackResult.AddedCount, 0);

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventorySortConfigTest,
	"DragonOath.Inventory.SortConfig",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventorySortConfigTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* EquipmentDefinition = CreateTestEquipmentDefinition(
		TEXT("DO_Test_Sort_Equipment"),
		DragonOathGameplayTags::Equipment::Slot::Weapon);
	EquipmentDefinition->SortPriority = 1;
	EquipmentDefinition->ItemType = DragonOathGameplayTags::Item::Type::Equipment;

	UDOItemDefinition* ConsumableDefinition = CreateTestConsumableDefinition(TEXT("DO_Test_Sort_Consumable"));
	ConsumableDefinition->SortPriority = 100;

	APlayerState* Owner = nullptr;
	UDOInventoryComponent* Inventory = CreateTestInventory(Owner, 4);
	const FPrimaryAssetType TestType(TEXT("DOTestItemDefinition"));
	const FPrimaryAssetId EquipmentId(TestType, EquipmentDefinition->GetFName());
	const FPrimaryAssetId ConsumableId(TestType, ConsumableDefinition->GetFName());

	TestEqual(TEXT("先加入消耗品"), Inventory->TryAddItem(ConsumableId, 1).AddedCount, 1);
	TestEqual(TEXT("再加入装备"), Inventory->TryAddItem(EquipmentId, 1).AddedCount, 1);

	UDOInventorySortConfig* SortConfig = NewObject<UDOInventorySortConfig>(GetTransientPackage());
	SortConfig->ItemTypeWeights.Add(DragonOathGameplayTags::Item::Type::Consumable, 0);
	SortConfig->ItemTypeWeights.Add(DragonOathGameplayTags::Item::Type::Equipment, 10);
	Inventory->SetSortConfig(SortConfig);

	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	TestTrue(TEXT("使用 SortConfig 排序成功"), Inventory->TrySortInventory(FailureReason));

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (TestEqual(TEXT("排序后仍保留两个实例"), Items.Num(), 2))
	{
		TestEqual(TEXT("SortConfig 权重优先于 Definition SortPriority"), Items[0].DefinitionId, ConsumableId);
		TestEqual(TEXT("排序后的第二项为装备"), Items[1].DefinitionId, EquipmentId);
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryMutationRevisionBatchTest,
	"DragonOath.Inventory.MutationRevisionBatch",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryMutationRevisionBatchTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* Definition = CreateTestDefinition(TEXT("DO_Test_Mutation_Batch"), 1);
	APlayerState* Owner = nullptr;
	UDOInventoryComponent* Inventory = CreateTestInventory(Owner, 4);
	const FPrimaryAssetId DefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), Definition->GetFName());
	TestEqual(TEXT("批量事务准备两个实例"), Inventory->TryAddItem(DefinitionId, 2).AddedCount, 2);

	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("批量事务拥有两个实例"), Items.Num(), 2))
	{
		return false;
	}

	const int32 RevisionBeforeBatch = Inventory->GetRevision();
	Inventory->BeginMutation();
	Inventory->RequestMoveItem(Items[0].InstanceId, Items[0].SlotIndex, 2, 0, 9101);
	Inventory->RequestMoveItem(Items[1].InstanceId, Items[1].SlotIndex, 3, 0, 9102);
	TestEqual(TEXT("事务未结束前 Revision 不变"), Inventory->GetRevision(), RevisionBeforeBatch);
	Inventory->EndMutation();
	TestEqual(TEXT("批量事务结束时 Revision 只增加一次"), Inventory->GetRevision(), RevisionBeforeBatch + 1);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryUseQuickBarAndSaveTest,
	"DragonOath.Inventory.UseQuickBarAndSave",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryUseQuickBarAndSaveTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* ConsumableDefinition = CreateTestConsumableDefinition(TEXT("DO_Test_HealthPotion"));
	const FPrimaryAssetId ConsumableDefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), ConsumableDefinition->GetFName());

	FDOAutomationWorld TestWorld;
	ADOPlayerState* PlayerState = TestWorld.SpawnPlayerState();
	if (!TestNotNull(TEXT("创建测试 PlayerState"), PlayerState))
	{
		return false;
	}
	InitializeTestAbilitySystem(PlayerState);

	UDOInventoryComponent* Inventory = PlayerState->GetInventoryComponent();
	UDOItemQuickBarComponent* QuickBar = PlayerState->GetItemQuickBarComponent();
	if (!TestNotNull(TEXT("创建测试背包组件"), Inventory) || !TestNotNull(TEXT("创建测试快捷栏组件"), QuickBar))
	{
		return false;
	}

	TestEqual(TEXT("添加测试药水数量"), Inventory->TryAddItem(ConsumableDefinitionId, 2).AddedCount, 2);
	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("药水添加后只有一个堆栈"), Items.Num(), 1))
	{
		return false;
	}

	EDOInventoryFailureReason FailureReason = EDOInventoryFailureReason::None;
	TestTrue(TEXT("使用即时 GameplayEffect 消耗品成功"), Inventory->TryUseItemByDefinition(ConsumableDefinitionId, FailureReason));
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("消耗品效果成功后才扣除一个数量"), Items.Num(), 1);
	if (Items.Num() == 1)
	{
		TestEqual(TEXT("消耗品剩余数量正确"), Items[0].StackCount, 1);
	}

	const TArray<FPrimaryAssetId> QuickBarDefinitions{
		ConsumableDefinitionId,
		FPrimaryAssetId(),
		FPrimaryAssetId(),
		FPrimaryAssetId() };
	TestTrue(TEXT("快捷栏可以绑定背包中存在的消耗品"), QuickBar->RestoreQuickBarSnapshot(QuickBarDefinitions));
	TestEqual(TEXT("快捷栏保存 DefinitionId 而非 InstanceId"), QuickBar->GetDefinitionForSlot(0), ConsumableDefinitionId);

	UDOSaveGame* SaveGame = UDOSaveGame::CaptureFromPlayerState(PlayerState);
	if (!TestNotNull(TEXT("可以创建背包存档"), SaveGame))
	{
		return false;
	}
	if (SaveGame)
	{
		TestEqual(TEXT("存档包含背包物品"), SaveGame->InventoryItems.Num(), 1);
		TestEqual(TEXT("存档包含四个快捷栏槽位"), SaveGame->QuickBarDefinitions.Num(), UDOItemQuickBarComponent::QuickBarSlotCount);
	}

	const TArray<FDOItemInstanceRecord> EmptyItems;
	TestTrue(TEXT("清空背包用于验证恢复"), Inventory->RestoreInventorySnapshot(EmptyItems, 40));
	TestTrue(TEXT("存档可以恢复背包和快捷栏"), SaveGame && SaveGame->RestoreToPlayerState(PlayerState));
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("恢复后背包物品数量一致"), Items.Num(), 1);
	TestEqual(TEXT("恢复后快捷栏绑定一致"), QuickBar->GetDefinitionForSlot(0), ConsumableDefinitionId);

	FailureReason = EDOInventoryFailureReason::None;
	TestTrue(TEXT("用尽快捷栏绑定物品"), Inventory->TryUseItemByDefinition(ConsumableDefinitionId, FailureReason));
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("物品用尽后库存为空"), Items.Num(), 0);
	UDOSaveGame* ZeroStockSaveGame = UDOSaveGame::CaptureFromPlayerState(PlayerState);
	TArray<uint8> ZeroStockSaveBytes;
	TestTrue(TEXT("零库存快捷栏状态可以序列化"), ZeroStockSaveGame && UGameplayStatics::SaveGameToMemory(ZeroStockSaveGame, ZeroStockSaveBytes));
	UDOSaveGame* LoadedZeroStockSaveGame = Cast<UDOSaveGame>(UGameplayStatics::LoadGameFromMemory(ZeroStockSaveBytes));
	TestTrue(TEXT("零库存快捷栏绑定不会阻断存档恢复"), LoadedZeroStockSaveGame && LoadedZeroStockSaveGame->RestoreToPlayerState(PlayerState));
	TestEqual(TEXT("零库存恢复后保留快捷栏 DefinitionId"), QuickBar->GetDefinitionForSlot(0), ConsumableDefinitionId);

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentTransactionTest,
	"DragonOath.Equipment.Transaction",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentTransactionTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* EquipmentDefinition = CreateTestEquipmentDefinition(
		TEXT("DO_Test_Equipment_Transaction"),
		DragonOathGameplayTags::Equipment::Slot::Weapon);
	const FPrimaryAssetId EquipmentDefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), EquipmentDefinition->GetFName());

	FDOAutomationWorld TestWorld;
	ADOPlayerState* PlayerState = TestWorld.SpawnPlayerState();
	if (!TestNotNull(TEXT("创建装备事务 PlayerState"), PlayerState))
	{
		return false;
	}
	InitializeTestAbilitySystem(PlayerState);

	UDOInventoryComponent* Inventory = PlayerState->GetInventoryComponent();
	UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
	if (!TestNotNull(TEXT("装备事务背包组件有效"), Inventory) || !TestNotNull(TEXT("装备事务装备组件有效"), Equipment))
	{
		return false;
	}

	TestEqual(TEXT("装备事务测试物品添加成功"), Inventory->TryAddItem(EquipmentDefinitionId, 1).AddedCount, 1);
	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("装备前背包有一件装备"), Items.Num(), 1))
	{
		return false;
	}

	const FGuid EquipmentInstanceId = Items[0].InstanceId;
	Equipment->RequestEquipItem(EquipmentInstanceId, 101);
	TArray<FDOEquippedItemEntry> EquippedEntries;
	Equipment->GetEquippedSnapshot(EquippedEntries);
	TestEqual(TEXT("装备到空槽成功"), EquippedEntries.Num(), 1);
	TestTrue(TEXT("装备槽状态正确"), Equipment->IsSlotEquipped(DragonOathGameplayTags::Equipment::Slot::Weapon));
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("装备成功后物品从背包移出"), Items.Num(), 0);

	Equipment->RequestUnequipItem(DragonOathGameplayTags::Equipment::Slot::Weapon, 102);
	Equipment->GetEquippedSnapshot(EquippedEntries);
	TestEqual(TEXT("卸下后装备槽为空"), EquippedEntries.Num(), 0);
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("卸下后装备回到背包"), Items.Num(), 1);

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOCombatRatingLibraryTest,
	"DragonOath.Inventory.CombatRatingLibrary",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOCombatRatingLibraryTest::RunTest(const FString& /*Parameters*/)
{
	FDOCombatRatingInput Input;
	Input.AttackPower = 100.0f;
	Input.DefensePower = 80.0f;
	Input.MaxHealth = 200.0f;
	Input.MaxMana = 100.0f;
	Input.CriticalRating = 20.0f;
	Input.HitRating = 10.0f;
	Input.EvasionRating = 10.0f;
	Input.AttackSpeed = 0.5f;
	Input.MoveSpeed = 20.0f;
	Input.LifeStealRate = 0.02f;

	TestEqual(TEXT("默认战力公式由配置 CDO 提供"), UDOCombatRatingLibrary::CalculateCombatPower(Input), 117);
	TestEqual(TEXT("默认守护力公式由配置 CDO 提供"), UDOCombatRatingLibrary::CalculateGuardPower(Input), 146);

	UDOCombatRatingConfig* CustomConfig = NewObject<UDOCombatRatingConfig>(GetTransientPackage());
	CustomConfig->CombatAttackPowerWeight = 2.0f;
	CustomConfig->GuardDefensePowerWeight = 2.0f;
	TestEqual(TEXT("自定义战力权重生效"), UDOCombatRatingLibrary::CalculateCombatPower(Input, CustomConfig), 217);
	TestEqual(TEXT("自定义守护力权重生效"), UDOCombatRatingLibrary::CalculateGuardPower(Input, CustomConfig), 226);

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentSaveAndRestoreTest,
	"DragonOath.Equipment.SaveAndRestore",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentSaveAndRestoreTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* EquipmentDefinition = CreateTestEquipmentDefinition(
		TEXT("DO_Test_Equipment_SaveRestore"),
		DragonOathGameplayTags::Equipment::Slot::Chest);
	const FPrimaryAssetId EquipmentDefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), EquipmentDefinition->GetFName());

	FDOAutomationWorld TestWorld;
	ADOPlayerState* PlayerState = TestWorld.SpawnPlayerState();
	if (!TestNotNull(TEXT("创建装备存档 PlayerState"), PlayerState))
	{
		return false;
	}
	InitializeTestAbilitySystem(PlayerState);

	UDOInventoryComponent* Inventory = PlayerState->GetInventoryComponent();
	UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
	if (!TestNotNull(TEXT("装备存档背包组件有效"), Inventory) || !TestNotNull(TEXT("装备存档装备组件有效"), Equipment))
	{
		return false;
	}

	TestEqual(TEXT("装备存档测试物品添加成功"), Inventory->TryAddItem(EquipmentDefinitionId, 1).AddedCount, 1);
	TArray<FDOItemInstanceRecord> Items;
	Inventory->GetInventorySnapshot(Items);
	if (!TestEqual(TEXT("装备存档装备前背包有一件物品"), Items.Num(), 1))
	{
		return false;
	}

	Equipment->RequestEquipItem(Items[0].InstanceId, 201);
	TArray<FDOEquippedItemEntry> EquippedEntries;
	Equipment->GetEquippedSnapshot(EquippedEntries);
	if (!TestEqual(TEXT("装备存档捕获前装备槽有一件装备"), EquippedEntries.Num(), 1))
	{
		return false;
	}

	UDOSaveGame* SaveGame = UDOSaveGame::CaptureFromPlayerState(PlayerState);
	if (!TestNotNull(TEXT("装备状态可以写入存档"), SaveGame))
	{
		return false;
	}
	if (SaveGame)
	{
		TestEqual(TEXT("存档包含装备条目"), SaveGame->EquippedItems.Num(), 1);
		TestEqual(TEXT("装备存档时普通背包为空"), SaveGame->InventoryItems.Num(), 0);
	}

	Equipment->RequestUnequipItem(DragonOathGameplayTags::Equipment::Slot::Chest, 202);
	TestTrue(TEXT("恢复前先卸下装备"), !Equipment->IsSlotEquipped(DragonOathGameplayTags::Equipment::Slot::Chest));

	TestTrue(TEXT("装备存档可以恢复装备和属性 GE"), SaveGame && SaveGame->RestoreToPlayerState(PlayerState));
	TestTrue(TEXT("恢复后装备槽重新装备"), Equipment->IsSlotEquipped(DragonOathGameplayTags::Equipment::Slot::Chest));
	Inventory->GetInventorySnapshot(Items);
	TestEqual(TEXT("恢复后装备不会重复出现在背包"), Items.Num(), 0);

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentSourceGrantLifecycleTest,
	"DragonOath.Equipment.SourceGrantLifecycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentSourceGrantLifecycleTest::RunTest(const FString& /*Parameters*/)
{
	FDOAutomationWorld TestWorld;
	ADOPlayerState* PlayerState = TestWorld.SpawnPlayerState();
	if (!TestNotNull(TEXT("创建来源授予测试 PlayerState"), PlayerState))
	{
		return false;
	}
	InitializeTestAbilitySystem(PlayerState);

	UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent();
	UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
	if (!TestNotNull(TEXT("来源授予测试 ASC 有效"), ASC) || !TestNotNull(TEXT("来源授予测试装备组件有效"), Equipment))
	{
		return false;
	}

	UDOEquipmentInstance* Instance = NewObject<UDOEquipmentInstance>(Equipment);
	FDOItemInstanceRecord Item;
	Item.InstanceId = FGuid::NewGuid();
	Item.DefinitionId = FPrimaryAssetId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), TEXT("DO_Test_SourceGrant"));
	Instance->Initialize(Item, DragonOathGameplayTags::Equipment::Slot::Weapon);

	UDOAbilitySet* AbilitySet = NewObject<UDOAbilitySet>(GetTransientPackage());
	FDOAbilityGrant AbilityGrant;
	AbilityGrant.AbilityId = DragonOathGameplayTags::Ability::Id::PrimaryAttack;
	AbilityGrant.AbilityClass = UDOTestGameplayAbility::StaticClass();
	AbilityGrant.InitialLevel = 1;
	AbilitySet->GrantedAbilities.Add(AbilityGrant);

	FDOGameplayEffectGrant EffectGrant;
	EffectGrant.GameplayEffectClass = UGE_SharedCooling::StaticClass();
	AbilitySet->GrantedGameplayEffects.Add(EffectGrant);

	FDOAbilitySetGrantedHandles Handles;
	TestTrue(TEXT("来源 AbilitySet 授予成功"), ASC->GiveDOAbilitySetForSource(AbilitySet, Instance, Handles));
	TestEqual(TEXT("来源句柄记录一个 Ability"), Handles.AbilitySpecHandles.Num(), 1);
	TestEqual(TEXT("来源句柄记录一个 GameplayEffect"), Handles.GameplayEffectHandles.Num(), 1);
	if (Handles.AbilitySpecHandles.Num() == 1)
	{
		const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handles.AbilitySpecHandles[0]);
		TestNotNull(TEXT("来源 AbilitySpec 存在"), Spec);
		if (Spec)
		{
			TestTrue(TEXT("AbilitySpec SourceObject 指向 EquipmentInstance"), Spec->SourceObject.Get() == Instance);
		}
	}

	ASC->RemoveDOAbilitySet(Handles);
	if (Handles.AbilitySpecHandles.Num() == 1)
	{
		TestNull(TEXT("卸下来源后 AbilitySpec 已移除"), ASC->FindAbilitySpecFromHandle(Handles.AbilitySpecHandles[0]));
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentAffixSpecTest,
	"DragonOath.Equipment.AffixSpec",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentAffixSpecTest::RunTest(const FString& /*Parameters*/)
{
	FDOAutomationWorld TestWorld;
	ADOPlayerState* PlayerState = TestWorld.SpawnPlayerState();
	if (!TestNotNull(TEXT("创建词缀 Spec 测试 PlayerState"), PlayerState))
	{
		return false;
	}
	InitializeTestAbilitySystem(PlayerState);
	UDOAbilitySystemComponent* ASC = PlayerState->GetDOAbilitySystemComponent();
	UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
	if (!TestNotNull(TEXT("词缀 Spec 测试 ASC 有效"), ASC)
		|| !TestNotNull(TEXT("词缀 Spec 测试 Equipment 有效"), Equipment))
	{
		return false;
	}

	FDOAttributeModifierValues BaseValues;
	BaseValues.AttackPower = 10.0f;
	FDOItemAffixRoll Affix;
	Affix.AffixTag = DragonOathGameplayTags::Data::Equipment::AttackPower;
	Affix.Magnitude = 7.5f;
	TArray<FDOItemAffixRoll> Affixes{ Affix };
	FGameplayEffectSpecHandle SpecHandle;
	UDOEquipmentInstance* SourceObject = NewObject<UDOEquipmentInstance>(Equipment);
	SourceObject->Initialize(FDOItemInstanceRecord(), DragonOathGameplayTags::Equipment::Slot::Chest);
	TestTrue(TEXT("装备 Spec 构建成功"), FDOItemEffectSpecBuilder::BuildEquipmentSpec(*ASC, *SourceObject, BaseValues, Affixes, 1.0f, SpecHandle));
	if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
	{
		TestEqual(TEXT("装备基础属性与实例词缀合并"), SpecHandle.Data->GetSetByCallerMagnitude(DragonOathGameplayTags::Data::Equipment::AttackPower), 17.5f);
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentDurabilityRuntimeStateTest,
	"DragonOath.Equipment.DurabilityRuntimeState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentDurabilityRuntimeStateTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* Definition = CreateTestEquipmentDefinition(
		TEXT("DO_Test_Durability_Equipment"),
		DragonOathGameplayTags::Equipment::Slot::Chest);
	UDOItemFragment_Equipment* Fragment = nullptr;
	for (UObject* FragmentObject : Definition->Fragments)
	{
		if (UDOItemFragment_Equipment* EquipmentFragment = Cast<UDOItemFragment_Equipment>(FragmentObject))
		{
			Fragment = EquipmentFragment;
			break;
		}
	}
	if (Fragment)
	{
		Fragment->MaxDurability = 100;
	}

	FDOAutomationWorld TestWorld;
	ADOPlayerState* PlayerState = TestWorld.SpawnPlayerState();
	if (!TestNotNull(TEXT("创建耐久测试 PlayerState"), PlayerState))
	{
		return false;
	}
	InitializeTestAbilitySystem(PlayerState);
	UDOEquipmentComponent* Equipment = PlayerState->GetEquipmentComponent();
	if (!TestNotNull(TEXT("耐久测试 Equipment 有效"), Equipment))
	{
		return false;
	}

	FDOEquippedItemEntry Entry;
	Entry.SlotTag = DragonOathGameplayTags::Equipment::Slot::Chest;
	Entry.Item = MakeTestItem(FPrimaryAssetId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), Definition->GetFName()), INDEX_NONE, 1);
	Entry.Item.CurrentDurability = 0;
	TestTrue(TEXT("耐久为零的装备仍可恢复到槽位"), Equipment->RestoreEquippedSnapshot({ Entry }));
	const UDOEquipmentInstance* Instance = Equipment->FindEquipmentInstance(Entry.SlotTag);
	TestNotNull(TEXT("耐久为零的装备实例存在"), Instance);
	if (Instance)
	{
		TestFalse(TEXT("耐久为零时不施加属性 GE"), Instance->GetAttributeEffectHandle().IsValid());
	}

	TestTrue(TEXT("修复耐久后重建装备运行时状态"), Equipment->SetEquippedDurability(Entry.SlotTag, 50));
	Instance = Equipment->FindEquipmentInstance(Entry.SlotTag);
	if (Instance)
	{
		TestTrue(TEXT("修复后属性 GE 生效"), Instance->GetAttributeEffectHandle().IsValid());
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentSnapshotValidationTest,
	"DragonOath.Equipment.SnapshotValidation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentSnapshotValidationTest::RunTest(const FString& /*Parameters*/)
{
	UDOItemDefinition* HeadDefinition = CreateTestEquipmentDefinition(
		TEXT("DO_Test_Equipment_Head"),
		DragonOathGameplayTags::Equipment::Slot::Head);
	UDOItemDefinition* WeaponDefinition = CreateTestEquipmentDefinition(
		TEXT("DO_Test_Equipment_Weapon"),
		DragonOathGameplayTags::Equipment::Slot::Weapon);

	const FPrimaryAssetId HeadDefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), HeadDefinition->GetFName());
	const FPrimaryAssetId WeaponDefinitionId(FPrimaryAssetType(TEXT("DOTestItemDefinition")), WeaponDefinition->GetFName());
	const FDOItemInstanceRecord HeadItem = MakeTestItem(HeadDefinitionId, INDEX_NONE, 1);
	const FDOItemInstanceRecord WeaponItem = MakeTestItem(WeaponDefinitionId, INDEX_NONE, 1);

	ADOPlayerState* PlayerState = NewObject<ADOPlayerState>(
		GetTransientPackage(),
		MakeUniqueObjectName(GetTransientPackage(), ADOPlayerState::StaticClass(), TEXT("DOEquipmentTestOwner")));
	UDOEquipmentComponent* Equipment = PlayerState ? PlayerState->GetEquipmentComponent() : nullptr;
	if (!TestNotNull(TEXT("创建装备组件"), Equipment))
	{
		return false;
	}

	FDOEquippedItemEntry HeadEntry;
	HeadEntry.SlotTag = DragonOathGameplayTags::Equipment::Slot::Head;
	HeadEntry.Item = HeadItem;

	FDOEquippedItemEntry WeaponEntry;
	WeaponEntry.SlotTag = DragonOathGameplayTags::Equipment::Slot::Weapon;
	WeaponEntry.Item = WeaponItem;

	TestTrue(TEXT("合法的不同装备槽快照可以通过校验"), Equipment->ValidateEquippedSnapshot({ HeadEntry, WeaponEntry }));

	FDOEquippedItemEntry DuplicateSlotEntry = WeaponEntry;
	DuplicateSlotEntry.SlotTag = DragonOathGameplayTags::Equipment::Slot::Head;
	TestFalse(TEXT("重复装备槽快照会被拒绝"), Equipment->ValidateEquippedSnapshot({ HeadEntry, DuplicateSlotEntry }));

	FDOEquippedItemEntry DuplicateInstanceEntry = WeaponEntry;
	DuplicateInstanceEntry.Item.InstanceId = HeadEntry.Item.InstanceId;
	TestFalse(TEXT("重复物品实例快照会被拒绝"), Equipment->ValidateEquippedSnapshot({ HeadEntry, DuplicateInstanceEntry }));

	FDOEquippedItemEntry MismatchedSlotEntry = HeadEntry;
	MismatchedSlotEntry.SlotTag = DragonOathGameplayTags::Equipment::Slot::Weapon;
	TestFalse(TEXT("记录槽位与物品定义槽位不匹配时会被拒绝"), Equipment->ValidateEquippedSnapshot({ MismatchedSlotEntry }));

	FDOEquippedItemEntry StackableEquipmentEntry = HeadEntry;
	StackableEquipmentEntry.Item.InstanceId = FGuid::NewGuid();
	StackableEquipmentEntry.Item.StackCount = 2;
	TestFalse(TEXT("装备实例堆叠数量大于一时会被拒绝"), Equipment->ValidateEquippedSnapshot({ StackableEquipmentEntry }));

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOInventoryItemDefinitionAssetValidationTest,
	"DragonOath.Inventory.ItemDefinitionAssets",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOInventoryItemDefinitionAssetValidationTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<FString> AssetPaths = {
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_HealthPotion_Small.DA_Item_HealthPotion_Small"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_ManaPotion_Small.DA_Item_ManaPotion_Small"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Material_Iron.DA_Item_Material_Iron"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Quest_Key.DA_Item_Quest_Key"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Equipment_Head_01.DA_Item_Equipment_Head_01"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Equipment_Chest_01.DA_Item_Equipment_Chest_01"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Equipment_Hands_01.DA_Item_Equipment_Hands_01"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Equipment_Feet_01.DA_Item_Equipment_Feet_01"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Equipment_Accessory_01.DA_Item_Equipment_Accessory_01"),
		TEXT("/Game/DragonOath/Items/Definitions/DA_Item_Equipment_Weapon_01.DA_Item_Equipment_Weapon_01")
	};

	for (const FString& AssetPath : AssetPaths)
	{
		UDOItemDefinition* Definition = LoadObject<UDOItemDefinition>(nullptr, *AssetPath);
		if (!TestNotNull(*FString::Printf(TEXT("测试 ItemDefinition 已加载：%s"), *AssetPath), Definition))
		{
			continue;
		}

		FDataValidationContext Context;
		const EDataValidationResult Result = Definition->IsDataValid(Context);
		TestTrue(*FString::Printf(TEXT("ItemDefinition 通过校验：%s"), *AssetPath), Result != EDataValidationResult::Invalid);
	}

	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
