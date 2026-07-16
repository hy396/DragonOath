# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目速览

- **DragonOath（龙契战纪）**：UE 5.8 原创 2.5D 横版动作 RPG，联机架构优先
- **代码位置**：`D:\ue_texiao\DragonOath`
- **引擎路径**：`D:\UE_5.8`
- **C++ 命名约定**：项目类前缀统一 `DO`（如 `UDOAbilitySystemComponent`、`ADOPlayerCharacter`）

## 上下文入口（不要重复读，先看这两个）

1. **`AGENTS.md`** — AI Agent 快速上下文（构建命令、源码结构、ASC 放置、GAS 约定、命名、AI 注意事项）。**Claude 每次开工应先读这个文件**。
2. **`Docs/README.md`** — 正式架构文档索引（开发规范、技术架构、消息总线、属性设计等）。

只在 AGENTS.md / Docs 没有覆盖当前任务时，再去读具体 .cpp / .h。

## 构建命令（Windows / PowerShell）

```powershell
# 重新生成项目文件（修改 .uplugin / Build.cs / 新增 .cpp 后必须执行）
& "D:\UE_5.8\Engine\Build\ProjectFiles\ProjectFileGenerator.exe" "D:\ue_texiao\DragonOath\DragonOath.uproject"

# 编译 Development Editor
& "D:\UE_5.8\Engine\Build\Build\Build.exe" DragonOathEditor Win64 Development -Project="D:\ue_texiao\DragonOath\DragonOath.uproject" -WaitMutex

# 启动编辑器
& "D:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "D:\ue_texiao\DragonOath\DragonOath.uproject"
```

只改 .cpp 可用 Live Coding 热重载；改 .h 或 Build.cs 必须重编译。

## 关键架构（GAS 三层职责）

代码生成时必须遵守的 Lyra 风格分层（详见 `Source/DragonOath/Components/DOHealthComponent.h` 顶部注释）：

| 层 | 职责 | 代表 |
|---|---|---|
| **数值层** | Attribute Meta 转换、`bOutOfHealth` 幂等 | `UDOHealthSet::PostGameplayEffectExecute` |
| **行为层** | 死亡状态机 `EDODeathState`、Status Tag 应用、FDOVerbMessage 广播、蓝图委托 | `UDOHealthComponent` |
| **流程层**（未来） | 死亡 Montage、Ragdoll、Respawn | `UGameplayAbility_Death`（蓝图技能） |

**通用规则**：
- `AttributeSet` 不做死亡判定 / 事件广播 / Tag 应用 —— 只做数值。
- `Component`（继承 UE 5.8 `ModularGameplay` 插件的 `UGameFrameworkComponent`）做状态机 + 事件桥接。
- **死亡 / 击杀 / 伤害事件**全部走 `FDOVerbMessage` 通用币广播（`Source/DragonOath/Messages/`），不直接监听 GameplayEvent。

## ASC 放置（影响所有 GAS 相关修改）

- **玩家**：`UDOAbilitySystemComponent` 挂在 `ADOPlayerState`，Avatar → `ADOPlayerCharacter`；`Mixed` replication。
- **怪物 / NPC**：ASC 挂 `ADOCharacter` 自身。
- **玩家 `UDOHealthComponent`** 挂在 `ADOPlayerState`（Pawn 销毁不影响）；**怪物** 挂在 `ADOCharacter`。
- 注入时机：`ADOCharacter::InitializeAbilitySystemComponent` 末尾，`InitAbilityActorInfo` 之后。

## 不要做的事（高频踩坑）

1. **不创建顶层 `Public/Private/`** —— 按功能域放在 `Source/DragonOath/<Domain>/`。
2. **不用 Behavior Tree** —— 主线是 StateTree，后期引入 Mass。
3. **不用枚举做职业标识** —— 用 GameplayTag（`Profession.*`）。
4. **不在 Blueprint 写服务器权威逻辑** —— 战斗结果由 C++ / GAS 流程决定。
5. **不用 `FGameplayTag::RequestGameplayTag` 字符串** —— 引用 `DOGameplayTag.h` 中的变量。
6. **不用 GE 施加技能激活期临时状态** —— 用 `ActivationOwnedTags`；**不在 `CanActivateAbility` 手动查状态标签** —— 用 `ActivationBlockedTags`。
7. **不直接编辑 `Binaries/`、`Intermediate/`、`Saved/`、`DerivedDataCache/`**。
8. **不引入全新 log category** —— 沿用 `LogDragonOath`（`DOLogChannels.h`）。
9. **代码注释用中文**，`UE_DEFINE_GAMEPLAY_TAG_COMMENT` 第三参数也用中文。
10. **C++ 侧完成蓝图相关改动** → 在 `Docs/蓝图需要做的事情/` 同步新增待办文件。

## 文档维护规则

- 改代码同时改文档；影响架构 / 目录 / 网络策略的决定必须更新对应 `Docs/*.md`。
- 临时想法写到单独草稿，进入开发前必须整理到正式文档。
- 当前决定优先级 > 历史讨论 > 旧实验文档（如 `QwQ.md` / `LongDouShi_Development_Document.md` 已归档）。

## Spec Kit

工具链生成的设计文档（`spec.md` / `plan.md` / `tasks.md`）如存在，应放在 `.specify/` 目录下。当前未启用 Spec Kit 工作流。

## 已规划的扩展任务（优先级参考）

- **M2 战斗闭环**：普攻、技能、伤害数字、死亡流程（已完成 `UDOHealthComponent` 三层重构，待 GA_Death 蓝图技能）
- **跨网事件**：在 `ADOPlayerState` 上挂 `FDOVerbMessageReplication`（FastArray），让客户端 RebroadcastMessage 到本地总线
- **协助检测**：`Message.Combat.Assist.Contributed` Tag 已声明但未实现，需独立 `UDOAssistProcessor` 组件
- **复活链路**：`Event.Reset` / `Event.RequestReset` + `HealthComponent::FinishDeath` 退出 Dying/Dead