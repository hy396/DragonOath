# ReplicationGraph 学习资料

本目录存放 DragonOath 项目中关于 **ReplicationGraph** 的学习笔记与实践记录。

## 适用背景

- 引擎：Unreal Engine 5.8
- 项目路径：`D:\ue_texiao\DragonOath`
- 项目策略：联机架构优先（见 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)）

DragonOath 当前尚未接入 ReplicationGraph。本目录用于系统学习、动手实验，并在合适阶段将结论沉淀到正式架构文档。

相关专题：
- [Iris 复制系统](../Iris_Replication_Learning/README.md) — UE 新一代复制架构（见 Iris [04_与ReplicationGraph的关系.md](../Iris_Replication_Learning/04_与ReplicationGraph的关系.md)）
- [MassAI](../MassAI_Learning/README.md) — 大规模实体 AI（与复制专题正交，怪物规模化时阅读）

## 建议阅读顺序

| 序号 | 文档 | 内容 |
|------|------|------|
| 00 | [00_学习路线.md](00_学习路线.md) | 学习目标、阶段划分、实践清单 |
| 01 | [01_核心概念.md](01_核心概念.md) | 为什么需要 RepGraph、节点模型、与默认复制的差异 |
| 02 | [02_引擎内置示例.md](02_引擎内置示例.md) | `BasicReplicationGraph` 源码导读与 ini 启用方式 |
| 03 | [03_DragonOath实践计划.md](03_DragonOath实践计划.md) | 在本项目中落地 RepGraph 的步骤与验收标准 |

> 后续可继续补充：`04_调试与控制台命令.md`、`05_横版2.5D场景策略.md`、`Practice/` 下的实验记录等。

## 引擎参考位置（UE 5.8）

```text
D:\UE_5.8\Engine\Plugins\Runtime\ReplicationGraph\
  Source\Public\ReplicationGraph.h          # 核心 API 与类注释
  Source\Public\BasicReplicationGraph.h     # 最小可运行示例
  Source\Private\BasicReplicationGraph.cpp
```

## 文档维护约定

- 学习笔记可以写得详细；进入正式开发前，将「已采纳的决定」同步到 `Docs/02_Technical_Architecture.md`。
- 每次实践在对应 `.md` 中记录：日期、改动文件、验证方式、结论。
- 与代码实验配套的 C++ 模块建议放在独立 Feature 分支，避免污染主开发线。
