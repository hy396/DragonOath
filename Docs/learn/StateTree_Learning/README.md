# StateTree 学习资料

本目录存放 DragonOath 项目中关于 **StateTree** 的学习笔记与实践记录。

## 适用背景

- 引擎：Unreal Engine 5.8
- 项目路径：`D:\ue_texiao\DragonOath`
- 项目策略：怪物 AI 首期用 `EnemyCharacter + GAS + StateTree`（见 [02_Technical_Architecture.md](../../02_Technical_Architecture.md)）

**StateTree 是 DragonOath 怪物 AI 的核心行为编排工具。** 无论是 M5 少量 Actor 怪物还是 M6+ 的 Mass 杂兵潮，都依赖 StateTree 描述行为逻辑。

## 建议阅读顺序

| 序号 | 文档 | 内容 |
|------|------|------|
| 00 | [00_学习路线.md](00_学习路线.md) | 学习目标、阶段划分、实践清单 |
| 01 | [01_核心概念.md](01_核心概念.md) | State/Transition/Condition/Task/Evaluator 心智模型 |
| 02 | [02_在Actor上使用.md](02_在Actor上使用.md) | `UStateTreeComponent`、Schema、绑定属性、C++/蓝图集成 |
| 03 | [03_状态树编辑器.md](03_状态树编辑器.md) | 编辑器操作、调试、断点、日志、常见陷阱 |
| 04 | [04_内置Task与Condition.md](04_内置Task与Condition.md) | 引擎内置 Task / Condition / Evaluator 清单与选型 |
| 05 | [05_DragonOath实践计划.md](05_DragonOath实践计划.md) | 在本项目中的使用时机、场景划分与实验步骤 |

> 后续可继续补充：`06_StateTree调试进阶.md`、`07_与GAS的配合.md`、`Practice/` 实验记录等。

## 与其他学习专题的关系

```
                  ┌──────────────────────┐
                  │     StateTree        │ ◄── 本专题
                  │  (行为编排核心)       │
                  └──────┬───────────────┘
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
   ┌──────────┐   ┌──────────┐   ┌──────────┐
   │  Actor   │   │  GAS     │   │  MassAI  │
   │  怪物    │   │ 属性技能  │   │  杂兵潮   │
   └──────────┘   └──────────┘   └──────┬───┘
                                        │
                           [MassAI_Learning/03_StateTree集成.md]
```

- **Actor 路线**：`UStateTreeComponent` 挂在 Actor 上 → 本专题 02
- **Mass 路线**：`UMassStateTreeTrait` + `UMassStateTreeSchema` → [MassAI_Learning/03_StateTree集成.md](../MassAI_Learning/03_StateTree集成.md)
- **GAS 配合**：StateTree Task 通过 `FGameplayTag` 触发 GAS Ability → 本专题 05

## 引擎参考位置（UE 5.8）

```text
D:\UE_5.8\Engine\Plugins\Runtime\StateTree\
  StateTree.uplugin
  Source\StateTreeModule\
    Public\StateTree.h                    # UStateTree 资产类
    Public\StateTreeTypes.h              # EStateTreeRunStatus、Transition 等类型
    Public\StateTreeSchema.h             # Schema 基类
    Public\StateTreeTaskBase.h           # Task 基类（EnterState/Tick/ExitState）
    Public\StateTreeConditionBase.h      # Condition 基类
    Public\StateTreeEvaluatorBase.h      # Evaluator 基类
    Public\StateTreeExecutionContext.h   # 执行上下文
    Public\Conditions\                   # 内置 Condition
    Public\Tasks\                        # 内置 Task
    Public\StateTreeEvents.h            # StateTree 事件系统
  Source\StateTreeEditorModule\          # 编辑器工具
  Source\StateTreeDeveloper\             # 调试与开发者工具
```

## 文档维护约定

- 学习笔记可以写得详细；进入正式开发前，将「已采纳的决定」同步到 `Docs/02_Technical_Architecture.md`。
- 每次实践在对应 `.md` 中记录：日期、改动文件、验证方式、结论。
- StateTree 资产应与 C++ 逻辑分离：行为放在 StateTree 中，系统级逻辑留在 C++。
