// Copyright Epic Games, Inc. All Rights Reserved.

#include "K2Node_AsyncAction_ListenForGameplayMessages.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "EdGraph/EdGraph.h"
#include "GameFramework/AsyncAction_ListenForGameplayMessage.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_AsyncAction_ListenForGameplayMessages)

class UEdGraph;

#define LOCTEXT_NAMESPACE "K2Node"

// 本节点各引脚的名字（内部字符串常量，外部不暴露）
namespace UK2Node_AsyncAction_ListenForGameplayMessagesHelper
{
	static FName ActualChannelPinName = "ActualChannel";	// 委托输出：实际收到消息的频道
	static FName PayloadPinName = "Payload";				// 输出：消息 Payload（通配符 → 用户指定类型）
	static FName PayloadTypePinName = "PayloadType";		// 输入：用户选的消息结构体类型
	static FName DelegateProxyPinName = "ProxyObject";		// 委托代理对象（隐藏引脚，内部用）
};

// 节点重构时刷新 Payload 引脚类型（蓝图加载 / 复制粘贴后）
void UK2Node_AsyncAction_ListenForGameplayMessages::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputPayloadType();
}

// 当 PayloadType 引脚值改变时刷新 Payload 引脚
// 只在 PayloadType 未连线时刷新（连线时用户自己控制类型）
void UK2Node_AsyncAction_ListenForGameplayMessages::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin == GetPayloadTypePin())
	{
		if (ChangedPin->LinkedTo.Num() == 0)
		{
			RefreshOutputPayloadType();
		}
	}
}

// 鼠标悬停 Payload 引脚时追加一行说明文字
void UK2Node_AsyncAction_ListenForGameplayMessages::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	Super::GetPinHoverText(Pin, HoverTextOut);
	if (Pin.PinName == UK2Node_AsyncAction_ListenForGameplayMessagesHelper::PayloadPinName)
	{
		HoverTextOut = HoverTextOut + LOCTEXT("PayloadOutTooltip", "\n\nThe message structure that we received").ToString();
	}
}

// 把本节点注册到蓝图右键菜单
// 遍历 UAsyncAction_ListenForGameplayMessage 的工厂函数，为每个工厂生成一个菜单项
void UK2Node_AsyncAction_ListenForGameplayMessages::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct GetMenuActions_Utils
	{
		// 为生成的节点设置工厂信息（让引擎知道调哪个 static 函数创建 AsyncAction）
		static void SetNodeFunc(UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UK2Node_AsyncAction_ListenForGameplayMessages* AsyncTaskNode = CastChecked<UK2Node_AsyncAction_ListenForGameplayMessages>(NewNode);
			if (FunctionPtr.IsValid())
			{
				UFunction* Func = FunctionPtr.Get();
				FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());

				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass        = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass               = ReturnProp->PropertyClass;
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterClassFactoryActions<UAsyncAction_ListenForGameplayMessage>(FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda([NodeClass](const UFunction* FactoryFunc)->UBlueprintNodeSpawner*
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
		check(NodeSpawner != nullptr);
		NodeSpawner->NodeClass = NodeClass;

		TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
		NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(GetMenuActions_Utils::SetNodeFunc, FunctionPtr);

		return NodeSpawner;
	}) );
}

// 创建节点默认引脚
// 隐藏内部使用的 ProxyObject 引脚，并额外添加一个通配符 Payload 输出引脚
void UK2Node_AsyncAction_ListenForGameplayMessages::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// UAsyncAction_ListenForGameplayMessage 的委托第一个参数是代理对象指针，
	// 供 GetPayload 调用使用，蓝图用户不需要看到，直接隐藏
	UEdGraphPin* DelegateProxyPin = FindPin(UK2Node_AsyncAction_ListenForGameplayMessagesHelper::DelegateProxyPinName);
	if (ensure(DelegateProxyPin))
	{
		DelegateProxyPin->bHidden = true;
	}

	// 新增 Payload 输出引脚（类型初始为 Wildcard，后续根据 PayloadType 输入自动变换）
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, UK2Node_AsyncAction_ListenForGameplayMessagesHelper::PayloadPinName);
}

// 处理委托触发时的逻辑：把标准的委托 Broadcast 流程拼接好，再追加 GetPayload 调用
// VariableOutputs 顺序：[0]=ProxyObject（内部） [1]=ActualChannel [2]=Payload
bool UK2Node_AsyncAction_ListenForGameplayMessages::HandleDelegates(const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;

	if (VariableOutputs.Num() != 3)
	{
		ensureMsgf(false, TEXT("UK2Node_AsyncAction_ListenForGameplayMessages::HandleDelegates - Variable output array not valid. Output delegates must only have the single proxy object output and than must have pin for payload."));
		return false;
	}

	// 遍历 ProxyClass 上的所有 MulticastDelegate（UAsyncAction_ListenForGameplayMessage 只有 OnMessageReceived）
	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		UEdGraphPin* LastActivatedThenPin = nullptr;
		// 父类帮我们把 Broadcast → 输出引脚 的标准逻辑接好
		bIsErrorFree &= FBaseAsyncTaskHelper::HandleDelegateImplementation(*PropertyIt, VariableOutputs, ProxyObjectPin, InOutLastThenPin, LastActivatedThenPin, this, SourceGraph, CompilerContext);

		// 再在后面追加：调用 GetPayload 把 Payload 内容拷出，赋值给 Payload 输出引脚
		bIsErrorFree &= HandlePayloadImplementation(*PropertyIt, VariableOutputs[0], VariableOutputs[2], VariableOutputs[1], LastActivatedThenPin, SourceGraph, CompilerContext);
	}

	return bIsErrorFree;
}

