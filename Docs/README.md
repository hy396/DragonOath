# DragonOath 文档索引

本目录保存 DragonOath 项目的正式开发文档。当前项目已搭建基础工程骨架和 GAS 基础，文档目标是统一方向、边界、技术路线和开发规范，避免后续实现时反复推翻基础结构。

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

6. [05_Scripting_Plugin_Comparison.md](05_Scripting_Plugin_Comparison.md)  
   脚本插件选型对比（UnrealCSharp / SkookumScript / Lua 机器）。

7. [06_Combat_Attribute_Design.md](06_Combat_Attribute_Design.md)  
   基于龙斗士原游戏的属性体系设计：一级属性、二级属性、伤害公式、宠物守护神、装备强化。

8. [ProfessionAbilityConfigDesign.md](ProfessionAbilityConfigDesign.md)  
   职业技能配置方案：一次性授予全部技能、用 Level 控制可用性。

9. [UE58_MCP_Notes.md](UE58_MCP_Notes.md)  
   UE 5.8 内置 MCP 与 Toolset 插件说明。

## 学习资料

- [learn/](learn/README.md)  
  专题学习笔记目录（非正式架构约束），当前包含：
  - [ReplicationGraph](learn/ReplicationGraph_Learning/README.md)
  - [Iris 复制系统](learn/Iris_Replication_Learning/README.md)
  - [MassAI](learn/MassAI_Learning/README.md)

## 归档文档

- [LongDouShi_Development_Document.md](LongDouShi_Development_Document.md)  
  早期实验性文档，已不作为正式开发依据。

- [QwQ.md](QwQ.md)  
  早期实验性开发规范草稿，类命名和目录结构与正式规范不一致，已归档。

## 文档维护规则

- 文档变更应和代码变更一起提交。
- 影响系统设计、目录结构、网络策略、GAS 架构的决定，必须更新对应文档。
- 临时想法可以先写在单独草稿中，但进入开发前必须整理到正式文档。
- 文档中的"当前决定"优先级高于历史讨论和旧实验文档。
- 代码注释用中文，文档正文用中文，文档标题用英文编号。

## 当前项目状态

- 引擎：Unreal Engine 5.8（路径 `D:\UE_5.8`）
- 项目名：DragonOath（龙斗士复刻学习项目）
- 当前阶段：M1-M2，基础工程骨架和 GAS 基础已部分完成
- 开发策略：联机架构优先，第一阶段不做完整公网服务
- 主要方向：复刻龙斗士原游戏体验，横版动作 RPG
- 类前缀：统一使用 `DO`（如 `UDOAbilitySystemComponent`、`ADOPlayerCharacter`）
- AI 架构：StateTree 为主线，不使用 Behavior Tree
