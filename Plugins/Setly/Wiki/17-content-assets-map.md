# 17. Content 资产地图

这篇记录 `Setly/Content` 目录下的资产职责。`Setly` 很多功能不是纯 C++，必须依赖资产配置才能跑起来，所以维护时不能只看代码。

## 顶层目录

```text
Plugins/Setly/Content
  Audio
  Dome
  Input
  UI
  __ExternalActors__
```

| 目录 | 维护方式 |
| --- | --- |
| `Audio` | 音频系统资产，和 `LyraAudioSettings` 强相关 |
| `Input` | Enhanced Input 资产，和 Mapping Context/按键设置强相关 |
| `UI` | CommonUI、设置界面、HUD、弹窗、字体、输入图标 |
| `Dome` | 示例或测试地图资源 |
| `__ExternalActors__` | World Partition 外部 Actor 数据，不手动编辑路径 |

## Audio 资产

主要目录：

```text
Content/Audio/Classes
Content/Audio/Modulation/ControlBuses
Content/Audio/Modulation/ControlBusMixes
Content/Audio/Modulation/ParameterPatches
Content/Audio/Submixes
Content/Audio/Effects/SubmixEffects
Content/Audio/MetaSounds
Content/Audio/Sounds
Content/Audio/SoundWaves
```

典型资产：

```text
CB_Main.uasset
CB_Music.uasset
CB_SFX.uasset
CB_UI.uasset
CB_VoiceChat.uasset
CBM_BaseMix.uasset
CBM_LoadingScreenMix.uasset
CBM_UserMix.uasset
```

它们被这些代码使用：

```text
ULyraAudioSettings
ULyraAudioMixEffectsSubsystem
LyraGameSettingRegistry_Audio.cpp
LyraSettingsLocal
```

维护规则：

1. Control Bus 名称要和设置含义一致。
2. Control Bus Mix 里要包含用户可调的 Bus。
3. 删除或移动资产后必须更新 Project Settings 或 Config 中的软引用。
4. 打包前检查软引用资产是否会被烘焙。

## Input 资产

主要目录：

```text
Content/Input
Content/Input/Actions
```

输入资产被这些代码使用：

```text
ULyraInputConfig
ULyraInputComponent
ASetlyPlayerController
USetlyForntUIComponent
LyraGameSettingRegistry_MouseAndKeyboard.cpp
LyraGameSettingRegistry_Gamepad.cpp
```

维护规则：

1. 新增 Input Action 后，要配置到 Mapping Context。
2. 需要 C++ 绑定时，要加入 `ULyraInputConfig` 并分配 GameplayTag。
3. 需要改键时，确认该映射支持 Player Mappable。
4. UI 输入和游戏输入要注意 Mapping Context 优先级。

## UI 资产

主要目录：

```text
Content/UI/Foundation
Content/UI/FrontEnd
Content/UI/Hud
Content/UI/Menu
Content/UI/PerfStats
Content/UI/Settings
Content/UI/Credits
```

### Foundation

基础控件、字体、图标、平台输入图标、RichText、LoadingScreen 等：

```text
Content/UI/Foundation/Buttons
Content/UI/Foundation/Dialogs
Content/UI/Foundation/Fonts
Content/UI/Foundation/Icons
Content/UI/Foundation/LoadingScreen
Content/UI/Foundation/Platform/Input
Content/UI/Foundation/RichTextData
Content/UI/Foundation/Subtitles
Content/UI/Foundation/TabbedView
Content/UI/Foundation/Widgets
```

对应代码：

```text
LyraButtonBase
LyraActionWidget
LyraConfirmationScreen
LyraControllerDisconnectedScreen
LyraLoadingScreenSubsystem
```

### Settings

设置界面资产：

```text
Content/UI/Settings
Content/UI/Settings/Editors
Content/UI/Settings/Extensions
Content/UI/Settings/Media
Content/UI/Settings/Screens
```

重点资产：

```text
Content/UI/Settings/GameSettingRegistryVisuals.uasset
```

它决定 GameSettings 里的不同设置类型如何映射到具体 UI Entry。新增设置项但 UI 显示不对时，必须检查这里。

### PerfStats

性能统计 UI：

```text
Content/UI/PerfStats
```

对应代码：

```text
LyraPerfStatContainerBase
LyraPerfStatWidgetBase
LyraPerfStatGraph
```

### Platform Input 图标

目录：

```text
Content/UI/Foundation/Platform/Input
```

包含：

```text
GamepadPS4
GamepadPS5
GamepadSwitch
GamepadXboxOne
GamepadXboxSeriesX
KeyboardMouse
Misc
```

如果按钮图标不显示或平台图标错误，优先检查 CommonUI 输入平台配置和这些资产。

## Dome 与 ExternalActors

```text
Content/Dome
Content/__ExternalActors__/Dome
```

这是地图和 World Partition 相关数据。维护规则：

1. 不要手动移动 `__ExternalActors__` 下的文件。
2. 地图改名或移动时用 Unreal Editor 操作。
3. 提交版本时地图和外部 Actor 数据要一起提交。

## 资产引用关系

常见引用链：

```text
Audio 资产
  -> ULyraAudioSettings
  -> ULyraAudioMixEffectsSubsystem
  -> 音量设置 UI
```

```text
Input 资产
  -> Mapping Context
  -> PlayerController/FrontUIComponent
  -> LyraInputComponent
  -> 按键设置 UI
```

```text
UI 资产
  -> Project Settings 或 UIExtension
  -> CommonUI Root Layout
  -> Activatable Widget Stack
```

```text
Settings VisualData
  -> GameSettings Setting 类型
  -> 对应 List Entry Widget
```

## 新增资产流程

新增 UI 资产：

1. 放到对应功能目录，不要随意丢到根目录。
2. 命名体现用途，例如 `W_`, `B_`, `MI_`, `T_`, `DA_`。
3. 如果是软引用入口，更新 Project Settings、Config 或 DataAsset。
4. 如果要进设置界面，更新 `GameSettingRegistryVisuals.uasset`。
5. 打包验证资产没有 Missing Reference。

新增 Audio 资产：

1. 放入 `Content/Audio` 对应子目录。
2. 如果是音量控制，更新 Control Bus/Mix。
3. 更新 `ULyraAudioSettings` 指向。
4. 测试加载界面和正常游戏两种混音状态。

新增 Input 资产：

1. 放入 `Content/Input/Actions` 或输入配置目录。
2. 加入 Mapping Context。
3. 配 GameplayTag 和 Player Mappable。
4. 在设置 UI 中验证改键。

## 维护检查

每次移动、删除、重命名资产后检查：

1. Unreal Editor 中 Fix Up Redirectors。
2. C++ 软引用路径是否更新。
3. Project Settings 中的类和资产引用是否更新。
4. DataAsset 中的 WidgetClass、LayerTag、SlotTag 是否有效。
5. 打包日志没有 Missing Asset、Failed to load。
6. Wiki 里对应资产地图同步更新。

