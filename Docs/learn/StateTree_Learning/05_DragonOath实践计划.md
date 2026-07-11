# 05. DragonOath 实践计划

## 背景

DragonOath 项目技术架构（[02_Technical_Architecture.md](../../02_Technical_Architecture.md)）已明确：

> M5 先用 `EnemyCharacter + ASC + StateTree / 简单 C++ 状态` 做少量怪物。
> M6/M8 压力出现后再引入 Mass + StateTree。

本计划聚焦 **M5 阶段：Actor 怪物 + StateTree** 的落地步骤。

## 里程碑对齐

| 里程碑 | StateTree 角色 | 关键交付 |
|--------|---------------|----------|
| M5 | 核心行为引擎 | 近战怪、远程怪各一棵完整 StateTree |
| M6 | 行为模板化 | 抽取通用 State（Patrol/Chase/Attack/Death）为 Linked Asset 复用 |
| M7 | Boss 复杂行为 | 多阶段 Boss（GAS + StateTree 混合编排） |
| M8+ | Mass 迁移准备 | 将 Actor StateTree 逻辑转为 Mass Schema 兼容版本 |

## 步骤 1：启用与配置

### 确认插件启用

UE 5.8 中 StateTree 默认启用。检查 `.uproject`：

```json
{ "Name": "StateTree", "Enabled": true }
```

### 项目 C++ 依赖

```cpp
// DragonOath.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "StateTreeModule",
    "GameplayStateTreeModule",  // 如需 GAS 集成
});
```

## 步骤 2：最小行为树（近战怪）

### 状态设计

```text
                        ┌──────────┐
              ┌────────▶│  Idle    │◀────────┐
              │         │ (停留)    │         │
              │         └────┬─────┘         │
              │              │ HasTarget     │ !HasTarget
              │              ▼               │
              │         ┌──────────┐         │
              │         │  Chase   │         │
              │         │ (靠近)    │         │
              │         └────┬─────┘         │
              │              │ InRange       │
              │              ▼               │
              │         ┌──────────┐         │
              │         │  Attack  │─────────┘
              │         │ (攻击)    │  AttackDone
              │         └────┬─────┘
              │              │ Health<=0
              │              ▼
              │         ┌──────────┐
              └─────────│  Dead    │
                        │ (死亡)    │
                        └──────────┘
```

### StateTree 资产结构

```
ST_Enemy_Melee (Root, Actor Schema)
  ├─ [State] Idle
  │    ├─ Enter Conditions: —
  │    ├─ Tasks:
  │    │   └─ Delay Task (Duration=1.0~3.0, RandomDeviation=1.0)
  │    └─ Transitions:
  │        └─→ Chase  [Condition: Object Is Valid (CurrentTarget)]
  │
  ├─ [State] Chase
  │    ├─ Tasks:
  │    │   └─ MoveToTarget (自定义 Task)
  │    └─ Transitions:
  │        ├─→ Attack  [Condition: DistanceCompare (CurrentTarget < AttackRange)]
  │        └─→ Idle   [Condition: !Object Is Valid (CurrentTarget)]
  │
  ├─ [State] Attack
  │    ├─ Tasks:
  │    │   └─ Trigger GAS Ability (自定义 Task, Tag="Ability.Attack.Melee")
  │    └─ Transitions:
  │        ├─→ Idle   [Condition: Delay 1s after AttackDone]
  │        └─→ Dead   [Condition: Health <= 0]   ← 外部 Event 触发
  │
  └─ [State] Dead
       ├─ Tasks:
       │   ├─ Play Animation Montage (Death)
       │   └─ Disable Collision
       └─ Transitions: —
```

## 步骤 3：远程怪行为树

远程怪对比近战怪的关键差异：

| 维度 | 近战怪 | 远程怪 |
|------|--------|--------|
| Attack 条件 | 进入近战范围 | 进入射程 + 有视线 |
| Chase → Attack 之间 | 直接切换 | 增加 `Strafe`（侧移保持距离） |
| !HasTarget 后 | 返回 Idle | 返回 `TakeCover` 或 Idle |
| Attack 方式 | 单次/连段近战 | 投射物 + 冷却 |

### 远程怪额外状态

