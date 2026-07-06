# Setly Wiki

这个目录用于学习、开发和维护 `Setly` 插件。现在 Wiki 覆盖的不只是语言设置，而是整个插件的全流程：

```text
插件结构
  -> Settings
  -> UI
  -> Player/FrontEnd
  -> Audio
  -> Input
  -> Performance/Development
  -> Content 资产
  -> 长期维护
```

`Setly` 当前是一套 Lyra 风格的前端和用户设置基础插件，包含 Settings UI、CommonUI 前端框架、Enhanced Input 输入映射、Audio Modulation 音量控制、性能设置、本地玩家设置生命周期，以及配套的 UI/Audio/Input 资产。

## 文档目录

### 设置系统专题

1. [01-settings-overview.md](01-settings-overview.md): 设置系统整体结构
2. [02-language-setting.md](02-language-setting.md): 语言设置的数据来源和当前值逻辑
3. [03-ui-binding-flow.md](03-ui-binding-flow.md): 语言选项如何加载到 UI
4. [04-apply-and-save.md](04-apply-and-save.md): Apply、Cancel、保存到配置文件的流程
5. [05-extension-guide.md](05-extension-guide.md): 如何新增设置、分类和自定义设置类
6. [06-debugging-notes.md](06-debugging-notes.md): 调试路径和常见坑
7. [07-visual-data-mapping.md](07-visual-data-mapping.md): Setting 类型如何映射到 UI Entry
8. [08-localization-workflow.md](08-localization-workflow.md): 本地化资源生产和语言列表维护流程
9. [09-new-setting-example.md](09-new-setting-example.md): 新增一个设置项的完整开发模板
10. [10-maintenance-checklist.md](10-maintenance-checklist.md): 设置系统维护、评审和测试清单

### 全插件覆盖

11. [11-plugin-map.md](11-plugin-map.md): Setly 插件全局地图、模块边界、学习路线
12. [12-audio-system.md](12-audio-system.md): Audio Modulation、Control Bus、加载界面混音、音量设置扩展
13. [13-input-system.md](13-input-system.md): Enhanced Input、GameplayTag 绑定、按键重映射、输入设置维护
14. [14-ui-framework.md](14-ui-framework.md): CommonUI、UI Manager、弹窗、HUD、UIExtension、加载界面
15. [15-player-front-end-flow.md](15-player-front-end-flow.md): LocalPlayer、PlayerController、前端组件、UI 注入流程
16. [16-performance-development.md](16-performance-development.md): 视频性能设置、帧率策略、性能统计、PIE 开发者设置
17. [17-content-assets-map.md](17-content-assets-map.md): Content 资产地图、Audio/Input/UI 资产维护
18. [18-full-maintenance-guide.md](18-full-maintenance-guide.md): 全流程维护手册、测试矩阵、交接模板

## 推荐学习路径

第一次学习整个插件：

```text
11 -> 01 -> 03 -> 14 -> 15 -> 12 -> 13 -> 16 -> 17 -> 18
```

目标是先建立插件地图，再理解设置 UI，最后掌握 Audio/Input/UI/Performance/Content 的完整维护方式。

只学习语言设置：

```text
01 -> 02 -> 03 -> 04 -> 06
```

目标是理解语言设置从 C++ 注册、进入 UI、用户修改、Apply 保存的完整生命周期。

## 推荐开发路径

新增一个设置项：

```text
05 -> 07 -> 09 -> 10
```

新增一个音量或音频选项：

```text
12 -> 05 -> 09 -> 18
```

新增一个输入动作或改键项：

```text
13 -> 05 -> 09 -> 18
```

新增一个前端页面、HUD 或弹窗：

```text
14 -> 15 -> 17 -> 18
```

新增一个性能统计或视频选项：

```text
16 -> 05 -> 09 -> 18
```

## 推荐维护路径

修设置相关问题：

```text
06 -> 10 -> 18
```

修资产引用或打包缺资源：

```text
17 -> 18
```

做团队交接：

```text
11 -> 18
```

## 快速定位

插件描述：

```text
Plugins/Setly/Setly.uplugin
```

Runtime 模块构建：

```text
Source/Setly/Setly.Build.cs
```

Editor 模块：

```text
Source/SetlyEditor
```

语言设置注册位置：

```text
Source/Setly/Settings/LyraGameSettingRegistry_Gameplay.cpp
```

语言选项生成位置：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

设置保存应用位置：

```text
Source/Setly/Settings/LyraSettingsLocal.cpp
Source/Setly/Settings/LyraSettingsShared.cpp
```

设置视觉映射资源：

```text
Content/UI/Settings/GameSettingRegistryVisuals.uasset
```

Audio 配置：

```text
Source/Setly/Audio/LyraAudioSettings.h
Content/Audio
```

Input 配置：

```text
Source/Setly/Input
Content/Input
```

UI 框架：

```text
Source/Setly/UI
Content/UI
```

Player/前端入口：

```text
Source/Setly/Player/LyraLocalPlayer.h
Source/Setly/Setly
```

性能设置：

```text
Source/Setly/Performance
Source/Setly/Development
```

## 维护原则

1. 先判断改动属于哪个模块，再找对应 Wiki。
2. 设置项要同时检查 Registry、Settings 对象、UI VisualData、保存逻辑。
3. UI、Audio、Input 很多行为依赖资产，不能只看 C++。
4. Local 设置和 Shared 设置不要混用。
5. 软引用资产移动后必须更新配置并 Fix Up Redirectors。
6. 每次扩展模块后同步更新 Wiki。
