# CommonUser 中文说明

## 定位

`CommonUser` 是 Lyra 的用户、平台和在线会话辅助插件。

它封装本地用户初始化、账号/权限检查、会话创建与加入、基础在线状态等流程。

## DragonOath 使用建议

- 当前先作为 `CommonGame` 依赖纳入。
- 后续做登录、角色选择、联机大厅、房间或匹配时再深入接入。
- Listen Server 测试阶段不需要急着使用完整 CommonUser 流程。

## 注意

- 它会接触 OnlineSubsystem / OnlineServices。
- 不要在早期战斗原型中强行引入完整账号登录流程。
- 正式联机服务方案确定前，CommonUser 先保持基础设施状态。
