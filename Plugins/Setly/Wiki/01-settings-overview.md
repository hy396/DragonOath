# 01. 设置系统整体结构

`Setly` 的设置界面不是直接从 UMG 蓝图或 DataTable 读取选项。它的核心是 C++ 构造一棵设置树，然后通用 UI 根据设置树生成列表。

## 主要角色

```text
ULyraSettingScreen
  创建设置注册表

ULyraGameSettingRegistry
  注册 Video / Audio / Gameplay / MouseAndKeyboard / Gamepad 等设置分类

UGameSettingCollection
  一个设置分类或子分类

UGameSettingValueDiscrete / UGameSettingValueDiscreteDynamic
  离散选项设置，例如 Language、On/Off、Low/Medium/High

UGameSettingPanel
  把 Registry 里的设置筛出来，塞给 ListView

UGameSettingListEntrySetting_Discrete
  通用离散设置行，负责把 Options 填到 Rotator 控件
```

## 总链路

```text
W_LyraSettingScreen.uasset
  -> ULyraSettingScreen::CreateRegistry()
  -> ULyraGameSettingRegistry::Initialize()
  -> ULyraGameSettingRegistry::OnInitialize()
  -> RegisterSetting(...)
  -> UGameSettingPanel::RefreshSettingsList()
  -> ListView_Settings->SetListItems(VisibleSettings)
  -> UGameSettingListEntrySetting_Discrete::Refresh()
  -> Rotator_SettingValue->PopulateTextLabels(Options)
```

## 设置屏幕入口

文件：

```text
Source/Setly/UI/LyraSettingScreen.cpp
```

关键函数：

```cpp
UGameSettingRegistry* ULyraSettingScreen::CreateRegistry()
{
	ULyraGameSettingRegistry* NewRegistry = NewObject<ULyraGameSettingRegistry>();

	if (ULyraLocalPlayer* LocalPlayer = CastChecked<ULyraLocalPlayer>(GetOwningLocalPlayer()))
	{
		NewRegistry->Initialize(LocalPlayer);
	}

	return NewRegistry;
}
```

对应 UI：

```text
Content/UI/Settings/W_LyraSettingScreen.uasset
```

这个 Widget 继承 `ULyraSettingScreen`。打开设置页后，它创建 `ULyraGameSettingRegistry`，再交给通用 `Settings_Panel` 显示。

## Registry 初始化

文件：

```text
Source/Setly/Settings/LyraGameSettingRegistry.cpp
```

关键函数：

```cpp
void ULyraGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	VideoSettings = InitializeVideoSettings(LyraLocalPlayer);
	RegisterSetting(VideoSettings);

	AudioSettings = InitializeAudioSettings(LyraLocalPlayer);
	RegisterSetting(AudioSettings);

	GameplaySettings = InitializeGameplaySettings(LyraLocalPlayer);
	RegisterSetting(GameplaySettings);

	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(LyraLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);

	GamepadSettings = InitializeGamepadSettings(LyraLocalPlayer);
	RegisterSetting(GamepadSettings);
}
```

如果要新增大的设置页签或分类，通常从这里开始。

