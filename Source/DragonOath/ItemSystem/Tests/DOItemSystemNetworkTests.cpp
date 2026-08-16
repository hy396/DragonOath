#include "ItemSystem/Inventory/DOInventoryComponent.h"
#include "ItemSystem/Equipment/DOEquipmentComponent.h"
#include "ItemSystem/Equipment/DOEquipmentPresentationComponent.h"
#include "ItemSystem/Equipment/DOEquipmentPresentationTypes.h"
#include "Net/RepLayout.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

namespace
{
	bool HasReplicationCondition(UClass* Class, const TCHAR* PropertyName, const ELifetimeCondition ExpectedCondition)
	{
		if (!Class)
		{
			return false;
		}

		const FProperty* Property = Class->FindPropertyByName(PropertyName);
		if (!Property || !(Property->PropertyFlags & CPF_Net))
		{
			return false;
		}

		// 通过引擎最终生成的 RepLayout 查询条件，而不是直接在 CDO 上手动调用
		// GetLifetimeReplicatedProps。后者在 UE 5.8 下可能尚未分配稳定 RepIndex，
		// 会把多个属性都当成同一个 RepIndex，从而触发 CoreNet.h 的断言。
		const TSharedPtr<FRepLayout> RepLayout = FRepLayout::CreateFromClass(Class);
		if (!RepLayout.IsValid())
		{
			return false;
		}

		for (int32 ParentIndex = 0; ParentIndex < RepLayout->GetNumParents(); ++ParentIndex)
		{
			if (RepLayout->GetParentProperty(ParentIndex) == Property)
			{
				return RepLayout->GetParentCondition(ParentIndex) == ExpectedCondition;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOItemSystemReplicationContractTest,
	"DragonOath.ItemSystem.Network.ReplicationContract",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOItemSystemReplicationContractTest::RunTest(const FString& /*Parameters*/)
{
	TestTrue(TEXT("Inventory Capacity 为 OwnerOnly"), HasReplicationCondition(UDOInventoryComponent::StaticClass(), TEXT("Capacity"), COND_OwnerOnly));
	TestTrue(TEXT("Inventory Revision 为 OwnerOnly"), HasReplicationCondition(UDOInventoryComponent::StaticClass(), TEXT("Revision"), COND_OwnerOnly));
	TestTrue(TEXT("Inventory FastArray 为 OwnerOnly"), HasReplicationCondition(UDOInventoryComponent::StaticClass(), TEXT("InventoryList"), COND_OwnerOnly));
	TestTrue(TEXT("Equipment Revision 为 OwnerOnly"), HasReplicationCondition(UDOEquipmentComponent::StaticClass(), TEXT("Revision"), COND_OwnerOnly));
	TestTrue(TEXT("Equipment FastArray 为 OwnerOnly"), HasReplicationCondition(UDOEquipmentComponent::StaticClass(), TEXT("EquipmentList"), COND_OwnerOnly));
	TestTrue(TEXT("Public Presentation Revision 对所有相关客户端公开"), HasReplicationCondition(UDOEquipmentPresentationComponent::StaticClass(), TEXT("Revision"), COND_None));
	TestTrue(TEXT("Public Presentation FastArray 对所有相关客户端公开"), HasReplicationCondition(UDOEquipmentPresentationComponent::StaticClass(), TEXT("PublicEquipmentList"), COND_None));
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOItemSystemPublicAppearanceBoundaryTest,
	"DragonOath.ItemSystem.Network.PublicAppearanceBoundary",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FDOItemSystemPublicAppearanceBoundaryTest::RunTest(const FString& /*Parameters*/)
{
	const UScriptStruct* PublicEntryStruct = FDOEquipmentPublicEntry::StaticStruct();
	TestNotNull(TEXT("Public appearance entry struct 存在"), PublicEntryStruct);
	if (PublicEntryStruct)
	{
		TestNull(TEXT("公开外观不包含 InstanceId"), PublicEntryStruct->FindPropertyByName(TEXT("InstanceId")));
		TestNull(TEXT("公开外观不包含 Affixes"), PublicEntryStruct->FindPropertyByName(TEXT("Affixes")));
		TestNull(TEXT("公开外观不包含 CurrentDurability"), PublicEntryStruct->FindPropertyByName(TEXT("CurrentDurability")));
	}
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
