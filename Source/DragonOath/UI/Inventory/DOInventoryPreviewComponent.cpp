#include "UI/Inventory/DOInventoryPreviewComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOInventoryPreviewComponent)

UDOInventoryPreviewComponent::UDOInventoryPreviewComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDOInventoryPreviewComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureCaptureObjects();
	DeactivatePreview();
}

void UDOInventoryPreviewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivatePreview();
	if (SceneCapture)
	{
		SceneCapture->DestroyComponent();
		SceneCapture = nullptr;
	}
	RenderTarget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UDOInventoryPreviewComponent::EnsureCaptureObjects()
{
	if (!GetOwner())
	{
		return;
	}

	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("InventoryPreviewRenderTarget"));
		RenderTarget->RenderTargetFormat = RTF_RGBA16f;
		RenderTarget->ClearColor = FLinearColor(0.015f, 0.02f, 0.035f, 1.0f);
		RenderTarget->InitAutoFormat(512, 512);
		RenderTarget->UpdateResourceImmediate(true);
	}

	if (!SceneCapture)
	{
		SceneCapture = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("InventoryPreviewSceneCapture"));
		SceneCapture->RegisterComponent();
		SceneCapture->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		SceneCapture->SetRelativeLocation(FVector(-300.0f, 0.0f, 120.0f));
		SceneCapture->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		SceneCapture->FOVAngle = 35.0f;
		SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = false;
		SceneCapture->TextureTarget = RenderTarget;
		SceneCapture->ShowOnlyActors.Add(GetOwner());
	}
}

void UDOInventoryPreviewComponent::ActivatePreview()
{
	EnsureCaptureObjects();
	if (!SceneCapture)
	{
		return;
	}

	bPreviewActive = true;
	SceneCapture->Activate(true);
	CapturePreview();
}

void UDOInventoryPreviewComponent::CapturePreview()
{
	if (bPreviewActive && SceneCapture)
	{
		SceneCapture->CaptureScene();
	}
}

void UDOInventoryPreviewComponent::DeactivatePreview()
{
	bPreviewActive = false;
	if (SceneCapture)
	{
		SceneCapture->Deactivate();
	}
}
