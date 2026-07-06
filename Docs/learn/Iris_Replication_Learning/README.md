# Iris 复制系统学习资料

本目录存放 DragonOath 项目中关于 **Iris Replication System** 的学习笔记与实践记录。

## 适用背景

- 引擎：Unreal Engine 5.8
- 项目路径：`D:\ue_texiao\DragonOath`
- 项目策略：联机架构优先（见 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)）

Iris 是 Epic 正在推进的**新一代复制系统**，与传统基于 Actor Channel 的 Generic 复制并存。DragonOath 当前尚未启用 Iris；本目录用于系统学习、对比实验，并在合适阶段将结论沉淀到正式架构文档。

相关专题：[MassAI](../MassAI_Learning/README.md)（Mass 实体复制走 `MassReplication`，与 Actor/Iris 路径并存时需统一权威策略）。

## 建议阅读顺序

| 序号 | 文档 | 内容 |
|------|------|------|
| 00 | [00_学习路线.md](00_学习路线.md) | 学习目标、阶段划分、实践清单 |
| 01 | [01_核心概念.md](01_核心概念.md) | ReplicationSystem、Bridge、NetObject、与传统复制的差异 |
| 02 | [02_引擎配置与启用.md](02_引擎配置与启用.md) | CVar、ini、`IrisNetDriverConfigs`、命令行启用 |
| 03 | [03_Filter与Prioritizer.md](03_Filter与Prioritizer.md) | 空间 Filter、Always Relevant、优先级与带宽控制 |
| 04 | [04_与ReplicationGraph的关系.md](04_与ReplicationGraph的关系.md) | Iris vs RepGraph vs Generic，选型建议 |
| 05 | [05_DragonOath实践计划.md](05_DragonOath实践计划.md) | 在本项目中启用 Iris 的步骤与验收标准 |

> 后续可继续补充：`06_PushModel与NetSerializer.md`、`07_调试与控制台命令.md`、`Practice/` 实验记录等。

## 引擎参考位置（UE 5.8）

```text
D:\UE_5.8\Engine\Source\Runtime\Net\Iris\
  Public\Iris\ReplicationSystem\ReplicationSystem.h   # 核心复制系统
  Public\Iris\IrisConfig.h                            # ShouldUseIrisReplication 等 API

D:\UE_5.8\Engine\Source\Runtime\Engine\
  Public\Net\Iris\ReplicationSystem\EngineReplicationBridge.h   # 引擎桥接层

D:\UE_5.8\Engine\Config\BaseEngine.ini
  ; Iris 默认 Filter / Prioritizer / Bridge 配置段（约 1479 行起）

D:\UE_5.8\Engine\Plugins\Experimental\Iris\
  Iris.uplugin   # 实验插件（IrisCore 已内置在引擎模块中）
```

## 文档维护约定

- 学习笔记可以写得详细；进入正式开发前，将「已采纳的决定」同步到 `02_Technical_Architecture.md`。
- 每次实践在对应 `.md` 中记录：日期、改动文件、验证方式、结论。
- Iris 仍为演进中的系统，以本机 UE 5.8 源码与实验结果为准，勿仅依赖旧版网络文档。
