# 06. DragonOath 实践计划

本文档描述 MassAI 在 DragonOath 中的**引入时机**与实验步骤。与 Iris/RepGraph 不同，Mass **不建议在 M5 之前强行落地**。

## 目标

- 在 M6/M8 前后评估 Mass 对刷怪潮的性能收益
- 跑通最小链路：Config → Spawner → Mass StateTree → NavMesh 移动 → 简单表现
- 明确 Mass 与 `EnemyCharacter + GAS` 的分工，更新架构文档

## 引入时机（与里程碑对齐）

| 里程碑 | Mass 相关工作 |
|--------|---------------|
| M5 | **不引入**；专注 Actor 怪 + StateTree |
| M6 | 观察副本刷怪同屏数量；若 &lt;30 且无性能问题，仅做**理论学习** |
| M8 | 若有刷怪潮设计，开实验分支做 Mass 杂兵 POC |
| M8+ | 决定是否将 Mass 纳入正式刷怪潮方案 |

## 步骤 1：实验分支与插件

```json
// DragonOath.uproject（实验分支）
{ "Name": "MassGameplay", "Enabled": true },
{ "Name": "MassAI", "Enabled": true },
{ "Name": "StateTree", "Enabled": true }
```

可选对照：`MassCrowd`、`ZoneGraph`、`SmartObjects`

## 步骤 2：实验关卡

- 复制一份横版测试关卡（或简化长廊）
- 放置 NavMesh Bounds Volume
- **不要**与正式 M5 怪物地图混用，避免干扰首期验收

## 步骤 3：Mass Entity Config

创建 `DA_Mass_TestGrunt_Config`（命名示例）：

| Trait | 作用 |
|-------|------|
| Mass Movement（MassGameplay） | 位置/速度 Fragment |
| Mass Representation | ISM 或简单 Mesh 表现 |
| `UMassStateTreeTrait` | 绑定 `ST_Mass_TestGrunt` |
| Mass LOD（可选） | 远距降频 |

## 步骤 4：StateTree 资产

- Schema：**Mass Behavior**（`UMassStateTreeSchema`）
- 最小行为：Spawn → `MassNavMeshPathFollowTask`（目标=玩家或路径点）

## 步骤 5：AMassSpawner

- Entity Config = 上一步资产
- Count：50～200（压测用）
- 触发：BeginPlay 或关卡事件

## 步骤 6：验证指标

| 指标 | 方式 |
|------|------|
| 是否生成 Entity | Mass 调试 / Spawner 日志 |
| 是否移动 | 观察场景 + NavMesh 路径 |
| CPU | `stat unit`、`stat Mass`（实践时确认有效命令） |
| 对比基线 | 同等数量 Actor 占位 pawn（可选） |

## 步骤 7：与正式怪物共存策略

实验结论应回答：

1. 刷怪潮段落是否采用 Mass？
2. 伤害用策略 A/B/C/D 中哪一种？（见 [05_与Actor怪物的边界.md](05_与Actor怪物的边界.md)）
3. Boss 战是否完全保留 Actor？

将答案写入 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)。

## 验收标准（实验分支）

- [ ] MassGameplay + MassAI 启用且 Editor 编译通过
- [ ] Spawner 生成 ≥50 Entity，可见简单表现
- [ ] Mass StateTree 驱动移动行为
- [ ] 有一份 CPU/同屏数量对比记录（Mass vs 无 Mass）
- [ ] 架构文档已更新或明确「暂不采用」及理由

## 风险与回滚

| 风险 | 缓解 |
|------|------|
| 编译时间大增 | 仅实验分支启用 Mass 插件 |
| 与 GAS 怪物逻辑冲突 | Mass 实验地图隔离 |
| Experimental API 变动 | 学习笔记记录 UE 5.8 具体 API，升级引擎时复查 |
| 联机复杂度 | 首期实验可仅 Listen Server 本地 PIE |

## 状态

| 项目 | 状态 |
|------|------|
| 插件启用 | 未开始 |
| Mass Entity Config | 未开始 |
| Mass StateTree | 未开始 |
| Spawner 压测 | 未开始 |
| 与 Actor 怪分工结论 | 未开始 |
| 架构文档更新 | 未开始 |

## 你需要我协助时

可以说：

- 「帮我在 DragonOath 实验分支启用 MassAI 并做最小 Spawner」
- 「帮我写一个 Mass 杂兵 StateTree 样本」
- 「帮我对比 100 个 Mass Agent vs Actor 的性能」

我会在实验分支改配置/代码，并把结论回写本目录笔记。
