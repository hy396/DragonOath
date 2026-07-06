# 16. Performance 与 Development

这篇覆盖性能设置、帧率策略、性能统计、平台模拟和开发者设置。它们决定了设置菜单里的视频/性能选项，以及 PIE 调试时的项目行为。

## 核心文件

```text
Source/Setly/Performance/LyraPerformanceSettings.h
Source/Setly/Performance/LyraPerformanceSettings.cpp
Source/Setly/Performance/LyraPerformanceStatTypes.h
Source/Setly/Performance/LyraPerformanceStatSubsystem.h
Source/Setly/Performance/LyraPerformanceStatSubsystem.cpp
Source/Setly/Performance/LyraMemoryDebugCommands.cpp
Source/Setly/Settings/LyraGameSettingRegistry_Video.cpp
Source/Setly/Settings/LyraGameSettingRegistry_PerfStats.cpp
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_MobileFPSType.h
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_OverallQuality.h
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_PerfStat.h
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Resolution.h
Source/Setly/Development/LyraDeveloperSettings.h
Source/Setly/Development/LyraPlatformEmulationSettings.h
Content/UI/PerfStats
```

## 性能设置结构

`ULyraPerformanceSettings` 是：

```text
UDeveloperSettingsBackedByCVars
```

它通过配置和 CVar 影响项目性能选项。

重要字段：

| 字段 | 作用 |
| --- | --- |
| `DesktopFrameRateLimits` | 桌面平台可选帧率列表 |
| `UserFacingPerformanceStats` | 设置界面允许显示的性能统计 |
| `PerPlatformSettings` | 平台专属渲染设置入口 |

平台专属设置：

```text
ULyraPlatformSpecificRenderingSettings
```

重要字段：

| 字段 | 作用 |
| --- | --- |
| `DefaultDeviceProfileSuffix` | 默认设备配置后缀 |
| `UserFacingDeviceProfileOptions` | 用户可选画质/设备配置档 |
| `bSupportsGranularVideoQualitySettings` | 是否支持细分画质 |
| `bSupportsAutomaticVideoQualityBenchmark` | 是否支持自动画质检测 |
| `FramePacingMode` | 帧率控制模式 |
| `MobileFrameRateLimits` | 移动端可选帧率 |

## 帧率控制模式

```text
ELyraFramePacingMode
```

| 模式 | 适用场景 |
| --- | --- |
| `DesktopStyle` | 桌面平台，用户可手动选择帧率和 VSync |
| `ConsoleStyle` | 主机平台，通过 Device Profile 控制 |
| `MobileStyle` | 移动平台，通过可选帧率档位和设备能力控制 |

对应应用逻辑在：

```text
ULyraSettingsLocal
```

重点函数：

```text
UpdateEffectiveFrameRateLimit
UpdateGameModeDeviceProfileAndFps
UpdateConsoleFramePacing
UpdateDesktopFramePacing
UpdateMobileFramePacing
SetDesiredMobileFrameRateLimit
```

## 视频设置 UI 流程

```text
LyraGameSettingRegistry_Video.cpp
  -> 读取 ULyraPerformanceSettings 和 ULyraPlatformSpecificRenderingSettings
  -> 根据平台决定显示哪些设置
  -> Getter/Setter 指向 ULyraSettingsLocal
  -> Apply 时写入 GameUserSettings、Scalability、DeviceProfile、CVar
```

视频设置通常属于本机设置，因此主要放在：

```text
ULyraSettingsLocal
```

不要随便放到 `ULyraSettingsShared`，否则一台机器上的分辨率和画质可能被云存档同步到另一台机器。

## 性能统计系统

统计类型：

```text
ELyraDisplayablePerformanceStat
```

包括：

```text
ClientFPS
ServerFPS
IdleTime
FrameTime
FrameTime_GameThread
FrameTime_RenderThread
FrameTime_RHIThread
FrameTime_GPU
Ping
PacketLoss_Incoming
PacketLoss_Outgoing
PacketRate_Incoming
PacketRate_Outgoing
PacketSize_Incoming
PacketSize_Outgoing
Latency_Game
Latency_Render
Latency_Total
```

显示模式：

```text
ELyraStatDisplayMode
```

包括：

```text
Hidden
TextOnly
GraphOnly
TextAndGraph
```

数据缓存入口：

```text
ULyraPerformanceStatSubsystem
```

它内部使用：

```text
FLyraPerformanceStatCache
```

流程：

