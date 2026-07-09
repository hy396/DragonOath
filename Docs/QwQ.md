# 归档：早期实验性开发规范草稿

本文档是 DragonOath 早期实验性草稿，原内容曾用于粗略探索单模块扁平化目录结构和龙斗士复刻方向。

由于该文档存在以下问题，已不再作为正式开发依据：

- 类命名使用 `ADOHeroCharacter` / `ADOEnemyCharacter`，与正式规范的 `ADOPlayerCharacter` / `ADOCharacter` 不一致。
- 目录结构使用 `Core/`、`Components/`、`AI/`、`Data/` 等分层，与正式规范的功能域目录不一致。
- Build.cs 依赖列表过时，缺少 Setly、GameplayTasks 等实际已使用的模块。
- 提到 `GetAbilityInputID()` 接口，该接口在基类精简重构中已移除，改为直接使用 GAS 原生 `InputPressed` / `AbilitySpecInputPressed`。
- GameplayTag 命名使用 `Input.Attack` 等旧格式，实际代码使用 `InputTag.Ability.*`。
- 多处提及"面试加分"等求职导向描述，不属于正式技术规范。
- AttributeSet 使用 `DOAttributeSet` 单一集合，实际代码已拆分为 `DOHealthSet` 和 `DOPlaySet`。

正式开发请以以下文档为准：

1. [01_Development_Standards.md](01_Development_Standards.md) — 正式开发规范
2. [02_Technical_Architecture.md](02_Technical_Architecture.md) — 技术架构
3. [03_Milestone_Roadmap.md](03_Milestone_Roadmap.md) — 里程碑路线图
4. [06_Combat_Attribute_Design.md](06_Combat_Attribute_Design.md) — 战斗属性设计
5. [ProfessionAbilityConfigDesign.md](ProfessionAbilityConfigDesign.md) — 职业技能配置方案

## 保留原因

保留这个文件名，是为了记录项目早期讨论来源，避免后续看到旧链接时误以为文件丢失。

## 当前结论

DragonOath 的正式方向是：

```text
龙斗士复刻学习项目
UE 5.8
GAS 核心战斗
联机架构优先
统一 DO 类前缀
StateTree AI 架构
Common UI 逐步接入
```
