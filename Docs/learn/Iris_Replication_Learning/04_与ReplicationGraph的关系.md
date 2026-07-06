# 04. Iris 与 ReplicationGraph 的关系

## 一句话区分

| 系统 | 层级 | 作用 |
|------|------|------|
| **Generic 复制** | 传统复制管线 | Actor Channel、`IsNetRelevantFor`、默认 NetDriver 路径 |
| **ReplicationGraph** | Generic 下的 **ReplicationDriver** | 用图节点预分类 Actor，优化大规模相关性 |
| **Iris** | **替代 Generic 的新复制管线** | NetObject + Bridge + Filter + Prioritizer + Serializer |

**ReplicationGraph 服务于传统 Generic 复制；Iris 启用后不走 ReplicationGraph 那条驱动链。**

二者不是「二选一叠加」，而是**不同复制架构世代**上的扩展方案。

## 架构对照图

```text
【路径 A：传统】
AActor → UNetDriver (Generic) → [可选 UReplicationGraph] → Actor Channel 同步

【路径 B：Iris】
AActor → UEngineReplicationBridge → UReplicationSystem
         → Filter / Prioritizer → DataStream → 客户端 NetObject 状态
```

## 选型建议（DragonOath）

### 优先考虑 Iris 的理由

- Epic 在 UE 5.x 持续投入 Iris（Push Model、并行化、Serializer）
- UE 5.8 引擎已内置 IrisCore 与默认 Filter/Prioritizer 配置
- 新项目没有历史 RepGraph 包袱，直接学 Iris 长期维护成本更低

### 仍值得学 ReplicationGraph 的理由

- 大量现有项目、教程、Lyra 早期思路仍围绕 RepGraph
- 理解 RepGraph 的「节点 / 路由 / 每连接列表」有助于理解 Iris 的 Filter 分层
- 若需维护旧项目或对比实验，RepGraph 仍实用

### DragonOath 推荐学习与实践顺序

```text
1. 联机基础
2. Iris（主推实践路径）
3. ReplicationGraph（对照理解 + 读懂旧资料）
4. 在架构文档中明确：首阶段目标为 Iris，RepGraph 仅作参考
```

## 配置冲突提醒

若同时在 `DefaultEngine.ini` 写：

```ini
; Generic 路径的 RepGraph 驱动
ReplicationDriverClassName="/Script/ReplicationGraph.BasicReplicationGraph"

; Iris 路径
[ConsoleVariables]
net.Iris.UseIrisReplication=1
```

**Iris 启用且 NetDriver 使用 Iris 时，RepGraph 驱动不会生效。** 避免误以为两套都在工作。

## 概念映射（帮助迁移思维）

| ReplicationGraph | Iris 近似概念 |
|------------------|---------------|
| `GridSpatialization2D` Node | `Spatial` Filter（`NetObjectGridWorldLocFilter`） |
| `AlwaysRelevant` Node | `DynamicFilterName=None` |
| `AlwaysRelevant_ForConnection` | 连接作用域 Filter / Bridge 策略 |
| `RouteAddNetworkActorToNodes` | `FilterConfigs` + `FActorReplicationParams` |
| `FGlobalActorReplicationInfo` | NetObject 全局策略 + ReplicationState |
| `GatherActorListsForConnection` | Filter 求值 + ReplicationView |

不是一一等价，但有助于从 RepGraph 笔记迁移到 Iris 配置。

## 与项目其他系统

- **GAS**：关注 Iris 对结构体 Serializer、`GameplayCue` 相关类型的支持列表
- **CommonGame / ModularGameplay**：联机入口仍在 `UNetDriver` / GameInstance；Iris 改的是复制内核
- **GameplayMessageRouter**：本地消息总线，与 Iris/RepGraph 正交（见 `04_Local_Message_Bus.md`）

## 练习

写一段 DragonOath 的「复制技术选型」草稿（200 字内），说明：

- 第一阶段是否启用 Iris
- ReplicationGraph 是否进入首期工程
- 横版 2.5D 首要优化点是 Filter 还是带宽

草稿可附在 [05_DragonOath实践计划.md](05_DragonOath实践计划.md) 末尾。

## 下一篇

[05_DragonOath实践计划.md](05_DragonOath实践计划.md)
