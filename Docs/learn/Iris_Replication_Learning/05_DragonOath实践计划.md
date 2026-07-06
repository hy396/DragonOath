# 05. DragonOath 实践计划

在 DragonOath 中启用 Iris 的推荐步骤。建议在**独立 feature 分支**实验，避免影响主开发线。

## 目标

- 通过 ini + CVar 让 `GameNetDriver` 使用 Iris
- PIE 多人验证复制正常
- 配置至少一条项目级 `FilterConfigs`
- 记录与 Generic 复制的差异，为架构文档提供依据

## 步骤 1：确认引擎能力

UE 5.8 已包含 `IrisCore`，无需额外下载。确认本机引擎路径：

```text
D:\UE_5.8\Engine\Source\Runtime\Net\Iris\
```

## 步骤 2：配置 DefaultEngine.ini

在 `Config/DefaultEngine.ini` 追加（实验分支）：

```ini
[/Script/Engine.Engine]
+IrisNetDriverConfigs=(NetDriverDefinition=GameNetDriver, bCanUseIris=true)

[ConsoleVariables]
net.Iris.UseIrisReplication=1
```

Dedicated Server 若用独立 ini，同步修改。

## 步骤 3：项目级 Filter 配置（示例）

```ini
[/Script/IrisCore.ObjectReplicationBridgeConfig]
; 在引擎默认基础上追加；若需全量覆盖先用 !FilterConfigs=ClearArray
+FilterConfigs=(ClassName=/Script/Engine.Info, DynamicFilterName=None)
; 替换为 DragonOath 测试 Actor 类路径：
; +FilterConfigs=(ClassName=/Script/DragonOath.DOTestAlwaysRelevantActor, DynamicFilterName=None)
```

## 步骤 4：测试 Actor 与地图

与 [ReplicationGraph 实践](../ReplicationGraph_Learning/03_DragonOath实践计划.md) 类似，准备三类对象：

| 类型 | 配置方式 | Iris 下预期 |
|------|----------|-------------|
| A 始终相关 | `FilterConfigs` → `None` 或 `FActorReplicationParams::AlwaysRelevant` | 所有客户端可见 |
| B 空间相关 | 默认 `Spatial` | 远距离不可见 |
| C 不复制 | `DynamicFilterName=NotRouted` | 客户端不应收到 |

## 步骤 5：PIE 验证

1. Play → Multiplayer：2 Players，Listen Server
2. 日志确认 Iris 启用
3. `stat net` 对比 `net.Iris.UseIrisReplication 0` 与 `1`
4. 验证 A/B/C 行为

可选启动参数：`-UseIrisReplication=1`

## 步骤 6：与 ReplicationGraph 实验隔离

- **Iris 实验分支**：只配 Iris 相关 ini，**不要**同时配 `ReplicationDriverClassName`
- **RepGraph 实验分支**：`net.Iris.UseIrisReplication=0` + RepGraph 驱动

便于分清楚哪套系统在起作用。

## 步骤 7：沉淀到正式文档

将采纳结论写入 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)，至少包括：

- DragonOath 首期复制系统：**Iris / Generic / 待定**
- 默认 Filter / Prioritizer 策略
- 横版 2.5D 特殊考虑
- 与 GAS 复制兼容性注意事项

## 验收标准

- [ ] `GameNetDriver` 日志或 API 确认 `IsUsingIrisReplication()==true`
- [ ] Listen Server + 2 Client PIE 无崩溃
- [ ] A/B/C 三类测试对象行为符合预期
- [ ] 有一份 Generic vs Iris 对比实验记录
- [ ] 架构文档已更新或明确「暂不启用」及原因

## 风险与回滚

| 风险 | 缓解 |
|------|------|
| Iris 与部分插件/组件不兼容 | 保持 Generic 回滚路径：`net.Iris.UseIrisReplication=0` |
| Filter 配错导致 Actor 消失 | 先用 `None` 验证链路，再加 `Spatial` |
| 与 RepGraph 实验混淆 | 分支隔离、ini 注释清楚 |

## 状态

| 项目 | 状态 |
|------|------|
| DefaultEngine.ini Iris 配置 | 未开始 |
| FilterConfigs 项目覆盖 | 未开始 |
| PIE 验证 | 未开始 |
| Generic vs Iris 对比记录 | 未开始 |
| 架构文档更新 | 未开始 |

## 选型草稿（待填写）

```markdown
DragonOath 首期复制方案：
- 主路径：Iris / Generic（择一）
- ReplicationGraph：启用 / 仅学习 / 不采用
- 理由：…
```

## 你需要我协助时

可以说：

- 「帮我在 DragonOath 配好 Iris 最小 ini」
- 「帮我写 Iris 测试 Actor 和 FilterConfigs」
- 「帮我做 Generic vs Iris 对比实验」

我会在实验分支改配置/代码，并把结论回写本目录笔记。
