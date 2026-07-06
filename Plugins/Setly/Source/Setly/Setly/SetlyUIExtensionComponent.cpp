// Copyright Epic Games, Inc. All Rights Reserved.

#include "Setly/SetlyUIExtensionComponent.h"

#include "CommonActivatableWidget.h"
#include "CommonLocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "PrimaryGameLayout.h"
#include "SetlyLogChannels.h"
#include "UIExtensionSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SetlyUIExtensionComponent)

USetlyUIExtensionComponent::USetlyUIExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USetlyUIExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogLyra, Log, TEXT("SetlyUIExtension BeginPlay Owner=%s PlayerIndex=%d Layouts=%d RegisterWidgets=%d"),
		*GetNameSafe(GetOwner()),
		PlayerIndex,
		Layouts.Num(),
		RegisterWidgets.Num());

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension skipped: World is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	if (UUIExtensionSubsystem* ExtensionSubsystem = World->GetSubsystem<UUIExtensionSubsystem>())
	{
		for (const FRegisterExtensionAsWidgetEntry& Entry : RegisterWidgets)
		{
			if (Entry.SlotID.IsValid() && Entry.WidgetClass)
			{
				UE_LOG(LogLyra, Log, TEXT("SetlyUIExtension registering widget. Slot=%s WidgetClass=%s Priority=%d"),
					*Entry.SlotID.ToString(),
					*GetNameSafe(Entry.WidgetClass.Get()),
					Entry.Priority);

				ExtensionHandles.Add(ExtensionSubsystem->RegisterExtensionAsWidget(Entry.SlotID, Entry.WidgetClass, Entry.Priority));
			}
			else
			{
				UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension register entry skipped. Slot=%s WidgetClass=%s"),
					*Entry.SlotID.ToString(),
					*GetNameSafe(Entry.WidgetClass.Get()));
			}
		}
	}
	else
	{
		UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension skipped widget registration: UUIExtensionSubsystem is null. World=%s"), *GetNameSafe(World));
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	ULocalPlayer* LocalPlayer = GameInstance ? GameInstance->GetLocalPlayerByIndex(PlayerIndex) : nullptr;
	UE_LOG(LogLyra, Log, TEXT("SetlyUIExtension local player lookup. GameInstance=%s LocalPlayer=%s LocalPlayerClass=%s"),
		*GetNameSafe(GameInstance),
		*GetNameSafe(LocalPlayer),
		LocalPlayer ? *GetNameSafe(LocalPlayer->GetClass()) : TEXT("None"));

	UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer);
	if (!CommonLocalPlayer)
	{
		UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension cannot push layouts: LocalPlayer is not UCommonLocalPlayer. Configure LocalPlayerClassName=/Script/Setly.LyraLocalPlayer."));
		return;
	}

	UPrimaryGameLayout* PrimaryLayout = CommonLocalPlayer ? UPrimaryGameLayout::GetPrimaryGameLayout(CommonLocalPlayer) : nullptr;
	if (!PrimaryLayout)
	{
		UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension cannot push layouts: PrimaryGameLayout is null. Check GameInstanceClass, DefaultUIPolicyClass, and root layout creation logs."));
		return;
	}

	for (const FContentToLayerWidgetEntry& Entry : Layouts)
	{
		if (Entry.LayerID.IsValid() && Entry.WidgetClass)
		{
			UE_LOG(LogLyra, Log, TEXT("SetlyUIExtension pushing layout widget. Layer=%s WidgetClass=%s"),
				*Entry.LayerID.ToString(),
				*GetNameSafe(Entry.WidgetClass.Get()));

			if (UCommonActivatableWidget* Widget = PrimaryLayout->PushWidgetToLayerStack<UCommonActivatableWidget>(Entry.LayerID, Entry.WidgetClass.Get()))
			{
				PushedWidgets.Add(Widget);
				UE_LOG(LogLyra, Log, TEXT("SetlyUIExtension pushed widget successfully. Widget=%s Layer=%s"),
					*GetNameSafe(Widget),
					*Entry.LayerID.ToString());
			}
			else
			{
				UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension push returned null. Layer=%s WidgetClass=%s"),
					*Entry.LayerID.ToString(),
					*GetNameSafe(Entry.WidgetClass.Get()));
			}
		}
		else
		{
			UE_LOG(LogLyra, Warning, TEXT("SetlyUIExtension layout entry skipped. Layer=%s WidgetClass=%s"),
				*Entry.LayerID.ToString(),
				*GetNameSafe(Entry.WidgetClass.Get()));
		}
	}
}

void USetlyUIExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogLyra, Log, TEXT("SetlyUIExtension EndPlay Owner=%s ExtensionHandles=%d PushedWidgets=%d"),
		*GetNameSafe(GetOwner()),
		ExtensionHandles.Num(),
		PushedWidgets.Num());

	for (FUIExtensionHandle& Handle : ExtensionHandles)
	{
		Handle.Unregister();
	}
	ExtensionHandles.Reset();

	for (UCommonActivatableWidget* Widget : PushedWidgets)
	{
		if (Widget)
		{
			UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(Widget->GetOwningLocalPlayer());
			if (UPrimaryGameLayout* PrimaryLayout = CommonLocalPlayer ? UPrimaryGameLayout::GetPrimaryGameLayout(CommonLocalPlayer) : nullptr)
			{
				PrimaryLayout->FindAndRemoveWidgetFromLayer(Widget);
			}
		}
	}
	PushedWidgets.Reset();

	Super::EndPlay(EndPlayReason);
}
