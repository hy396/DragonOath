# 03. DragonOath 实践计划

本文档描述在 DragonOath 中接入 ReplicationGraph 的推荐步骤。执行前请确认使用**独立 feature 分支**，避免影响主开发线。

## 目标

- 启用 ReplicationGraph 插件
- 提供 `UDragonOathReplicationGraph`（第一期可继承 `UBasicReplicationGraph`）
- 通过 PIE 多人验证复制行为
- 为横版 2.5D 场景预留自定义 Node 扩展点

## 步骤 1：启用插件

在 `DragonOath.uproject` 的 `Plugins` 数组中添加：

```json
{
  "Name": "ReplicationGraph",
  "Enabled": true
}
```

重新生成项目文件并编译 Editor。

## 步骤 2：模块依赖

在承载游戏逻辑的 C++ 模块（例如未来的 `DragonOath` 或独立 `DragonOathNet` 模块）的 `Build.cs` 中：

```csharp
PrivateDependencyModuleNames.AddRange(new string[] {
    "ReplicationGraph",
    // ...
});
```

若尚无游戏 C++ 模块，可先创建最小 `Source/DragonOath/` 模块，仅包含 ReplicationGraph 子类与空 GameMode 用于测试。

## 步骤 3：实现子类（最小版）

建议第一期直接继承 `UBasicReplicationGraph`，确认链路无误后再重写 `RouteAddNetworkActorToNodes`：

```cpp
// DragonOathReplicationGraph.h（示意，实施时以仓库为准）
UCLASS(transient, config = Engine)
class UDragonOathReplicationGraph : public UBasicReplicationGraph
{
    GENERATED_BODY()
public:
    UDragonOathReplicationGraph();
    // 后续：Override RouteAddNetworkActorToNodes 等
};
```

模块需包含 `ReplicationGraph` 与 `BasicReplicationGraph` 所需头文件路径（`BasicReplicationGraph.h` 在 ReplicationGraph 插件内）。

## 步骤 4：配置 NetDriver

`Config/DefaultEngine.ini`：

```ini
[/Script/OnlineSubsystemUtils.IpNetDriver]
ReplicationDriverClassName="/Script/DragonOath.DragonOathReplicationGraph"
```

> 将 `DragonOath` 替换为实际模块名。若使用 Dedicated Server 专用配置，同步修改 `DefaultServer.ini` 等。

## 步骤 5：测试 Actor 与地图

建议准备三类测试 Actor（可用 C++ 或 Blueprint）：

| 类型 | 复制标志 | 预期 |
|------|----------|------|
| A | `bAlwaysRelevant = true` | 所有客户端始终有通道 |
| B | 默认 + `NetCullDistanceSquared` | 仅近距离客户端相关 |
| C | `bOnlyRelevantToOwner = true` | 仅 Owner 连接相关 |

地图：简单横版直线关卡，便于观察距离剔除。

## 步骤 6：PIE 验证

1. Editor → Play → Multiplayer Options：2 Players，Net Mode = Play As Listen Server
2. 检查客户端是否能看到 A/B/C 的预期表现
3. 控制台：`Net.RepGraph.PrintGraph`、`stat net`

## 步骤 7：横版定制（第二期）

在 Basic 跑通后，考虑：

- **2D Grid**：调整 `GridSpatialization2D` 参数，或自定义 Node 按 X 轴 band 划分
- **房间/段落**：关卡分区切换时批量迁移 Actor 到不同 Node
- **GAS 对齐**：GameplayCue、属性同步频率与 RepGraph 的 `ReplicationPeriodFrame` 协调

将采纳的策略更新到 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)。

## 验收标准

- [ ] 插件启用且 Editor 编译通过
- [ ] Listen Server + 2 Client PIE 无崩溃
- [ ] 三类测试 Actor 行为符合预期
- [ ] `Net.RepGraph.PrintGraph` 能打印出 Grid / AlwaysRelevant 等节点
- [ ] 学习笔记中有一次完整「实验记录」

## 风险与回滚

| 风险 | 缓解 |
|------|------|
| 复制驱动配置错误导致无人复制 | 保留 ini 备份，可快速切回默认驱动（删除或注释 `ReplicationDriverClassName`） |
| 仅 Owner 相关 Actor 在生成时无连接 | 参考 Basic 的 `ActorsWithoutNetConnection` 处理 |
| 与现有 Gameplay 框架冲突 | 第一期仅测试地图，不接入 CommonGame 完整流程 |

## 状态

| 项目 | 状态 |
|------|------|
| 插件启用 | 未开始 |
| C++ 子类 | 未开始 |
| ini 配置 | 未开始 |
| PIE 验证 | 未开始 |

> 完成每项后更新上表，并在 [00_学习路线.md](00_学习路线.md) 勾选对应清单。

## 你需要我协助时

可以说：

- 「帮我在 DragonOath 里实现最小 ReplicationGraph 子类」
- 「帮我写测试 Actor 和 PIE 验证步骤」
- 「帮我设计横版 2.5D 的 Grid 策略」

我会在对应分支改代码，并把结论回写本目录笔记。
