# 03. 语言选项如何加载到 UI

本页解释“设置界面打开后，语言选项如何显示到 UI 上”。

## Settings Screen 创建 Registry

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingScreen.cpp
```

关键代码：

```cpp
UGameSettingRegistry* NewRegistry = this->CreateRegistry();
Settings_Panel->SetRegistry(NewRegistry);
Registry = NewRegistry;
```

`CreateRegistry()` 在 `ULyraSettingScreen` 中实现，创建的是 `ULyraGameSettingRegistry`。

## Panel 刷新设置列表

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingPanel.cpp
```

关键函数：

```cpp
void UGameSettingPanel::RefreshSettingsList()
```

关键代码：

```cpp
Registry->GetSettingsForFilter(FilterState, MutableView(VisibleSettings));
ListView_Settings->SetListItems(VisibleSettings);
```

这里把 Registry 里符合过滤条件的设置项塞给 `ListView_Settings`。

## 离散设置行接收设置项

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingListEntry.cpp
```

类：

```cpp
UGameSettingListEntrySetting_Discrete
```

当 ListView 创建条目时会调用：

```cpp
void UGameSettingListEntrySetting_Discrete::SetSetting(UGameSetting* InSetting)
{
	DiscreteSetting = Cast<UGameSettingValueDiscrete>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}
```

对于语言设置，这里的 `DiscreteSetting` 实际上是：

```cpp
ULyraSettingValueDiscrete_Language
```

## Options 填到 Rotator

关键函数：

```cpp
void UGameSettingListEntrySetting_Discrete::Refresh()
{
	if (ensure(DiscreteSetting))
	{
		const TArray<FText> Options = DiscreteSetting->GetDiscreteOptions();
		ensure(Options.Num() > 0);

		Rotator_SettingValue->PopulateTextLabels(Options);
		Rotator_SettingValue->SetSelectedItem(DiscreteSetting->GetDiscreteOptionIndex());
		Rotator_SettingValue->SetDefaultOption(DiscreteSetting->GetDiscreteOptionDefaultIndex());
	}
}
```

这就是语言选项加载到 UI 的关键点。

```text
ULyraSettingValueDiscrete_Language::GetDiscreteOptions()
  -> 返回 TArray<FText>
  -> UGameSettingListEntrySetting_Discrete::Refresh()
  -> Rotator_SettingValue->PopulateTextLabels(Options)
```

## 对应 UMG 资产

设置主界面：

```text
Content/UI/Settings/W_LyraSettingScreen.uasset
```

设置面板：

```text
Content/UI/Settings/W_SettingsPanel.uasset
```

离散设置行：

```text
Content/UI/Settings/Editors/W_SettingsListEntry_Discrete.uasset
```

离散选项切换控件：

```text
Content/UI/Settings/Editors/W_SettingsRotator.uasset
```

## 必须绑定的控件名

`W_SettingsListEntry_Discrete` 必须有这些 `BindWidget`：

```cpp
Panel_Value
Rotator_SettingValue
Button_Decrease
Button_Increase
```

声明位置：

```text
Plugins/GameSettings/Source/Public/Widgets/GameSettingListEntry.h
```

`Rotator_SettingValue` 是真正显示语言文本并左右切换的控件。

## 点击左右按钮

点击左：

```cpp
Rotator_SettingValue->ShiftTextLeft();
DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
```

点击右：

```cpp
Rotator_SettingValue->ShiftTextRight();
DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
```

对语言设置来说，最终会进入：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

也就是 `ULyraSettingValueDiscrete_Language::SetDiscreteOptionByIndex()`。

