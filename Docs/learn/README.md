# 学习资料索引

本目录存放 DragonOath 的**专题学习笔记**（非正式架构约束）。正式开发规范仍以 `Docs/` 根目录下的编号文档为准。

## 专题列表

| 专题 | 目录 | 说明 |
|------|------|------|
| ReplicationGraph | [ReplicationGraph_Learning/](ReplicationGraph_Learning/README.md) | 传统复制驱动下的图节点模型，适合理解大规模 Actor 相关性优化 |
| Iris 复制系统 | [Iris_Replication_Learning/](Iris_Replication_Learning/README.md) | UE 新一代复制架构：Bridge、Filter、Prioritizer、NetSerializer |
| StateTree | [StateTree_Learning/](StateTree_Learning/README.md) | 层次化状态机：State/Transition/Condition/Task/Evaluator，怪物 AI 核心 |
| MassAI | [MassAI_Learning/](MassAI_Learning/README.md) | 大规模实体 AI：ECS 式 Mass Entity + StateTree + 导航 |
| GAS | [GAS_Learning/](GAS_Learning/README.md) | UE 5.8 Gameplay Ability System 基础、输入链路、项目 Ability 基类边界 |

## 建议学习顺序

### 联机方向

1. 先掌握 UE 联机基础（Server/Client、`AActor` 复制、`NetDriver`）
2. **Iris** — Epic 主推的下一代复制路径（UE 5.8 仍可与传统系统并存）
3. **ReplicationGraph** — 在传统 Generic 复制下做相关性扩展；与 Iris 是不同层级的方案

两者对比见 [Iris_Replication_Learning/04_与ReplicationGraph的关系.md](Iris_Replication_Learning/04_与ReplicationGraph的关系.md)。

### 怪物 / AI 方向

1. **StateTree** — 先掌握层次化状态机：State/Transition/Condition/Task/Evaluator，这是所有怪物 AI 的基础行为编排工具
2. 然后实践 `EnemyCharacter + GAS + StateTree`（项目 M5 路线）；见 [StateTree_Learning/05_DragonOath实践计划.md](StateTree_Learning/05_DragonOath实践计划.md)
3. **MassAI** — 同屏大量杂兵、刷怪潮时再引入；见 [MassAI_Learning/05_与Actor怪物的边界.md](MassAI_Learning/05_与Actor怪物的边界.md)
4. Mass 联机复制与 Iris 可对照阅读，但分属不同子系统

## 维护约定

- 学习笔记可详细；**已采纳的技术决定**需同步到 [02_Technical_Architecture.md](../02_Technical_Architecture.md)
- 实验代码建议放在独立 feature 分支
- 每次实践在对应专题目录记录：日期、配置、验证方式、结论
