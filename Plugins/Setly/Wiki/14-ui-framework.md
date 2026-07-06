# 14. UI 框架

这篇解释 `Setly` 的 CommonUI 前端框架、HUD、弹窗、UIExtension、加载界面和设置屏幕如何串起来。

## 核心文件

```text
Source/Setly/UI/LyraActivatableWidget.h
Source/Setly/UI/LyraHUD.h
Source/Setly/UI/LyraHUDLayout.h
Source/Setly/UI/LyraSettingScreen.h
Source/Setly/UI/Subsystem/LyraUIManagerSubsystem.h
Source/Setly/UI/Subsystem/LyraUIMessaging.h
Source/Setly/UI/Foundation/LyraLoadingScreenSubsystem.h
Source/Setly/UI/Foundation/LyraConfirmationScreen.h
Source/Setly/UI/Common
Source/Setly/UI/PerformanceStats
Source/Setly/Data/LyraUIExData.h
Source/Setly/Setly/SetlyUIExtensionComponent.h
Content/UI
```

## UI 分层模型

`Setly` 使用 CommonUI 的 Layer/Activatable Widget 思路：

```text
Root Layout
  -> HUD Layer
  -> Menu Layer
  -> Modal/Dialog Layer
  -> Loading/Overlay Layer
```

实际项目里 Layer 通常由 GameplayTag 或 UI Policy 资产管理。`Setly` 侧提供 C++ 基类和注册入口，具体 Widget 多数在 `Content/UI` 中。

## UI Manager

```text
ULyraUIManagerSubsystem
```

继承自：

```text
UGameUIManagerSubsystem
```

它的职责：

1. 初始化游戏 UI 管理器。
2. 使用 Tick 同步 Root Layout 可见性。
3. 根据 HUD 显示状态控制根布局。

编辑器模块会把它注册到 Project Settings：

```text
Source/SetlyEditor/Private/SetlyEditor.cpp
```

Project Settings 页面名称：

```text
Setly UI Manager
```

## UI Messaging

```text
ULyraUIMessaging
```

继承自：

```text
UCommonMessagingSubsystem
```

它负责统一弹出：

| 函数 | 用途 |
| --- | --- |
| `ShowConfirmation` | 确认弹窗 |
| `ShowError` | 错误弹窗 |

配置字段：

```text
ConfirmationDialogClass
ErrorDialogClass
```

编辑器模块同样会把它注册到 Project Settings：

```text
Setly UI Messaging
```

维护弹窗时优先检查 Project Settings 中的软类引用是否正确。

## HUD Layout

```text
ULyraHUDLayout
```

这是 HUD 布局基类，继承 `ULyraActivatableWidget`。它的核心点：

| 成员 | 作用 |
| --- | --- |
| `EscapeMenuClass` | 按 Escape 时推入的菜单 Widget |
| `HandleEscapeAction` | 处理 Escape 菜单打开逻辑 |

如果 Escape 菜单打不开，优先检查：

```text
EscapeMenuClass
CommonUI 输入 Action
当前 Root Layout 是否存在
```

## UIExtension 数据

```text
ULyraUIExData
```

这是给 UIExtension 使用的 DataAsset，包含几类注册数据：

| 结构 | 用途 |
| --- | --- |
| `FContentToLayerWidgetEntry` | 把 Widget 推入某个 Layer |
| `FRegisterExtensionAsWidgetEntry` | 在某个 Slot 注册 Widget |
| `FRegisterExtensionAsWidgetForContextEntry` | 带 Context 注册 Widget |
| `FRegisterExtensionPointEntry` | 注册扩展点 |
| `FRegisterExtensionPointContextEntry` | 带 Context 注册扩展点 |
| `FRegisterExtensionAsDataEntry` | 注册数据扩展 |

组件入口：

```text
USetlyUIExtensionComponent
```

它有两个重要数组：

```text
Layouts
RegisterWidgets
```

运行时会保存：

```text
ExtensionHandles
PushedWidgets
```

也就是说，UI 注入必须在 EndPlay 或对象销毁时释放 Handle，否则容易出现重复注册、旧 Widget 残留等问题。

