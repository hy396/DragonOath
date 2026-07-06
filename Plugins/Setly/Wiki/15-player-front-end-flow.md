# 15. Player 与前端启动流程

这篇解释 `Setly` 中 LocalPlayer、PlayerController、前端 UI 组件如何串起来。很多设置、输入和 UI 问题，最后都会回到“本地玩家有没有准备好”这个点。

## 核心文件

```text
Source/Setly/Player/LyraLocalPlayer.h
Source/Setly/Player/LyraLocalPlayer.cpp
Source/Setly/Setly/SetlyPlayerController.h
Source/Setly/Setly/SetlyPlayerController.cpp
Source/Setly/Setly/SetlyForntUIComponent.h
Source/Setly/Setly/SetlyForntUIComponent.cpp
Source/Setly/Setly/SetlyUIExtensionComponent.h
Source/Setly/Setly/SetlyUIExtensionComponent.cpp
```

## `ULyraLocalPlayer`

`ULyraLocalPlayer` 继承自：

```text
UCommonLocalPlayer
```

它是玩家本地状态的核心入口，负责：

| 能力 | 说明 |
| --- | --- |
| `GetLocalSettings` | 获取机器本地设置，来自 `UGameUserSettings` |
| `GetSharedSettings` | 获取玩家共享设置，来自 SaveGame |
| `LoadSharedSettingsFromDisk` | 异步读取或创建玩家设置 |
| `SwitchController` | 玩家控制器切换时重新绑定 |
| `OnAudioOutputDeviceChanged` | 音频输出设备变化时处理 |
| `OnPlayerControllerChanged` | PlayerController 变化后更新绑定 |

它持有：

```text
SharedSettings
NetIdForSharedSettings
InputMappingContext
LastBoundPC
```

理解它时要区分两个设置对象：

| 对象 | 保存位置 | 适合放什么 |
| --- | --- | --- |
| `ULyraSettingsLocal` | 本机 Config/GameUserSettings | 分辨率、帧率、画质、音量等机器相关设置 |
| `ULyraSettingsShared` | SaveGame/玩家数据 | 语言、手柄震动、字幕、色盲、按键偏好等跟玩家走的设置 |

## `ASetlyPlayerController`

`ASetlyPlayerController` 继承自：

```text
ACommonPlayerController
```

它在 BeginPlay/EndPlay 中处理默认输入映射：

```text
DefaultInputMappings
```

每个元素是：

```text
FInputMappingContextAndPriority
```

包含：

```text
InputMapping
Priority
bRegisterWithSettings
```

也就是说 PlayerController 是默认游戏输入进入 Enhanced Input 的重要入口。

## `USetlyForntUIComponent`

代码里的类名是 `Fornt`，这里保持工程原名。

它是一个可挂到 Actor 上的前端 UI 组件，负责：

| 成员 | 作用 |
| --- | --- |
| `DefaultInputMappings` | 前端 UI 需要的输入映射 |
| `bForceFeedbackEnabled` | 是否允许震动反馈 |
| `CachedLocalPlayer` | 缓存本地玩家 |
| `IsForceFeedbackEnabled` | 对外查询震动是否启用 |

它适合处理和前端体验有关的输入映射，比如菜单、确认、返回、前端界面专属操作。

## `USetlyUIExtensionComponent`

它是 UI 注入组件，负责通过 UIExtension 系统注册或推入 Widget。

关键字段：

```text
Layouts
RegisterWidgets
PlayerIndex
```

运行时字段：

```text
ExtensionHandles
PushedWidgets
```

它适合做：

1. 给某个前端页面自动挂 Widget。
2. 把 HUD、菜单、提示条注入到指定 Slot。
3. 根据 Actor/体验配置注册 UI。

## 前端启动流程

一个典型流程：

```text
玩家进入游戏或前端地图
  -> ULyraLocalPlayer 创建或复用
  -> Local Settings 加载
  -> Shared Settings 异步加载或创建
  -> ASetlyPlayerController BeginPlay
  -> 注册 DefaultInputMappings
  -> UI Manager 准备 Root Layout
  -> USetlyForntUIComponent 注册前端输入
  -> USetlyUIExtensionComponent 注入 Widget
  -> 玩家可以操作菜单、设置、HUD
```

退出或切图：

```text
EndPlay
  -> 移除输入映射
  -> 注销 UIExtension Handle
  -> 清理 Pushed Widget
  -> 保存需要持久化的设置
```

## 设置和玩家生命周期的关系

设置界面读取当前玩家：

```text
LocalPlayer
  -> GetLocalSettings
  -> GetSharedSettings
```

如果 Shared Settings 还没加载完成，语言、字幕、手柄等设置可能读到临时值。因此需要关注：

```text
LoadSharedSettingsFromDisk
OnSharedSettingsLoaded
```

如果你遇到“刚进游戏设置显示默认值，过一会又变了”，通常就是 Shared Settings 异步加载时机的问题。

## 新增一个前端功能

例如新增“角色选择页面”：

1. 创建 Widget Blueprint，继承合适的 CommonUI 基类。
2. 确认它属于哪个 Layer，例如 Menu 或 Modal。
3. 如果通过组件自动进入界面，添加到 `USetlyUIExtensionComponent::Layouts`。
4. 如果只是在某个 Slot 显示，添加到 `RegisterWidgets`。
5. 如果页面需要输入，创建或复用 Mapping Context。
6. 把 Mapping Context 加入 `USetlyForntUIComponent::DefaultInputMappings`。
7. 页面需要读取设置时，从当前 `ULyraLocalPlayer` 获取 Local/Shared Settings。
8. EndPlay 时确认输入映射和 UIExtension 都被移除。

## 常见问题

| 现象 | 可能原因 |
| --- | --- |
| 前端页面无法操作 | UI 输入 Mapping Context 没注册 |
| 设置读到空对象 | LocalPlayer 没有准备好或 Shared Settings 尚未加载 |
| 切换 Controller 后输入失效 | `SwitchController` 后没有重新绑定 |
| UI 重复出现 | UIExtension 注册没有释放 |
| 震动设置无效 | 使用的是组件本地开关，没同步 Shared Settings |
| 多玩家时 UI 跑到错的屏幕 | `PlayerIndex` 配置错误 |

## 维护检查

修改 Player/FrontEnd 流程后至少检查：

1. PIE 单玩家进入前端能操作。
2. 切图后输入映射没有重复。
3. 打开设置界面能读到正确 Local/Shared 设置。
4. Controller 切换后输入仍然有效。
5. UIExtension 在 EndPlay 后没有残留。
6. 多本地玩家场景下 `PlayerIndex` 没有混用。