```text
Engine charting/performance data
  -> FLyraPerformanceStatCache::ProcessFrame
  -> ULyraPerformanceStatSubsystem::GetCachedStat
  -> PerfStats Widget 读取并显示
  -> Settings 中选择显示模式
```

## 新增一个性能统计项

1. 在 `LyraPerformanceStatTypes.h` 的 `ELyraDisplayablePerformanceStat` 中增加枚举。
2. 在 `FLyraPerformanceStatCache::GetCachedStat` 中返回该数据。
3. 如果数据来自网络或引擎统计，确认 `ProcessFrame` 能拿到。
4. 在 `LyraPerformanceSettings` 的 `UserFacingPerformanceStats` 中配置允许显示。
5. 在 `LyraGameSettingRegistry_PerfStats.cpp` 中确认设置项生成逻辑覆盖它。
6. 在 `Content/UI/PerfStats` 中确认显示 Widget 能处理该类型。
7. 测试 Text、Graph、TextAndGraph、Hidden 四种模式。

## 新增一个帧率档位

桌面平台：

```text
ULyraPerformanceSettings::DesktopFrameRateLimits
```

移动平台：

```text
ULyraPlatformSpecificRenderingSettings::MobileFrameRateLimits
```

注意：

1. 移动端还会受 Device Profile 的最大帧率限制。
2. 显示器刷新率不支持时，高帧率选项可能被过滤。
3. PIE 中是否应用帧率设置受 Platform Emulation 设置影响。

## Development 设置

`ULyraDeveloperSettings` 存在于：

```text
Source/Setly/Development/LyraDeveloperSettings.h
```

配置域：

```text
EditorPerProjectUserSettings
```

适合放只影响开发者本机的东西。

重要内容：

| 字段 | 用途 |
| --- | --- |
| `ExperienceOverride` | PIE 时覆盖体验配置 |
| `bOverrideBotCount` | 是否覆盖 Bot 数量 |
| `OverrideNumPlayerBotsToSpawn` | Bot 数量 |
| `bTestFullGameFlowInPIE` | PIE 是否测试完整游戏流程 |
| `bShouldAlwaysPlayForceFeedback` | 是否始终播放震动反馈 |
| `bSkipLoadingCosmeticBackgroundsInPIE` | PIE 是否跳过装饰背景 |
| `CheatsToRun` | PIE 自动执行作弊命令 |
| `LogGameplayMessages` | 是否记录 Gameplay Message |
| `CommonEditorMaps` | 编辑器常用地图 |

## 平台模拟

```text
ULyraPlatformEmulationSettings
```

它用于 PIE 中模拟平台差异：

| 字段 | 用途 |
| --- | --- |
| `AdditionalPlatformTraitsToEnable` | 额外启用的平台 Trait |
| `AdditionalPlatformTraitsToSuppress` | 额外屏蔽的平台 Trait |
| `PretendPlatform` | 假装当前平台 |
| `PretendBaseDeviceProfile` | 假装当前基础 Device Profile |
| `bApplyFrameRateSettingsInPIE` | PIE 是否应用帧率设置 |
| `bApplyFrontEndPerformanceOptionsInPIE` | PIE 是否应用前端性能设置 |
| `bApplyDeviceProfilesInPIE` | PIE 是否应用设备配置 |

如果设置项在 PIE 和打包里显示不同，优先检查平台 Trait 和这些模拟选项。

## 常见问题

| 现象 | 可能原因 |
| --- | --- |
| 视频设置项不显示 | 平台设置不支持该项，或 Visibility Query 不通过 |
| 帧率档位少了 | 刷新率、Device Profile、Mobile MaxFrameRate 限制 |
| Apply 后画质没变 | `ULyraSettingsLocal` 没调用对应应用逻辑 |
| 性能统计全是 0 | StatSubsystem 没初始化或 charting 数据没到 |
| PIE 和 Standalone 不一致 | Platform Emulation 设置参与了 PIE |
| 画质被自动改回 | Device Profile 限制或移动端质量钳制 |

## 维护检查

修改性能/开发设置后至少检查：

1. 桌面、移动、主机风格配置不会互相污染。
2. 视频设置 Apply 后实际 CVar 或 GameUserSettings 有变化。
3. 重启后本机设置保留。
4. PIE 平台模拟关闭后，真实平台逻辑仍然正确。
5. 性能统计 Widget 不因为新增枚举崩溃。
6. Device Profile 相关改动需要记录到项目 Config 维护说明中。

