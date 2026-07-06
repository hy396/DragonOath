# 13. Input 系统

这篇解释 `Setly` 的输入配置、Enhanced Input 绑定、按键重映射和输入设置如何维护。输入系统连接了三块内容：C++ 绑定、Input 资产、设置 UI。

## 核心文件

```text
Source/Setly/Input/LyraInputConfig.h
Source/Setly/Input/LyraInputComponent.h
Source/Setly/Input/LyraInputModifiers.h
Source/Setly/Input/LyraInputUserSettings.h
Source/Setly/Input/LyraMappableConfigPair.h
Source/Setly/Input/LyraPlayerMappableKeyProfile.h
Source/Setly/Setly/SetlyPlayerController.h
Source/Setly/Setly/SetlyForntUIComponent.h
Source/Setly/Settings/LyraGameSettingRegistry_Gamepad.cpp
Source/Setly/Settings/LyraGameSettingRegistry_MouseAndKeyboard.cpp
Source/Setly/Settings/CustomSettings/LyraSettingKeyboardInput.h
Content/Input
```

## 核心概念

### Input Action

Enhanced Input 的原子输入，例如跳跃、移动、攻击、打开菜单。资产通常在：

```text
Content/Input/Actions
```

### Mapping Context

把键盘、鼠标、手柄按键映射到 Input Action。项目里默认输入映射通过这些结构挂到玩家身上：

```text
FInputMappingContextAndPriority
FSetlyInputMappingContextAndPriority
```

它们包含：

| 字段 | 作用 |
| --- | --- |
| `InputMapping` | 输入映射上下文资产 |
| `Priority` | 输入优先级 |
| `bRegisterWithSettings` | 是否注册到设置系统供玩家改键 |

### GameplayTag 输入标签

`ULyraInputConfig` 用 GameplayTag 把 Input Action 分成两类：

| 字段 | 用途 |
| --- | --- |
| `NativeInputActions` | C++ 手动绑定的输入 |
| `AbilityInputActions` | 自动绑定到 Gameplay Ability 的输入 |

核心查找函数：

```text
FindNativeInputActionForTag
FindAbilityInputActionForTag
```

## 输入绑定流程

整体流程：

```text
InputAction 资产
  -> MappingContext 资产
  -> PlayerController 或 FrontUIComponent 注册 MappingContext
  -> EnhancedInputLocalPlayerSubsystem 启用映射
  -> LyraInputComponent 根据 LyraInputConfig 绑定 GameplayTag
  -> 玩家按键触发 Native 或 Ability 输入逻辑
```

`ULyraInputComponent` 提供三个核心能力：

| 函数 | 作用 |
| --- | --- |
| `AddInputMappings` | 添加输入映射 |
| `RemoveInputMappings` | 移除输入映射 |
| `BindNativeAction` | 根据 GameplayTag 绑定 C++ 函数 |
| `BindAbilityActions` | 批量绑定 Ability 输入按下/释放 |
| `RemoveBinds` | 移除已绑定句柄 |

## 默认输入映射从哪里来

玩家控制器：

```text
Source/Setly/Setly/SetlyPlayerController.h
```

字段：

```text
DefaultInputMappings
```

前端 UI 组件：

```text
Source/Setly/Setly/SetlyForntUIComponent.h
```

字段：

```text
DefaultInputMappings
```

注意：代码里的类名是 `SetlyForntUIComponent`，这里保留工程原名。文档中提到 FrontUI 时指的就是这个组件。

## 输入设置如何影响手感

`LyraInputModifiers` 中有几类设置驱动的输入修饰器：

| 类 | 用途 |
| --- | --- |
| `ULyraSettingBasedScalar` | 根据 Shared Settings 中的数值缩放输入轴 |
| `ULyraInputModifierDeadZone` | 根据设置控制手柄摇杆死区 |
| `ULyraInputModifierGamepadSensitivity` | 根据灵敏度等级调整手柄视角输入 |
| `ULyraInputModifierAimInversion` | 根据设置反转瞄准轴 |

