# DragonOath 文档索引

本目录保存 DragonOath 项目的正式开发文档。当前项目处于空工程阶段，文档目标是先统一方向、边界、技术路线和开发规范，避免后续实现时反复推翻基础结构。

## 正式文档

建议按以下顺序阅读：

1. [00_Project_Brief.md](00_Project_Brief.md)  
   项目定位、目标体验、版权边界、首个可玩版本范围。

2. [01_Development_Standards.md](01_Development_Standards.md)  
   Git、目录、命名、C++/Blueprint、资产、网络、文档维护规范。

3. [02_Technical_Architecture.md](02_Technical_Architecture.md)  
   UE 5.8、GAS、联机架构、Common UI、数据资产、MCP/Toolset 的总体技术设计。

4. [03_Milestone_Roadmap.md](03_Milestone_Roadmap.md)  
   从空项目到第一个可玩版本的阶段拆分和验收标准。

5. [04_Local_Message_Bus.md](04_Local_Message_Bus.md)
   GameplayMessageRouter 本地消息总线的定位、联机边界和使用规范。

6. [06_Combat_Attribute_Design.md](06_Combat_Attribute_Design.md)
   玩家、怪物、防御、元素、命中闪避、GAS AttributeSet 的属性设计草案。

7. [UE58_MCP_Notes.md](UE58_MCP_Notes.md)
   UE 5.8 内置 MCP 与 Toolset 插件说明。

## 学习资料

- [learn/](learn/README.md)  
  专题学习笔记目录（非正式架构约束），当前包含：
  - [ReplicationGraph](learn/ReplicationGraph_Learning/README.md)
  - [Iris 复制系统](learn/Iris_Replication_Learning/README.md)
  - [MassAI](learn/MassAI_Learning/README.md)

## 归档文档

- [LongDouShi_Development_Document.md](LongDouShi_Development_Document.md)  
  早期实验性文档，已不作为正式开发依据。正式文档以本索引列出的文件为准。

## 文档维护规则

- 文档变更应和代码变更一起提交。
- 影响系统设计、目录结构、网络策略、GAS 架构的决定，必须更新对应文档。
- 临时想法可以先写在单独草稿中，但进入开发前必须整理到正式文档。
- 文档中的“当前决定”优先级高于历史讨论和旧实验文档。

## 当前项目状态

- 引擎：Unreal Engine 5.8
- 项目名：DragonOath
- 当前阶段：空 UE 工程 + Spec Kit + Git + 基础文档规范
- 开发策略：联机架构优先，第一阶段不做完整公网服务
- 主要方向：原创 2.5D 横版动作 RPG，内部参考经典横版动作页游体验
