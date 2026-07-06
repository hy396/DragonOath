# 11. Setly 插件全局地图

这篇是 `Setly` 的总览图，用来回答三个问题：

1. 这个插件到底管什么。
2. 每个目录和模块应该去哪里看。
3. 后续开发时应该从哪条流程切入。

`Setly` 当前不是单纯的“设置菜单插件”。它更像是从 Lyra 体系拆出来的一套前端基础插件，里面同时包含用户设置、CommonUI 前端框架、输入映射、音频混音、性能选项、本地玩家设置存取、Editor Project Settings 注册，以及一批配套 UI/Audio/Input 资产。

## 插件模块

插件描述文件：

```text
Plugins/Setly/Setly.uplugin
```

运行时模块：

```text
Source/Setly
```

编辑器模块：

```text
Source/SetlyEditor
```

`Setly.uplugin` 中启用了这些关键依赖：

```text
EnhancedInput
GameplayAbilities
DataRegistry
CommonUI
GameSettings
CommonGame
CommonLoadingScreen
ControlFlows
AudioModulation
AudioGameplay
GameSubtitles
UIExtension
```

理解依赖的意义很重要：

| 依赖 | Setly 中的用途 |
| --- | --- |
| `EnhancedInput` | 输入 Action、Mapping Context、按键重绑定 |
| `CommonUI` | 前端界面、Activatable Widget、输入图标、UI 层级 |
| `GameSettings` | 设置注册、设置项数据、设置面板 |
| `CommonGame` | LocalPlayer、Messaging、UI Manager 基础设施 |
| `CommonLoadingScreen` | 加载界面显示状态、加载时音频混音 |
| `AudioModulation` | 音量控制总线、用户音量设置 |
| `UIExtension` | 通过 GameplayTag 把 Widget 注入到界面槽位 |
| `GameSubtitles` | 字幕显示设置 |

## Source 目录地图

```text
Source/Setly
  Audio
  Data
  Development
  Input
  Performance
  Player
  Setly
  Settings
  UI
```

| 目录 | 负责内容 | 优先阅读文件 |
| --- | --- | --- |
| `Audio` | 音量控制总线、加载界面混音、HDR/LDR Submix 链 | `LyraAudioSettings.h`, `LyraAudioMixEffectsSubsystem.h` |
| `Data` | UIExtension 使用的数据结构和 DataAsset | `LyraUIExData.h` |
| `Development` | PIE 调试、平台模拟、开发者设置 | `LyraDeveloperSettings.h`, `LyraPlatformEmulationSettings.h` |
| `Input` | 输入配置、GameplayTag 绑定、可重映射按键、输入修饰器 | `LyraInputConfig.h`, `LyraInputComponent.h`, `LyraInputModifiers.h` |
| `Performance` | 帧率、画质、性能统计、内存调试命令 | `LyraPerformanceSettings.h`, `LyraPerformanceStatSubsystem.h` |
| `Player` | 本地玩家、Local/Shared 设置生命周期 | `LyraLocalPlayer.h` |
| `Setly` | 项目前端 PlayerController、前端 UI 组件、UIExtension 组件 | `SetlyPlayerController.h`, `SetlyForntUIComponent.h`, `SetlyUIExtensionComponent.h` |
| `Settings` | GameSettings 注册、Local/Shared 设置、各类自定义设置项 | `LyraGameSettingRegistry.cpp`, `LyraSettingsLocal.h`, `LyraSettingsShared.h` |
| `UI` | CommonUI 基础控件、HUD、设置屏幕、消息弹窗、加载界面、性能统计 UI | `LyraSettingScreen.h`, `LyraHUDLayout.h`, `LyraUIMessaging.h` |

编辑器模块：

| 目录 | 负责内容 |
| --- | --- |
| `Source/SetlyEditor` | 把 `LyraUIManagerSubsystem` 和 `LyraUIMessaging` 注册到 Project Settings |

## Content 目录地图

```text
Content
  Audio
  Dome
  Input
  UI
  __ExternalActors__
```

| 目录 | 用途 |
| --- | --- |
| `Content/Audio` | SoundClass、ControlBus、ControlBusMix、MetaSound、Submix、音乐和音效 |
| `Content/Input` | 输入 Action 等 Enhanced Input 资产 |
| `Content/UI` | 前端、HUD、设置界面、基础控件、字体、平台输入图标 |
| `Content/Dome` | 测试地图或示例地图资源 |
| `Content/__ExternalActors__` | World Partition 外部 Actor 数据，不手动移动 |

## 全流程心智模型

`Setly` 的主要运行链路可以这样理解：

```text
项目启动
  -> Setly Runtime 模块加载
  -> LocalPlayer 创建
  -> Local/Shared Settings 初始化或读取
  -> PlayerController 注册默认输入映射
  -> UI Manager 创建 CommonUI Root Layout
  -> Setting Screen 请求 GameSettingRegistry
  -> Audio/Input/Video/Gameplay 设置项被注册到 UI
  -> 用户修改设置
  -> Apply/Save 写入 Config 或 SaveGame
  -> Audio/Input/Performance/UI 子系统立即应用变更
```

编辑器相关链路：

```text
Editor 启动
  -> SetlyEditor 模块加载
  -> 注册 Project Settings 页面
  -> 可在 Project Settings 中配置 Setly UI Manager 和 UI Messaging
```

## 学习路线

第一次学习建议按这个顺序：

```text
11 插件地图
01 设置系统总览
03 UI 绑定流程
14 UI 框架
15 Player/前端流程
12 Audio
13 Input
16 Performance/Development
17 Content 资产地图
18 维护手册
```

为什么这样读：

1. 先知道插件边界，避免只盯着语言设置。
2. 再看设置系统，因为它是最容易观察 UI 数据流的入口。
3. 然后看 UI/Player，因为设置界面必须挂在前端框架上。
4. 最后看 Audio/Input/Performance/Content，它们是设置系统背后真正被改变的业务模块。

## 开发入口选择

新增一个普通设置：

```text
Settings -> GameSettingRegistry -> LyraSettingsLocal 或 LyraSettingsShared -> UI VisualData
```

新增音量分类：

```text
Audio -> LyraAudioSettings -> ControlBus/ControlBusMix -> LyraSettingsLocal -> Registry_Audio
```

新增按键或输入行为：

```text
Input -> InputAction/MappingContext -> LyraInputConfig -> LyraInputComponent -> Settings key binding
```

新增前端页面：

```text
UI -> CommonActivatableWidget -> UIExtension 或 HUD Layout -> Content/UI
```

新增性能统计：

```text
Performance -> LyraPerformanceStatTypes -> LyraPerformanceStatSubsystem -> PerfStats UI -> Settings registry
```

## 维护原则

维护 `Setly` 时优先按“所有权”定位问题：

| 问题现象 | 先看哪里 |
| --- | --- |
| 设置项没有出现在 UI | `Settings` 和 `Content/UI/Settings/GameSettingRegistryVisuals.uasset` |
| Apply 后没有保存 | `LyraSettingsLocal` 或 `LyraSettingsShared` |
| 音量滑条无效 | `Audio` 设置、ControlBus 资产、`LyraGameSettingRegistry_Audio.cpp` |
| 按键绑定无效 | `Input` 资产、Mapping Context、`LyraInputComponent` |
| 菜单或弹窗打不开 | `UI`、`SetlyUIExtensionComponent`、Project Settings |
| 性能选项不符合平台 | `Performance`、DeviceProfile、Platform Emulation |
| PIE 和打包行为不一致 | `Development`、Config、平台 ini |

