#include "ItemSystem/Equipment/DOEquipmentAppearanceRegistry.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystem/Core/DOGameplayTag.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOEquipmentAppearanceRegistryTest,
	"DragonOath.Equipment.AppearanceRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDOEquipmentAppearanceRegistryTest::RunTest(const FString& /*Parameters*/)
{
	UDOEquipmentAppearanceRegistry* Registry = NewObject<UDOEquipmentAppearanceRegistry>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient appearance registry can be created."), Registry))
	{
		return false;
	}

	FDOEquipmentAppearanceEntry BaseEntry;
	BaseEntry.AppearanceId = TEXT("Weapon_01");
	BaseEntry.AttachSocket = TEXT("hand_r");

	FDOEquipmentAppearanceEntry RubyEntry = BaseEntry;
	RubyEntry.VariantTag = DragonOathGameplayTags::Equipment::Appearance::Variant::Ruby;
	RubyEntry.AttachSocket = TEXT("hand_r_ruby");

	Registry->Entries.Add(BaseEntry);
	Registry->Entries.Add(RubyEntry);

	FDOEquipmentAppearanceEntry Resolved;
	TestTrue(TEXT("Exact variant resolves first."), Registry->ResolveAppearance(
		TEXT("Weapon_01"),
		DragonOathGameplayTags::Equipment::Appearance::Variant::Ruby,
		Resolved));
	TestEqual(TEXT("Exact variant keeps its socket."), Resolved.AttachSocket, FName(TEXT("hand_r_ruby")));

	Resolved = FDOEquipmentAppearanceEntry();
	TestTrue(TEXT("Unknown variant uses empty-variant fallback."), Registry->ResolveAppearance(
		TEXT("Weapon_01"),
		DragonOathGameplayTags::Equipment::Appearance::Variant::Sapphire,
		Resolved));
	TestEqual(TEXT("Fallback uses the base socket."), Resolved.AttachSocket, FName(TEXT("hand_r")));
	TestFalse(TEXT("Unknown appearance does not resolve."), Registry->ResolveAppearance(TEXT("Missing"), FGameplayTag(), Resolved));

	return true;
}

#endif
