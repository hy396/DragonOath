# 06. 调试路径与常见坑

## 调试路径

### 1. 看语言设置是否注册

文件：

```text
Source/Setly/Settings/LyraGameSettingRegistry_Gameplay.cpp
```

确认执行到：

```cpp
LanguageSubsection->AddSetting(Setting);
```

### 2. 看语言资源是否枚举到

文件：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

在 `OnInitialized()` 里打印：

```cpp
AllCultureNames
AvailableCultureNames
```

如果 `AllCultureNames` 为空或只有很少项目，通常是本地化资源没生成或没加载。

### 3. 看 UI 是否拿到 Options

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingListEntry.cpp
```

在这里看 `Options.Num()`：

```cpp
const TArray<FText> Options = DiscreteSetting->GetDiscreteOptions();
```

### 4. 看 Rotator 是否填充

同一文件：

```cpp
Rotator_SettingValue->PopulateTextLabels(Options);
Rotator_SettingValue->SetSelectedItem(DiscreteSetting->GetDiscreteOptionIndex());
```

如果 `Options` 有值但 UI 不显示，重点检查 `W_SettingsListEntry_Discrete.uasset` 里的 `Rotator_SettingValue` 绑定是否正确。

### 5. 看点击是否进设置类

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingListEntry.cpp
```

点击左/右后应进入：

```cpp
DiscreteSetting->SetDiscreteOptionByIndex(...)
```

语言设置最终进入：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

```cpp
ULyraSettingValueDiscrete_Language::SetDiscreteOptionByIndex()
```

### 6. 看 Apply 是否写入配置

文件：

```text
Source/Setly/Settings/LyraSettingsShared.cpp
```

函数：

```cpp
ULyraSettingsShared::ApplyCultureSettings()
```

检查：

```cpp
PendingCulture
bResetToDefaultCulture
GGameUserSettingsIni
```

## 常见坑

### PIE 里语言切换不生效

这是预期行为。用 Standalone 或 `-game` 测。

### UI 上没有其他语言

通常是没有 Game 本地化资源。检查是否存在：

```text
Content/Localization/Game
```

以及是否生成 `.locres`。

### 代码硬塞语言名后仍然没翻译

语言列表能显示，不代表翻译资源存在。没有 `.locres`，文本不会真的翻译。

### 切换后有些文本没刷新

这就是当前实现提示重启的原因。运行时 Culture 改变不能保证所有 UI 和资源全部重新加载。

### Apply 前关闭界面

`ULyraSettingScreen::HandleBackAction()` 当前会先 `ApplyChanges()`，再关闭界面。

如果按 Cancel：

```cpp
ChangeTracker.RestoreToInitial();
```

语言设置会清掉 `PendingCulture`。

### `GetDiscreteOptions()` 有数据但 Rotator 不显示

重点检查：

```text
Content/UI/Settings/Editors/W_SettingsListEntry_Discrete.uasset
```

确认里面有绑定名：

```text
Rotator_SettingValue
Button_Decrease
Button_Increase
Panel_Value
```

这些名字必须和 C++ `BindWidget` 完全一致。