这些 Modifier 通常挂在 Input Action 或 Mapping Context 上，运行时读取玩家设置。

相关设置值主要在：

```text
Source/Setly/Settings/LyraSettingsShared.h
```

例如：

```text
GamepadMoveStickDeadZone
GamepadLookStickDeadZone
GamepadTargetingSensitivity
GamepadLookSensitivity
bInvertVerticalAxis
bInvertHorizontalAxis
```

## 按键重映射

按键重映射涉及：

```text
ULyraInputUserSettings
ULyraPlayerMappableKeySettings
ULyraPlayerMappableKeyProfile
ULyraSettingKeyboardInput
ULyraSettingsListEntrySetting_KeyboardInput
```

其中：

| 类 | 作用 |
| --- | --- |
| `ULyraInputUserSettings` | Enhanced Input 用户设置扩展点 |
| `ULyraPlayerMappableKeySettings` | 每个可映射按键的额外 UI 元数据，例如 Tooltip |
| `ULyraSettingKeyboardInput` | GameSettings 中的键盘输入设置项 |
| `ULyraSettingsListEntrySetting_KeyboardInput` | 设置列表里显示改键项的 Entry |

`FMappableConfigPair` 目前在代码里标记了 UE 5.3 之后废弃，注释建议使用 `FInputMappingContextAndPriority`。后续开发应优先使用新的 Mapping Context + Priority 结构。

## 新增一个输入动作

例如新增“闪避”：

1. 在 `Content/Input/Actions` 创建 `IA_Dodge`。
2. 在 Mapping Context 中绑定默认键位，例如键盘 `Space` 或手柄 `FaceButtonBottom`。
3. 给动作分配一个 GameplayTag，例如 `InputTag.Ability.Dodge`。
4. 在 `ULyraInputConfig` 的 `AbilityInputActions` 或 `NativeInputActions` 中加入它。
5. 如果是 Ability 输入，确保 Ability 的输入标签一致。
6. 如果是 Native 输入，在 Pawn/Controller 中用 `BindNativeAction` 绑定函数。
7. 如果需要玩家改键，确认 Mapping Context 的 `bRegisterWithSettings = true`。
8. 在设置 UI 中检查键位是否出现。
9. 测试按下、释放、改键、保存、重启。

## 新增一个输入设置

例如新增“瞄准时镜头加速度”：

1. 判断它是本机设置还是玩家设置。
2. 如果跟玩家账号走，放进 `LyraSettingsShared`。
3. 增加 Getter/Setter，并在 Setter 中标记 Dirty。
4. 在 `LyraGameSettingRegistry_Gamepad.cpp` 或对应 Registry 中注册设置项。
5. 如果要实时影响输入，新增或修改 `UInputModifier`。
6. 在 Input Action 或 Mapping Context 中使用该 Modifier。
7. 测试 Apply、Cancel、保存和重启恢复。

## 常见问题

| 现象 | 可能原因 |
| --- | --- |
| 按键没有反应 | Mapping Context 没添加、优先级被覆盖、InputTag 不一致 |
| Ability 不触发 | `AbilityInputActions` 没配置或 Ability 输入标签不匹配 |
| UI 里看不到改键项 | `bRegisterWithSettings` 关闭或资产不是 Player Mappable |
| 改键后重启丢失 | Input user settings 或 Shared settings 没保存 |
| 手柄死区无效 | Modifier 没挂到输入轴，或 Shared Settings 没读到 |
| PIE 正常但打包不正常 | 输入资产软引用没有被引用或烘焙 |

## 维护检查

修改 Input 后至少检查：

1. 键鼠和手柄都能触发核心操作。
2. CommonUI 输入图标显示正确。
3. 改键后能 Apply、Cancel、恢复默认。
4. 重启后玩家改键仍然存在。
5. Mapping Context 优先级没有让 UI 输入和游戏输入冲突。
6. 废弃的 `FMappableConfigPair` 不再用于新逻辑。

