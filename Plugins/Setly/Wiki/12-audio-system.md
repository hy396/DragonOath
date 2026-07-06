# 12. Audio 系统

这篇解释 `Setly` 的音频设置、混音应用和后续扩展方式。它和 Settings UI 的关系很密切：UI 上看到的音量滑条，最后会落到 Audio Modulation 的 Control Bus 上。

## 核心文件

```text
Source/Setly/Audio/LyraAudioSettings.h
Source/Setly/Audio/LyraAudioSettings.cpp
Source/Setly/Audio/LyraAudioMixEffectsSubsystem.h
Source/Setly/Audio/LyraAudioMixEffectsSubsystem.cpp
Source/Setly/Settings/LyraGameSettingRegistry_Audio.cpp
Source/Setly/Settings/LyraSettingsLocal.h
Source/Setly/Settings/LyraSettingsLocal.cpp
Content/Audio
```

## 两个核心类

### `ULyraAudioSettings`

这是一个 `UDeveloperSettings`，配置存放在 Game config 中，用来告诉系统：

| 字段 | 作用 |
| --- | --- |
| `DefaultControlBusMix` | 游戏默认启用的 Control Bus Mix |
| `LoadingScreenControlBusMix` | 加载界面期间启用的混音 |
| `UserSettingsControlBusMix` | 用户音量设置写入的混音 |
| `OverallVolumeControlBus` | 总音量 Control Bus |
| `MusicVolumeControlBus` | 音乐音量 Control Bus |
| `SoundFXVolumeControlBus` | 音效音量 Control Bus |
| `DialogueVolumeControlBus` | 对话音量 Control Bus |
| `VoiceChatVolumeControlBus` | 语音聊天音量 Control Bus |
| `HDRAudioSubmixEffectChain` | HDR 音频使用的 Submix 效果链 |
| `LDRAudioSubmixEffectChain` | LDR 音频使用的 Submix 效果链 |

对应资产主要在：

```text
Content/Audio/Modulation/ControlBuses
Content/Audio/Modulation/ControlBusMixes
Content/Audio/Modulation/ParameterPatches
Content/Audio/Submixes
Content/Audio/Effects/SubmixEffects
```

### `ULyraAudioMixEffectsSubsystem`

这是一个 `UWorldSubsystem`，负责真正把音频配置应用到当前 World。

它做的事情：

1. 初始化时读取 `ULyraAudioSettings`。
2. 找到默认 Mix、加载界面 Mix、用户 Mix。
3. 找到各类音量 Control Bus。
4. World 开始播放时启用默认 Mix 和用户 Mix。
5. 加载界面显示/隐藏时应用或移除 `LoadingScreenControlBusMix`。
6. 根据 HDR Audio 设置切换 HDR/LDR Submix 效果链。

## 音量设置 UI 的数据流

音量滑条不是直接调 SoundClass 音量，而是通过 Settings 和 Audio Modulation 连接。

```text
LyraGameSettingRegistry_Audio.cpp
  -> 注册 Overall/Music/SFX/Dialogue/VoiceChat 等设置项
  -> Getter/Setter 指向 LyraSettingsLocal
  -> 用户在 UI 上拖动滑条
  -> LyraSettingsLocal 保存数值
  -> LyraAudioMixEffectsSubsystem / AudioModulation 应用到 Control Bus
  -> ControlBusMix 影响最终听到的音量
```

这意味着排查音量问题时要同时看三类东西：

1. Settings 是否注册了滑条。
2. `LyraSettingsLocal` 是否保存了数值。
3. `ULyraAudioSettings` 是否绑定了正确的 Control Bus 资产。

## 加载界面混音流程

加载界面出现时，音频系统会临时启用加载界面 Mix：

```text
CommonLoadingScreen 显示状态改变
  -> ULyraAudioMixEffectsSubsystem::OnLoadingScreenStatusChanged
  -> ApplyOrRemoveLoadingScreenMix(true)
  -> LoadingScreenControlBusMix 生效
```

加载界面隐藏时：

```text
CommonLoadingScreen 隐藏
  -> ApplyOrRemoveLoadingScreenMix(false)
  -> 移除 LoadingScreenControlBusMix
```

如果加载界面期间音乐、环境声或 UI 声音表现异常，优先检查：

```text
ULyraAudioSettings::LoadingScreenControlBusMix
Content/Audio/Modulation/ControlBusMixes/CBM_LoadingScreenMix.uasset
```

## HDR/LDR 音频流程

`ApplyDynamicRangeEffectsChains(bool bHDRAudio)` 会根据用户或平台设置决定使用：

```text
HDRAudioSubmixEffectChain
LDRAudioSubmixEffectChain
```

维护时注意：

1. 每条 Submix 链都必须有合法的 `SoundSubmix`。
2. Effect 顺序会影响最终声音，数组顺序就是处理顺序。
3. HDR 和 LDR 不是两个音量大小，而是两套动态范围处理策略。

## 新增一个音量分类

例如你要新增“环境音量 Ambience”：

1. 在 `Content/Audio/Modulation/ControlBuses` 创建 `CB_Ambience`。
2. 如果需要默认参数，在 `ParameterPatches` 创建对应 Patch。
3. 如果需要用户 Mix 控制，在 `ControlBusMixes/CBM_UserMix` 中添加对应 Control Bus。
4. 在 `ULyraAudioSettings` 增加 `AmbienceVolumeControlBus` 配置字段。
5. 在 Project Settings 或 DefaultGame.ini 中指向新资产。
6. 在 `ULyraSettingsLocal` 增加 `Get/SetAmbienceVolume` 和 Config 字段。
7. 在 `LyraGameSettingRegistry_Audio.cpp` 注册新的滑条。
8. 在 `Content/UI/Settings/GameSettingRegistryVisuals.uasset` 里确认该类型能显示成正确 Entry。
9. 进入游戏测试拖动滑条、Apply、重启后是否保留。

## 常见问题

| 现象 | 可能原因 |
| --- | --- |
| 滑条存在但声音不变 | Control Bus 没绑定、User Mix 没包含对应 Bus |
| 重启后音量恢复默认 | `LyraSettingsLocal` 没有 Config 字段或没有保存 |
| 加载界面声音突然变大/变小 | `LoadingScreenControlBusMix` 覆盖值不正确 |
| HDR 开关无效 | Submix Chain 没配置或 WorldSubsystem 没创建 |
| PIE 正常但打包异常 | Config 没进包、软引用资产没被烘焙 |

## 维护检查

修改 Audio 系统后至少检查：

1. 打开设置界面，所有音量项能正常显示。
2. 调整音量后立即能听到变化。
3. Apply 后退出重进，值仍然保留。
4. 加载界面出现时音频没有异常突变。
5. 打包或 Standalone 下软引用资产能加载。
6. 新增 Control Bus 时同步更新 Wiki 和资产命名。

