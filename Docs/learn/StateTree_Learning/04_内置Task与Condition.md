# 04. 内置 Task 与 Condition

## 概述

UE 5.8 StateTree 模块内置了一批可复用的 Task / Condition / Evaluator / Consideration。DragonOath 项目应**优先使用内置节点**，仅在内置不足以表达特殊逻辑时才自定义。

## 内置 Condition

所有 Condition 位于 `StateTreeModule/Public/Conditions/`。

### 通用条件（StateTreeCommonConditions.h）

| Condition | DisplayName | 说明 | 关键参数 |
|-----------|-------------|------|----------|
| `FStateTreeCompareIntCondition` | Integer Compare | 整数比较 | Left, Right, Operator, bInvert |
| `FStateTreeCompareFloatCondition` | Float Compare | 浮点数比较 | Left, Right, Operator, bInvert |
| `FStateTreeCompareBoolCondition` | Bool Compare | 布尔比较 | Left, Right |
| `FStateTreeCompareEnumCondition` | Enum Compare | 枚举比较 | Left, Right |

**运算符支持**：`Less`, `LessOrEqual`, `Equal`, `NotEqual`, `GreaterOrEqual`, `Greater`

> 可直接绑定到 `AEnemyCharacter::CurrentHealth`、`DetectionRange` 等属性。

### GameplayTag 条件（StateTreeGameplayTagConditions.h）

| Condition | 说明 | 典型用途 |
|-----------|------|----------|
| `FGameplayTagSingleMatchCondition` | 单 Tag 匹配 | `HasTag(A.Boss)`、`HasTag(State.Stunned)` |
| `FGameplayTagQueryCondition` | Tag Query 匹配 | 复杂 Tag 查询（多个 Tag 的 AND/OR） |
| `FGameplayTagContainerMatchCondition` | Tag Container 匹配 | 判断「是否拥有某组 Tag」 |

### 对象条件（StateTreeObjectConditions.h）

| Condition | DisplayName | 说明 |
|-----------|-------------|------|
| `FStateTreeObjectIsValidCondition` | Object Is Valid | 判断对象是否非空 |
| `FStateTreeObjectEqualsCondition` | Object Equals | 判断两对象是否相同 |

> 常用于 `CurrentTarget != nullptr` 检测 → 触发追逐 Transition。

### 条件辅助（StateTreeConditionHelpers.h）

| Condition | 说明 |
|-----------|------|
| `StateTreeCompareDistanceCondition` | 距离比较 |
| `StateTreeCompareBBEntryCondition` | 兼容旧 BehaviorTree 黑板（不推荐） |
| `StateTreeRandomCondition` | 随机条件（按权重随机通过） |
| `StateTreeCooldownCondition` | 冷却条件（通过后进入冷却） |

## 内置 Task

### Delay Task（StateTreeDelayTask.h）

```cpp
// 等待指定时长后 Succeeded
USTRUCT(meta = (DisplayName = "Delay Task"))
struct FStateTreeDelayTask : public FStateTreeTaskCommonBase
{
    float Duration = 1.f;           // 延迟时长（秒）
    float RandomDeviation = 0.f;    // 随机偏差
    bool bRunForever = false;       // 无限运行（等待 Transition 打断）
};
```

**典型用法**：
- `Idle` 状态中停留 2~3 秒随机
- `Attack` 状态后冷却

### Debug Text Task（StateTreeDebugTextTask.h）

在 HUD/屏幕上绘制调试文字。

```cpp
struct FStateTreeDebugTextTask
{
    bool bEnabled = true;
    FString Text;                   // 显示文本
    FColor TextColor = FColor::White;
    float FontScale = 1.f;
    FVector Offset;
    TObjectPtr<AActor> ReferenceActor;  // 绘制位置（可选）
};
```

### Run Parallel StateTree Task

```cpp
// StateTreeRunParallelStateTreeTask.h
// 并行运行另一个 StateTree 资产（进程内）
```

## 内置 Evaluator

Evaluator 在状态激活期间持续评估。

### 通用 Consideration（StateTreeCommonConsiderations.h）

| Consideration | 说明 |
|---------------|------|
| `FStateTreeConstantConsideration` | 返回固定分数 |
| `FStateTreeDistanceConsideration` | 距离 → 分数（通过曲线映射） |
| `FStateTreeAgeConsideration` | 实体年龄 → 分数 |

> Consideration 通常配合 Utility Selector 状态使用，DragonOath 前期不需要 Utility 选型，了解即可。

## 自定义 Task（DragonOath 扩展）

当内置不够时，按以下模板创建：

```cpp
// DragonOathTask_MoveToTarget.h
#pragma once
#include "StateTreeTaskBase.h"
#include "DragonOathTask_MoveToTarget.generated.h"

USTRUCT()
struct FDragonOathMoveToTargetTaskInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> Target;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float AcceptableRadius = 50.f;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float MoveSpeed = 600.f;
};

USTRUCT(meta = (DisplayName = "Move To Target"))
struct FDragonOathMoveToTargetTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FDragonOathMoveToTargetTaskInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override
    {
        // 获取实例数据
        FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
        // 开始移动逻辑...
        return EStateTreeRunStatus::Running;
    }

    virtual EStateTreeRunStatus Tick(
        FStateTreeExecutionContext& Context,
        const float DeltaTime) const override
    {
        FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
        // 每帧推进移动
        if (HasReachedTarget(InstanceData))
            return EStateTreeRunStatus::Succeeded;
        return EStateTreeRunStatus::Running;
    }

    virtual void ExitState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override
    {
        // 清理
    }
};
```

### 注册自定义 Task 到 Schema

1. Task 继承 `FStateTreeTaskCommonBase`（`Common` 命名空间）
2. Schema 的 `IsStructAllowed` 返回 `true` 对 `FStateTreeTaskCommonBase` 子类
3. 默认 Actor Schema 已允许所有 `CommonBase` 子类 → 无需额外注册

## DragonOath 怪物 AI 常用组合

| 行为 | Task | Condition | 说明 |
|------|------|-----------|------|
| 索敌 | (C++ Tick 更新 Target) | Object Is Valid + Distance Compare | 检测 CurrentTarget 存在且在范围内 |
| 靠近 | `Move To Target` (自定义) | — | 朝目标移动 |
| 攻击 | `Trigger GAS Ability` (自定义) | — | 通过 Tag 触发 `GA_Attack` |
| 返回 | `Move To` (自定义) | — | 返回出生点/巡逻点 |
| 死亡 | — | Health <= 0 | 进入 Dead 状态，禁用碰撞 |
| 巡逻 | `Move To` (自定义) + `Delay` | — | 巡逻点间移动 + 停留 |

## 练习

1. 在编辑器中为 Transition 添加一个 `Integer Compare` Condition，绑定到自定义属性。
2. 使用 `Delay Task` 实现 Idle 状态的停留。
3. 阅读 `StateTreeCommonConditions.h` 所有可用 Condition，列出 DragonOath 可用的 5 个场景。

## 下一篇

[05_DragonOath实践计划.md](05_DragonOath实践计划.md)
