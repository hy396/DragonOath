# 04. 导航与 SmartObject

## MassAI 中的三条导航路径

MassAI 同时支持多种导航后端，按场景选型：

| 路径 | 模块 | 典型场景 |
|------|------|----------|
| **NavMesh** | MassNavMeshNavigation | 常规 3D/横版地面寻路，依赖 Recast NavMesh |
| **ZoneGraph** | MassZoneGraphNavigation | 车道、人行道、人群通道（MassCrowd） |
| **SmartObjects** | MassAIBehavior + MassSmartObjects | 占位、交互点、座椅、掩体等 |

DragonOath 横版关卡：**首期 Actor 怪用 NavMesh 或简化移动即可**；Mass 实验阶段可同样基于 NavMesh，不必强行上 ZoneGraph。

## NavMesh 路线

相关 Task：

- `MassNavMeshPathfollowTask` — 沿路径移动
- `MassNavMeshStandTask` — 停止移动
- `MassNavMeshFindReachablePointTask` — 找可达点
- `MassNavMeshAnimateTask` — 移动与动画同步（若配置了 Representation）

**前提：** 关卡有有效 NavMesh Bounds Volume，且 Agent 具备 Mass Movement Trait。

横版注意点：

- NavMesh 通常是 2D 可行域投影，与横版「锁定 Y/Z 轴」移动规则需一致
- 大量 Agent 同时寻路时，考虑简化逻辑（直线追击、轨道移动）降低 NavMesh 查询压力

## ZoneGraph 路线

依赖 `ZoneGraph` + `ZoneGraphAnnotations` 插件。

- 在路网车道上行驶，适合 MassCrowd 式人群
- Task 如 `MassZoneGraphPathFollowTask`、`MassZoneGraphFindEscapeTarget`
- DragonOath 主线关卡若为房间式横版，**优先级低于 NavMesh**

学习建议：若要体验 ZoneGraph，可启用 **MassCrowd** 示例插件对照样本地图。

## SmartObject 路线

依赖 `SmartObjects` 插件与 `MassSmartObjects` 模块。

典型流程：

```text
MassFindSmartObjectTask → MassClaimSmartObjectTask → MassUseSmartObjectTask
```

适用于：

- 怪物找掩体、占点
- NPC 使用场景交互点
- 群体行为中的「占位」逻辑

横版项目中，SmartObject 可用于「刷怪点」「跳板」「机关触发站位」等，但首期怪物可不接入。

## LookAt 系统

MassAI 还提供 `UMassLookAtTrait` / `MassLookAtTask` / `UMassLookAtSubsystem`：

- 批量处理朝向（玩家、队友、目标点）
- 与 Representation 配合驱动上半身/头部朝向（视表现方案而定）

## 导航选型建议（DragonOath 草案）

| 场景 | 推荐 |
|------|------|
| 直线涌来的刷怪潮 | 简化 Movement Processor 或 NavMesh 短路径 |
| 房间内巡逻杂兵 | NavMesh |
| 城镇背景人群（若有关） | ZoneGraph + MassCrowd 参考 |
| Boss 舞台机关站位 | Actor 逻辑为主；Mass 杂兵可用 SmartObject |

## 练习

1. 对比阅读 `MassNavMeshPathfollowTask.h` 与 `MassZoneGraphPathFollowTask.h` 的类注释。
2. 在横版实验关卡仅放 NavMesh，记录 Mass Agent 能否路径跟随。
3. 思考：刷怪潮是否需要完整 NavMesh，还是「朝玩家 Lerp」就够。

## 下一篇

[05_与Actor怪物的边界.md](05_与Actor怪物的边界.md)
