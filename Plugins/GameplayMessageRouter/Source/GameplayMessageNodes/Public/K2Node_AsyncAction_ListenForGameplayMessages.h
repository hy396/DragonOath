// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "K2Node_AsyncAction.h"

#include "K2Node_AsyncAction_ListenForGameplayMessages.generated.h"

class FBlueprintActionDatabaseRegistrar;
class FKismetCompilerContext;
class FMulticastDelegateProperty;
class FString;
class UEdGraph;
class UEdGraphPin;
class UObject;

/**
 * 自定义蓝图节点：`Listen For Gameplay Messages`
 *
 * 专门为 `UAsyncAction_ListenForGameplayMessage` 生成可视化节点。
 * 标准 K2Node_AsyncAction 已经能处理"异步节点 + 输入参数 + 委托输出"这套模板，
 * 本类在此基础上做两件事：
 *
 *   1. **通配符 Payload 引脚类型刷新**
 *      用户选 PayloadType 后，`RefreshOutputPayloadType()` 把输出 Payload 引脚
 *      从 Wildcard 改成用户指定的结构体类型。
 *
 *   2. **自动拼接 GetPayload 调用**
 *      委托触发后，编译期自动在逻辑链尾部插入对 `GetPayload` 的调用，
 *      把拷贝结果接到 Payload 输出引脚——用户不用手动连 GetPayload。
 *
 * 这是典型的"编译期 K2Node 改写"手法，属于引擎进阶特性。
 * 一般不需要修改，除非要加新参数或改变节点外观。
 */
UCLASS()
class UK2Node_AsyncAction_ListenForGameplayMessages : public UK2Node_AsyncAction
{
	GENERATED_BODY()

	//~UEdGraphNode interface
	// 节点重建后（蓝图重新加载等）同步刷新 Payload 引脚类型
	virtual void PostReconstructNode() override;
	// 用户在 PayloadType 引脚输入不同类型时，刷新 Payload 输出引脚
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	// 鼠标悬停时的提示文本
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~End of UEdGraphNode interface

	//~UK2Node interface
	// 注册到蓝图菜单（让用户在右键菜单里能找到 Listen For Gameplay Messages）
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	// 创建默认引脚：在父类的基础上加一个通配符 Payload 输出引脚
	virtual void AllocateDefaultPins() override;
	//~End of UK2Node interface

protected:
	// 编译期：把委托的广播流水线拼接到逻辑链
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext) override;

private:

	// 编译期：在委托逻辑末尾插入 GetPayload 调用，把结果赋给 Payload 输出引脚
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	// 根据 PayloadType 输入引脚的值，把输出 Payload 通配符引脚变成具体类型
	void RefreshOutputPayloadType();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
	UEdGraphPin* GetOutputChannelPin() const;
};
