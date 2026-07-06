# 03. Filter 与 Prioritizer

## 为什么需要 Filter

在 Iris 中，**不再依赖** `AActor::IsNetRelevantFor` 做主要相关性判断，而是由 **NetObjectFilter** 决定：某个 NetObject 是否进入某连接的复制视图。

可以粗略对应：

| 传统概念 | Iris |
|----------|------|
| 距离剔除 | `Spatial`（`NetObjectGridWorldLocFilter`） |
| Always Relevant | `DynamicFilterName=None` |
| 不复制 | `NotRouted`（`FilterOutNetObjectFilter`） |
| 仅 Owner（传统标志） | 需结合 Bridge 逻辑与专用 Filter / 策略 |

## 引擎默认 Filter 定义

`BaseEngine.ini`：

```ini
[/Script/IrisCore.NetObjectFilterDefinitions]
+NetObjectFilterDefinitions=(FilterName=Spatial, ClassName=/Script/IrisCore.NetObjectGridWorldLocFilter, ...)
+NetObjectFilterDefinitions=(FilterName=NotRouted, ClassName=/Script/IrisCore.FilterOutNetObjectFilter, ...)
```

`ObjectReplicationBridgeConfig`：

```ini
DefaultSpatialFilterName=Spatial
```

未单独配置的、应做空间剔除的类 → 使用 `Spatial`。

## 按类配置 Filter（FilterConfigs）

在项目的 `DefaultEngine.ini` 可覆盖：

```ini
[/Script/IrisCore.ObjectReplicationBridgeConfig]
!FilterConfigs=ClearArray
+FilterConfigs=(ClassName=/Script/Engine.Info, DynamicFilterName=None)
+FilterConfigs=(ClassName=/Script/Engine.LevelScriptActor, DynamicFilterName=NotRouted)
+FilterConfigs=(ClassName=/Script/DragonOath.DOTestAlwaysRelevantActor, DynamicFilterName=None)
```

字段含义：

- `ClassName` — 目标类（可配合 `bIncludeSubclasses`）
- `DynamicFilterName` — 引用 `NetObjectFilterDefinitions` 中的 `FilterName`
  - `None` → **始终相关**
  - `Spatial` → 空间网格 Filter
  - `NotRouted` → 永不复制

## C++ 注册时指定 Filter

`UEngineReplicationBridge` 支持注册参数 `FActorReplicationParams`：

```cpp
FActorReplicationParams Params;
Params.FilterType = FActorReplicationParams::AlwaysRelevant;
// 或 DefaultSpatial、ExplicitFilter + ExplicitDynamicFilterName
```

适合运行时生成的特殊 Actor，而不想写全局 FilterConfig。

## Prioritizer 做什么

Filter 解决「看不看得到」；**Prioritizer** 解决「看得到时，先同步谁、带宽不够裁谁」。

引擎默认：

```ini
[/Script/IrisCore.NetObjectPrioritizerDefinitions]
+NetObjectPrioritizerDefinitions=(PrioritizerName=DefaultPrioritizer, ClassName=/Script/IrisCore.SphereNetObjectPrioritizer, ...)
+NetObjectPrioritizerDefinitions=(PrioritizerName=PlayerStatePrioritizer, ClassName=/Script/IrisCore.NetObjectCountLimiter, ...)
```

按类绑定：

```ini
[/Script/IrisCore.ObjectReplicationBridgeConfig]
+PrioritizerConfigs=(ClassName=/Script/Engine.PlayerState, PrioritizerName=PlayerStatePrioritizer, bForceEnableOnAllInstances=true)
```

`PlayerState` 虽常带 `bAlwaysRelevant`，但引擎用专用 Prioritizer 控制其复制频率与数量。

## 滞后（Hysteresis）

```ini
[/Script/IrisCore.ReplicationFilteringConfig]
bEnableObjectScopeHysteresis=true
DefaultHysteresisFrameCount=6
```

对象在 Filter 边界进出时，可延迟几帧再变更可见性，减少闪烁。横版快速移动时值得观察该参数影响。

## DragonOath 横版场景策略（草案）

| 对象类型 | Filter 建议 | Prioritizer 建议 | 备注 |
|----------|-------------|------------------|------|
| 玩家 Pawn | DefaultSpatial 或自定义 | DefaultPrioritizer | 视横版视野调 Grid |
| 队友 Pawn | Spatial 或队伍 Filter | 提高优先级 | 可能需要自定义 Filter |
| Boss | `None`（始终相关） | 高优先级 | 全场战斗焦点 |
| 远处装饰 Actor | `NotRouted` 或 Spatial | 低 | 减少无效同步 |
| 局部弹幕 | Spatial + 短生命周期 | 中等 | 与 GAS 投射物策略对齐 |
| GameState / PlayerState | 引擎默认 | PlayerStatePrioritizer | 先沿用引擎配置 |

第二期再考虑自定义 `UNetObjectFilter` 子类（例如按 X 轴 band）。

## 实践练习

1. 在测试分支添加 `FilterConfigs`，使某一 Blueprint 测试 Actor 始终相关。
2. PIE 2 客户端，一人在远处，验证另一客户端能否看到该 Actor。
3. 改回 `Spatial`，确认远距离消失。
4. 将结果记入 `05_DragonOath实践计划.md` 状态表。

## 下一篇

[04_与ReplicationGraph的关系.md](04_与ReplicationGraph的关系.md)
