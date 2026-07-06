// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：Common UI 设置界面框架，用于设置项、设置集合、动态数据源和设置页面。

#pragma once

#include "GameSetting.h"

#include "GameSettingAction.generated.h"

//--------------------------------------
// UGameSettingAction
//--------------------------------------

class ULocalPlayer;

/** 自定义动作委托 */
DECLARE_DELEGATE_TwoParams(UGameSettingCustomAction, UGameSetting* /*Setting*/, ULocalPlayer* /*LocalPlayer*/)

/**
 * 动作型设置（如按钮操作），用于执行不可逆操作或导航。
 * 例如显示致谢、EULA、恢复默认等操作。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingAction : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingAction();

public:

	/** 命名动作执行事件 */
	DECLARE_EVENT_TwoParams(UGameSettingAction, FOnExecuteNamedAction, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);
	FOnExecuteNamedAction OnExecuteNamedActionEvent;  // 命名动作执行事件委托

public:

	/** 获取动作文本 */
	FText GetActionText() const { return ActionText; }
	// 设置动作文本
	void SetActionText(FText Value) { ActionText = Value; }
#if !UE_BUILD_SHIPPING
	/** 非发布版本的重载，用于不需要本地化的场景 */
	void SetActionText(const FString& Value) { SetActionText(FText::FromString(Value)); }
#endif

	/** 获取命名动作标签 */
	FGameplayTag GetNamedAction() const { return NamedAction; }
	// 设置命名动作标签
	void SetNamedAction(FGameplayTag Value) { NamedAction = Value; }

	/** 是否绑定了自定义动作 */
	bool HasCustomAction() const { return CustomAction.IsBound(); }
	// 设置自定义动作委托
	void SetCustomAction(UGameSettingCustomAction InAction) { CustomAction = InAction; }
	/** 通过 Lambda 设置自定义动作 */
	void SetCustomAction(TFunction<void(ULocalPlayer*)> InAction);

	/**
	 * 设置动作是否会弄脏设置状态。默认不会，因为大多数动作不可逆（如显示致谢）。
	 * 如果动作需要触发变更事件，则设为 true。
	 */
	void SetDoesActionDirtySettings(bool Value) { bDirtyAction = Value; }

	/** 执行动作 */
	virtual void ExecuteAction();

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;

protected:
	FText ActionText;  // 动作文本
	FGameplayTag NamedAction;  // 命名动作标签
	UGameSettingCustomAction CustomAction;  // 自定义动作委托
	bool bDirtyAction = false;  // 动作是否会弄脏设置状态
};