```text
[State] Chase
  └─→ [State] Strafe    [Condition: Distance < MinRange]
       ├─ Tasks: MoveToCircleAround(Target, MinRange, MaxRange)
       └─→ Attack       [Condition: HasLineOfSight]
            └─→ Strafe  [Condition: AttackCooldown]
```

## 步骤 4：StateTree 与 GAS 集成

### Task 触发 Ability

```cpp
// DragonOathTask_TriggerAbility.h
USTRUCT(meta = (DisplayName = "Trigger GAS Ability"))
struct FDragonOathTriggerAbilityTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Parameter")
    FGameplayTag AbilityTag;  // 如 "Ability.Attack.Melee"

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, ...) const override
    {
        // 1. 从 Context 获取 Actor
        AActor* Owner = Context.GetOwner();
        // 2. 获取 ASC
        UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
        // 3. 触发 Ability
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
        return EStateTreeRunStatus::Running;
    }
};
```

### Event 驱动 Transition

```cpp
// C++ 侧：受到伤害时发送 StateTree Event
// 注意：项目规范禁止手写 FGameplayTag::RequestGameplayTag 字符串，
//      应引用 DOGameplayTag.h 中声明的变量（此处用 DragonOathGameplayTags::Event::StateTreeDamaged 示意）。
void AEnemyCharacter::OnDamaged(float Damage, AActor* Instigator)
{
    FStateTreeEvent Event;
    Event.Tag = DragonOathGameplayTags::Event::StateTreeDamaged;   // 不要写 RequestGameplayTag("...")
    Event.Payload = FInstancedStruct::Make(FDamagedPayload{ Damage, Instigator });

    StateTreeComponent->SendStateTreeEvent(Event);
}
```

StateTree 中：
- Transition: Any State → Stun [Condition: OnEvent(对应 Tag) && Health > 0]
- Transition: Any State → Dead [Condition: OnEvent("StateTree.Event.Damaged") && Health <= 0]

## 步骤 5：行为复用（Linked Asset）

M6 阶段将通用状态抽取为 Linked Asset：

```text
ST_Common_Idle       # 通用 Idle（Delay + 巡逻选点）
ST_Common_Patrol     # 通用巡逻（巡逻路径跟随）
ST_Common_Dead       # 通用死亡（动画 + 禁用 + 延迟销毁）
ST_Common_Stun       # 通用硬直（动画 + 恢复时间）
```

然后在具体怪物 StateTree 中引用：
```
ST_Enemy_Melee
  ├─ [Linked State] ST_Common_Idle
  ├─ [State] Chase     ← 自定义
  ├─ [State] Attack    ← 自定义
  └─ [Linked State] ST_Common_Dead
```

## 实验记录模板

每次实践在 `Practice/` 下记录：

```markdown
# 实验：<标题>

- 日期：2026-MM-DD
- 实验分支：`feature/state-tree-xxx`
- 改动文件：列出

## 目的
一句话。

## 步骤
1.
2.

## 验证方式
- PIE / Editor / 日志

## 结论
- 成功/失败/待改进
- 截图或日志关键行

## 后续
- [ ] 待办
```

## 检查清单

```text
[ ] 项目 .Build.cs 添加 StateTreeModule 依赖
[ ] 创建 ST_Enemy_Melee 资产
[ ] 创建 EnemyCharacter C++ 类（含 UStateTreeComponent）
[ ] 实现自定义 Task：MoveToTarget、TriggerGASAbility
[ ] PIE 验证 Idle → Chase → Attack → Idle 循环
[ ] 接入 Event：OnDamaged → Stun / Dead
[ ] 创建远程怪 StateTree（ST_Enemy_Ranged）
[ ] 实现 Strafe + 视线检测
[ ] 抽取通用状态为 Linked Asset
[ ] 更新 README 实践记录
```

## 参考资料

- [StateTree Module 源码](file:///d:/UE_5.8/Engine/Plugins/Runtime/StateTree/Source/StateTreeModule/)
- [MassAI StateTree 集成](file:///d:/ue_texiao/DragonOath/Docs/learn/MassAI_Learning/03_StateTree%E9%9B%86%E6%88%90.md)
- [02_Technical_Architecture.md](../../02_Technical_Architecture.md) — 怪物 AI 架构章节
