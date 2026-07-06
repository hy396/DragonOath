# 02. 在 Actor 上使用 StateTree

## 概述

DragonOath 首期怪物走 Actor 路线（`EnemyCharacter + GAS + StateTree`）。本节说明如何将 StateTree 挂载到 `AActor` 上。

## UStateTreeComponent

这是 Actor 与 StateTree 之间的桥梁组件。

### 基本用法

```cpp
// EnemyCharacter.h
#pragma once
#include "GameFramework/Character.h"
#include "Components/StateTreeComponent.h"
#include "StateTreeReference.h"

class AEnemyCharacter : public ACharacter
{
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UStateTreeComponent> StateTreeComponent;

    // 需要通过属性绑定的数据
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DetectionRange = 1000.f;
};
```

```cpp
// EnemyCharacter.cpp
AEnemyCharacter::AEnemyCharacter()
{
    StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    StateTreeComponent->StartLogic();  // 启动 StateTree
}
```

### 关键 API

```cpp
// 启动 / 停止
void UStateTreeComponent::StartLogic();
void UStateTreeComponent::StopLogic(EStateTreeRunStatus CompletionStatus);

// 发送事件
void UStateTreeComponent::SendStateTreeEvent(const FStateTreeEvent& Event);

// 获取运行状态
EStateTreeRunStatus UStateTreeComponent::GetStateTreeRunStatus() const;

// 获取实例数据（高级）
FStateTreeInstanceData& UStateTreeComponent::GetInstanceData();
```

## Schema

StateTree 的 Schema 决定了：

- 哪些 Task / Condition / Evaluator 可选
- 哪些外部数据（Context Data）可绑定
- 状态选择行为（Selector / Sequence 等）

### Actor 默认 Schema

使用 **`UStateTreeSchema`** 默认子类（或 `UStateTreeComponentSchema`）。编辑器创建 StateTree 资产时，选择 "Actor" Schema 即可。

### 自定义 Schema（高级）

当需要限制某些 StateTree 只能使用特定 Task 时：

```cpp
UCLASS()
class UDragonOathStateTreeSchema : public UStateTreeSchema
{
    GENERATED_BODY()

    virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override
    {
        // 只允许 FStateTreeTaskCommonBase 及其子类
        return InScriptStruct->IsChildOf(FStateTreeTaskCommonBase::StaticStruct());
    }
};
```

DragonOath 前期不需要自定义 Schema，直接用默认即可。

## 属性绑定（Property Bindings）

StateTree 不通过黑板传递数据，而是通过 **属性绑定** 直接读写外部对象的属性。

### 绑定源

StateTree 可绑定以下类型的外部数据：

| 数据源类型 | 说明 | 示例 |
|-----------|------|------|
| Context Actor | 挂载 StateTreeComponent 的 Actor | `AEnemyCharacter` |
| Global | 全局参数（在 StateTree 根上定义） | `EnemyConfig` |
| External Data | 通过 Schema 注册的外部对象 | `UAIPerceptionComponent` |

### 声明可绑定属性

```cpp
// 在 Schema 中声明 Context Data
USTRUCT()
struct FEnemyStateTreeContextData
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> CurrentTarget;

    UPROPERTY()
    float HealthPercent;

    UPROPERTY()
    bool bHasReachedWaypoint;
};
```

### 编辑器绑定

在 StateTree 编辑器中选择 Task/Condition 节点 → 点击属性旁的绑定按钮 → 选择绑定目标属性。

## 最小示例：Idle → Combat

### StateTree 结构

```text
[Root] (Actor Schema)
  ├─ [State] Idle
  │    └─ Task: Wait (Delay=2s)
  │
  └─ [State] Combat
       ├─ Task: FocusTarget
       └─ Task: MoveToTarget

Transition: Idle ──[HasTarget]──► Combat
Transition: Combat ──[!HasTarget]──► Idle
```

### C++ 侧配置

```cpp
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 1. 加载 StateTree 资产
    if (StateTreeComponent && EnemyStateTreeAsset)
    {
        StateTreeComponent->SetStateTree(EnemyStateTreeAsset);
    }

    // 2. 绑定 Context Data（如使用自定义 Schema）
    // StateTreeComponent->GetInstanceData().GetMutableStruct() = YourContextData;

    // 3. 启动
    StateTreeComponent->StartLogic();
}
```

### 蓝图侧配置

1. 在 EnemyCharacter 蓝图细节面板找到 `StateTreeComponent`
2. 设置 `State Tree` 为你的 `ST_Enemy_Melee` 资产
3. 在 `BeginPlay` 调用 `StartLogic`

## StateTree 与 Tick 的关系

- `UStateTreeComponent` 默认通过 `TickComponent` 驱动 StateTree Tick
- 可设置 `bStartWithTickEnabled = false` 然后手动调用 `TickStateTree(DeltaTime)`
- 每个 Active State 的 Task 按顺序 Tick

## 常见问题

### Q: StateTree 不启动？

检查：
1. `StateTreeComponent->StartLogic()` 是否被调用
2. StateTree 资产是否已设置
3. Schema 是否匹配

### Q: 属性绑定后值始终为默认？

检查：
1. Context Data 结构体是否在 Schema 中注册
2. 绑定的属性是否标记 `UPROPERTY()`
3. 外部数据是否正确传入

### Q: Transition 不触发？

检查：
1. Condition 中绑定的数据是否被正确更新
2. `EStateTreeRunStatus` 返回值是否正确

## 与 BehaviorTree 并行

UE 5.8 支持同一个 Actor 同时拥有 `UStateTreeComponent` 和 `UBrainComponent`（BehaviorTree），但 **不推荐**。

DragonOath 原则：
- 首期怪物 **只用 StateTree**
- Boss 可考虑 StateTree + GAS GameplayAbility 脚本
- 避免 BehaviorTree 引入额外心智负担

## 练习

1. 创建 `ST_Test` 资产，选择 Actor Schema，添加 Idle / Run 两个状态和一个 Transition。
2. 在测试 Actor 上挂载 `UStateTreeComponent`，绑定并运行。
3. 在 PIE 中通过 `LogStateTree` 观察状态切换日志。

## 下一篇

[03_状态树编辑器.md](03_状态树编辑器.md)
