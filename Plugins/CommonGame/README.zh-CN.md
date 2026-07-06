# CommonGame 中文说明

## 定位

`CommonGame` 是 Lyra 的通用游戏 UI 与玩家框架插件。

它包含 `PrimaryGameLayout`、`GameUIManagerSubsystem`、`GameUIPolicy`、Common UI 异步入栈节点、确认弹窗和基础 PlayerController/LocalPlayer 类。

## DragonOath 使用建议

- 后续 Common UI 分层可以参考 `PrimaryGameLayout`。
- 玩家打开背包、设置、技能树等界面时，可以参考 `GameUIManagerSubsystem` 的层级管理方式。
- 确认弹窗和异步推入 UI 页面可以参考 `Actions/` 目录。

## 注意

- 这个插件会牵涉 `CommonUser`、`ModularGameplayActors`、`OnlineFramework` 等依赖。
- 当前先作为基础设施纳入，正式继承它的类之前，需要确认 DragonOath 的 PlayerController、GameInstance、LocalPlayer 规划。
- 不要直接把 Lyra 的 Shooter UI 业务照搬进项目。
