# 05. 与 Actor 怪物的边界

## 项目已定路线（摘自架构文档）

DragonOath 在 [02_Technical_Architecture.md](../../02_Technical_Architecture.md) 中已明确：

```text
M5：EnemyCharacter + ASC + StateTree/简单 C++ 状态（少量怪物）
M6/M8：副本刷怪、Boss 闭环
之后：同屏数量或刷怪潮压力出现时，再引入 Mass + StateTree
```

**Mass 的价值在「规模化实体管理」，不是替代普通怪物 Actor 的全部职责。**

## 对比表

| 维度 | EnemyCharacter（Actor） | Mass Entity（MassAI） |
|------|-------------------------|------------------------|
| 定位 | 精英怪、Boss、玩法主体 | 大量杂兵、刷怪潮、背景兵 |
| GAS | 完整 ASC、伤害、Buff、Cue | 需额外桥接；不适合首期主战斗体 |
| 碰撞/受击 | `UCapsuleComponent`、精确打击感 | 常简化或用代理 Actor |
| 动画 | AnimBlueprint、Montage、通知 | Representation + 批量/LOD 动画 |
| AI | StateTree on Actor | Mass StateTree Processor |
| 复制 | 标准 Actor / Iris | MassReplication + MassAIReplication |
| 调试 | 成熟、Per-Actor | Mass 调试工具，批量视角 |
| 开发成本 | 低，符合现有 GAS 管线 | 高，需 Template/Trait/Processor 体系 |
| 性能 | 单个体贵，数量少无碍 | 单个体便宜，数量多优势明显 |

## 推荐分层架构（中后期）

```text
【战斗主体层 — Actor】
Boss / 精英 / 需精确交互的敌人
  -> EnemyCharacter + GAS + StateTree
  -> 完整伤害、硬直、技能、掉落

【规模层 — Mass】
杂兵潮 / 远程小兵 / 装饰敌人
  -> Mass Entity Config + Mass StateTree
  -> 简化伤害（范围检测、统一 GE 事件、或「代理」转 Actor）

【桥接层（可选）】
Mass Entity <-> Actor 转换
  -> 杂兵近身时生成为真实 EnemyCharacter（LOD 升级）
  -> 或 Mass 只负责移动，命中判定由服务器范围扫描
```

桥接层最复杂，**不要在第一期实现**。先证明 Mass 能跑，再谈战斗整合。

## 何时引入 Mass（触发条件）

满足任一即可启动 Mass 实验：

- 同屏怪物 > 30～50 且 CPU 成为瓶颈
- 需要「刷怪潮」玩法段落
- 需要大量背景兵营造战场氛围且不与主角精细交互
- MassRepresentation + LOD 可显著降低动画/渲染开销

不满足时继续用 Actor 方案，避免过早优化。

## 与 GAS 的协作策略（草案）

| 策略 | 说明 | 复杂度 |
|------|------|--------|
| A. Mass 不管战斗 | Mass 只移动；伤害由玩家技能范围检测统一处理 | 低 |
| B. 命中代理 Actor | 每个 Mass 实体关联轻量 HitProxy | 中 |
| C. 升级转 Actor | 进入战斗圈后 Spawn 真 EnemyCharacter | 中高 |
| D. 完整 Mass GAS | 自定义 Fragment 同步 Attribute | 高，不建议首期 |

DragonOath 刷怪潮段落可优先评估 **A 或 C**。

## 与联机的关系

- Mass 复制走 `MassReplication` / `MassAIReplication`，与 [Iris](../Iris_Replication_Learning/README.md) / Actor 复制并存时需统一「谁是权威」
- 大量 Mass 杂兵建议 **服务器权威模拟**，客户端以表现为主
- 细节见后续 `08_联机复制.md`（待写）

## 练习

为 DragonOath 填写下表（可先写学习笔记，不必立刻定案）：

| 对象 | Actor / Mass / 混合 | 理由 |
|------|---------------------|------|
| 第一章普通近战怪 | | |
| 刷怪潮小兵 | | |
| 第一章 Boss | | |
| 背景巡逻兵 | | |

## 下一篇

[06_DragonOath实践计划.md](06_DragonOath实践计划.md)
