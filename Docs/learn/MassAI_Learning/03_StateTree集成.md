# 03. StateTree 集成

## 为什么 MassAI 要接 StateTree

DragonOath 首期怪物 AI 已计划用 **StateTree**（`EnemyCharacter` 路线）。MassAI 提供同一套行为工具的 **Mass 批量执行版**：

- 单个 StateTree 资产描述行为逻辑
- 每个 Mass Entity 持有 StateTree 实例数据（非完整 Actor）
- `UMassStateTreeProcessor` 批量推进状态机

这样可以在保持可视化行为编辑的同时，获得 Mass 的规模优势。

## 核心类型

### UMassStateTreeTrait

路径：`MassAI/Source/MassAIBehavior/Public/MassStateTreeTrait.h`

```cpp
UCLASS(meta=(DisplayName="StateTree"))
class UMassStateTreeTrait : public UMassEntityTraitBase
{
    UPROPERTY(EditAnywhere, meta=(RequiredAssetDataTags="Schema=/Script/MassAIBehavior.MassStateTreeSchema"))
    TObjectPtr<UStateTree> StateTree;
};
```

在 Mass Entity Config 中添加此 Trait，即声明该类 Agent 需要 StateTree 行为。

### UMassStateTreeSchema

路径：`MassStateTreeSchema.h`

- StateTree 的 Schema 类型须为 **Mass Behavior**（`UMassStateTreeSchema`）
- 限制可用的 Task / Evaluator / Condition 为 Mass 兼容类型
- 链接阶段收集 `FMassStateTreeDependency`，声明 Processor 对 Fragment 的依赖

**注意：** 普通 Actor 用 StateTree 资产不能直接拿来给 Mass 用，需用 Mass Schema 重建或转换。

### UMassStateTreeSubsystem

管理 Mass 上下文中 StateTree 实例数据的分配与生命周期（`AllocateInstanceData` 等），避免每个 Entity 各自堆叠 UObject 开销。

### UMassStateTreeProcessor

每帧（按 Phase）批量执行 StateTree 逻辑，驱动 Task 完成移动、站立、寻路等。

## 内置 Mass Task 示例

引擎 `MassAIBehavior/Public/Tasks/` 目录（选读）：

| Task                            | 用途               |
| ------------------------------- | ------------------ |
| `MassNavMeshPathfollowTask`   | NavMesh 路径跟随   |
| `MassNavMeshStandTask`        | NavMesh 上站立     |
| `MassZoneGraphPathFollowTask` | ZoneGraph 通道跟随 |
| `MassLookAtTask`              | 朝向目标           |
| `MassFindSmartObjectTask`     | 查找 SmartObject   |
| `MassUseSmartObjectTask`      | 使用 SmartObject   |
| `MassClaimSmartObjectTask`    | 占用 SmartObject   |

横版 2.5D 项目早期可优先关注 **NavMesh** 类 Task；ZoneGraph 更适合城市/车道样本（如 MassCrowd）。

## 最小行为树结构（概念）

```text
[Root]
  └─ Selector
       ├─ Sequence: Patrol
       │    ├─ MassNavMeshPathFollowTask
       │    └─ MassNavMeshStandTask
       └─ Sequence: Chase（条件：发现玩家）
            └─ MassNavMeshPathFollowTask（目标=玩家位置）
```

实际条件/感知需配合 Mass 感知 Fragment 或自定义 Processor；上图为学习用骨架。

## 与 Actor StateTree 的差异

| 维度     | Actor StateTree                  | Mass StateTree                   |
| -------- | -------------------------------- | -------------------------------- |
| 执行载体 | `UStateTreeComponent` on Actor | `UMassStateTreeProcessor` 批量 |
| Schema   | 默认 Actor Schema                | `UMassStateTreeSchema`         |
| Task 库  | 通用 + 游戏自定义                | Mass 专用 Task 为主              |
| 实例数据 | 每 Actor 一份                    | Subsystem 池化 + per-entity 句柄 |
| 调试     | Actor 级可视化                   | Mass 调试模块 + Entity 视图      |

## 练习

1. 在编辑器创建 StateTree，确认 Schema 选 **Mass Behavior**。
2. 打开 `MassNavMeshPathfollowTask`，记录它需要哪些 Fragment（阅读头文件 `GetStateTreeDependencies` 或文档注释）。
3. 设计一个「刷怪潮小兵」两段式行为：生成 → 向玩家方向移动（不写代码，只画状态图）。

## 下一篇

[04_导航与SmartObject.md](04_导航与SmartObject.md)
