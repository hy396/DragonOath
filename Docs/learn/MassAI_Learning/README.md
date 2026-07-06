# MassAI 学习资料

本目录存放 DragonOath 项目中关于 **Mass Entity + MassAI** 的学习笔记与实践记录。

## 适用背景

- 引擎：Unreal Engine 5.8
- 项目路径：`D:\ue_texiao\DragonOath`
- 项目策略：联机架构优先；怪物首期用 `EnemyCharacter + GAS + StateTree`（见 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)）

**MassAI 不是第一只怪物的默认方案。** 它的价值在于大量实体、刷怪潮、群体移动/感知/表现优化。DragonOath 计划在 M6/M8 性能压力出现后再引入。

## 建议阅读顺序

| 序号 | 文档 | 内容 |
|------|------|------|
| 00 | [00_学习路线.md](00_学习路线.md) | 学习目标、阶段划分、实践清单 |
| 01 | [01_核心概念.md](01_核心概念.md) | Entity/Fragment/Processor/Trait/Query 心智模型 |
| 02 | [02_插件栈与依赖.md](02_插件栈与依赖.md) | MassEntity、MassGameplay、MassAI 模块关系与启用 |
| 03 | [03_StateTree集成.md](03_StateTree集成.md) | `UMassStateTreeTrait`、Schema、Mass 专用 Task |
| 04 | [04_导航与SmartObject.md](04_导航与SmartObject.md) | NavMesh、ZoneGraph、SmartObject 在 MassAI 中的角色 |
| 05 | [05_与Actor怪物的边界.md](05_与Actor怪物的边界.md) | 何时用 Mass、何时保留 `EnemyCharacter` |
| 06 | [06_DragonOath实践计划.md](06_DragonOath实践计划.md) | 在本项目中的引入时机与实验步骤 |

> 后续可继续补充：`07_表现与LOD.md`、`08_联机复制.md`、`Practice/` 实验记录等。

## 引擎参考位置（UE 5.8）

```text
D:\UE_5.8\Engine\Source\Runtime\MassEntity\
  Public\MassEntitySubsystem.h          # 世界级 EntityManager
  Public\MassProcessor.h                # Processor 基类

D:\UE_5.8\Engine\Plugins\Runtime\MassGameplay\
  MassGameplay.uplugin                  # Spawner、Movement、LOD、Representation、Replication

D:\UE_5.8\Engine\Plugins\AI\MassAI\
  MassAI.uplugin                        # AI 行为、导航、StateTree、AI 复制

D:\UE_5.8\Engine\Plugins\AI\MassCrowd\
  MassCrowd.uplugin                     # 人群示例（依赖 MassAI，适合对照学习）
```

## 文档维护约定

- 学习笔记可以写得详细；进入正式开发前，将「已采纳的决定」同步到 `Docs/02_Technical_Architecture.md`。
- 每次实践在对应 `.md` 中记录：日期、改动文件、验证方式、结论。
- Mass 栈插件默认为 **Experimental**，实验建议放在独立 feature 分支。