## 设置界面

设置界面入口：

```text
Source/Setly/UI/LyraSettingScreen.h
Source/Setly/UI/LyraSettingScreen.cpp
```

设置数据来自：

```text
Source/Setly/Settings/LyraGameSettingRegistry.cpp
Source/Setly/Settings/LyraGameSettingRegistry_*.cpp
```

UI 列表和 Entry 还依赖 `GameSettings` 插件。详细设置流程看：

```text
01-settings-overview.md
03-ui-binding-flow.md
07-visual-data-mapping.md
```

## 基础控件

`Source/Setly/UI/Common` 和 `Source/Setly/UI/Foundation` 提供了一批可复用控件：

| 文件/目录 | 作用 |
| --- | --- |
| `LyraButtonBase` | 通用按钮基类 |
| `LyraActionWidget` | 输入动作显示 |
| `LyraBoundActionButton` | 绑定输入动作的按钮 |
| `LyraTabButtonBase` | Tab 按钮 |
| `LyraTabListWidgetBase` | Tab 列表 |
| `LyraListView` | 列表封装 |
| `LyraWidgetFactory` | Widget 工厂 |
| `LyraConfirmationScreen` | 确认弹窗 |
| `LyraControllerDisconnectedScreen` | 手柄断连提示 |

新增 UI 时优先复用这些基类，避免每个 Widget 自己处理输入、焦点、激活栈。

## 加载界面内容

```text
ULyraLoadingScreenSubsystem
```

它是 `UGameInstanceSubsystem`，只保存当前加载界面内容 Widget Class：

```text
SetLoadingScreenContentWidget
GetLoadingScreenContentWidget
```

它不负责判断“什么时候显示加载界面”，这个由 `CommonLoadingScreen` 负责。它负责告诉加载界面里应该显示哪个内容 Widget。

## 性能统计 UI

目录：

```text
Source/Setly/UI/PerformanceStats
Content/UI/PerfStats
```

核心控件：

| 类 | 作用 |
| --- | --- |
| `LyraPerfStatContainerBase` | 性能统计容器 |
| `LyraPerfStatWidgetBase` | 单个性能统计项 |
| `LyraPerfStatGraph` | 曲线显示 |

数据来自：

```text
ULyraPerformanceStatSubsystem
```

## 新增一个前端页面

1. 在 `Content/UI` 下创建 Widget Blueprint。
2. 继承合适的 C++ 基类，例如 `LyraActivatableWidget`。
3. 如果页面要进菜单层，配置到 UI Layer 或通过 `USetlyUIExtensionComponent` 推入。
4. 如果需要弹窗，走 `ULyraUIMessaging`。
5. 如果需要设置数据，接入 `LyraSettingScreen` 或 GameSettings Registry。
6. 检查键鼠、手柄、返回键、焦点默认位置。
7. 检查退出页面时是否从 Activatable Stack 中移除。

## 常见问题

| 现象 | 可能原因 |
| --- | --- |
| Widget 创建了但看不到 | 没有 Root Layout、Layer Tag 错、没推入正确层 |
| 弹窗打不开 | Project Settings 中 DialogClass 为空或软引用未加载 |
| Escape 没反应 | HUD Layout 没绑定输入或 `EscapeMenuClass` 没配置 |
| 手柄焦点乱跳 | Widget 没设置默认焦点或 CommonUI 导航配置不完整 |
| UIExtension 重复出现 | Extension Handle 没释放或组件 BeginPlay 多次注册 |
| 打包后图标/字体丢失 | 软引用资产没有被烘焙 |

## 维护检查

修改 UI 后至少检查：

1. 鼠标、键盘、手柄都能操作。
2. 打开、返回、关闭流程没有卡住输入。
3. 弹窗、菜单、设置界面不会互相遮挡。
4. UIExtension 注册和卸载成对出现。
5. PIE、Standalone、打包环境使用同一套 Project Settings 配置。
6. 新增 UI 资产要记录到 `17-content-assets-map.md`。