// 编译期生成 GetPayload 调用逻辑链：
//   CallGetPayload → AssignNode（Payload 输出 = 临时变量）
// 同时把 ActualChannel 输出引脚接到本地变量
bool UK2Node_AsyncAction_ListenForGameplayMessages::HandlePayloadImplementation(FMulticastDelegateProperty* CurrentProperty, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar, UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;
	const UEdGraphPin* PayloadPin = GetPayloadPin();
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	check(CurrentProperty && SourceGraph && Schema);

	const FEdGraphPinType& PinType = PayloadPin->PinType;

	// 若 Payload 还是通配符（用户没选 PayloadType）：
	//   - 没连线 → 直接跳过（允许无 Payload 用法）
	//   - 有连线 → 返回错误（必须指定 PayloadType）
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		if (PayloadPin->LinkedTo.Num() == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// 创建一个和 Payload 类型相同的临时变量，作为 GetPayload 的中转
	UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
		this, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(), PinType.ContainerType, PinType.PinValueType);

	// 生成 GetPayload 调用节点
	UK2Node_CallFunction* const CallGetPayloadNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallGetPayloadNode->FunctionReference.SetExternalMember(TEXT("GetPayload"), CurrentProperty->GetOwnerClass());
	CallGetPayloadNode->AllocateDefaultPins();

	// 把 GetPayload 的 self 引脚连到 ProxyObject（即 AsyncAction 实例）
	UEdGraphPin* GetPayloadCallSelfPin = Schema->FindSelfPin(*CallGetPayloadNode, EGPD_Input);
	if (GetPayloadCallSelfPin)
	{
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadCallSelfPin, ProxyObjectVar.TempVar->GetVariablePin());

		// 拼 Exec 链：委托 then → GetPayload Exec；GetPayload Then → Assign Exec
		UEdGraphPin* GetPayloadExecPin = CallGetPayloadNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* GetPayloadThenPin = CallGetPayloadNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

		UEdGraphPin* LastThenPin = nullptr;
		UEdGraphPin* GetPayloadPin = CallGetPayloadNode->FindPinChecked(TEXT("OutPayload"));
		// OutPayload 通配符绑到临时变量
		bIsErrorFree &= Schema->TryCreateConnection(TempVarOutput->GetVariablePin(), GetPayloadPin);


		// Payload 输出赋值节点：Payload 输出引脚 = 临时变量
		UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
		AssignNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadThenPin, AssignNode->GetExecPin());
		bIsErrorFree &= Schema->TryCreateConnection(PayloadVar.TempVar->GetVariablePin(), AssignNode->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetVariablePin());
		bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), TempVarOutput->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetValuePin());


		// 把原来委托链的下游接到 Assign 之后（保证继续往下执行）
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*InOutLastActivatedThenPin, *AssignNode->GetThenPin()).CanSafeConnect();
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastActivatedThenPin, GetPayloadExecPin);

		// 最后把 ActualChannel 输出引脚接到委托的本地变量
		UEdGraphPin* OutActualChannelPin = GetOutputChannelPin();
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OutActualChannelPin, *ActualChannelVar.TempVar->GetVariablePin()).CanSafeConnect();
	}

	return bIsErrorFree;
}

// 根据 PayloadType 输入引脚当前的值，把输出 Payload 引脚的类型调为对应的 UStruct
// PayloadType 为空时 → 回到 Wildcard
void UK2Node_AsyncAction_ListenForGameplayMessages::RefreshOutputPayloadType()
{
	UEdGraphPin* PayloadPin = GetPayloadPin();
	UEdGraphPin* PayloadTypePin = GetPayloadTypePin();

	if (PayloadTypePin->DefaultObject != PayloadPin->PinType.PinSubCategoryObject)
	{
		// 如果 Payload 曾被"分解"成多个子引脚（split），先合并回去
		if (PayloadPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(PayloadPin);
		}

		PayloadPin->PinType.PinSubCategoryObject = PayloadTypePin->DefaultObject;
		PayloadPin->PinType.PinCategory = (PayloadTypePin->DefaultObject == nullptr) ? UEdGraphSchema_K2::PC_Wildcard : UEdGraphSchema_K2::PC_Struct;
	}
}

// —— 以下三个 Getter 都用 FindPinChecked，断言引脚一定存在 ——

UEdGraphPin* UK2Node_AsyncAction_ListenForGameplayMessages::GetPayloadPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_AsyncAction_ListenForGameplayMessagesHelper::PayloadPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_ListenForGameplayMessages::GetPayloadTypePin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_AsyncAction_ListenForGameplayMessagesHelper::PayloadTypePinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_ListenForGameplayMessages::GetOutputChannelPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_AsyncAction_ListenForGameplayMessagesHelper::ActualChannelPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

#undef LOCTEXT_NAMESPACE
