#pragma once

#include "Components/ActorComponent.h"

#include "DOInventoryPreviewComponent.generated.h"

class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

/**
 * 背包页面的本地角色预览捕获器。
 *
 * 直接捕获当前 Pawn，作为基础角色预览占位；服饰系统接入后再提供独立的外观快照。
 * 只启用 ShowOnlyList，不会把场景中的其他角色和私有背包数据带入 UI。页面关闭时停止捕获。
 */
UCLASS(ClassGroup = (DragonOath), meta = (BlueprintSpawnableComponent))
class DRAGONOATH_API UDOInventoryPreviewComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDOInventoryPreviewComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 打开背包页面时启用捕获并立即更新一帧。 */
	void ActivatePreview();

	/** 角色或未来服饰外观变化后按需重新捕获。 */
	void CapturePreview();

	/** 关闭背包页面时停止捕获。 */
	void DeactivatePreview();

	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void EnsureCaptureObjects();

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	bool bPreviewActive = false;
};
