# CommonLoadingScreen 中文说明

## 定位

`CommonLoadingScreen` 是 Lyra 的加载屏管理插件。

它提供加载屏管理器、启动加载屏和 `LoadingProcessInterface`，可以让多个系统声明“当前还需要显示加载屏”。

## DragonOath 使用建议

- 副本切换、地图加载、角色登录进入场景时使用。
- M1/M2 可以先只确认插件可编译。
- M6 副本流程阶段再接入实际加载屏 Widget。

## 注意

- 它不负责真正加载地图，只负责加载期间的显示和状态聚合。
- 加载屏 Widget 样式应由 DragonOath 自己实现。
- 服务器逻辑和客户端加载表现要分开处理。
