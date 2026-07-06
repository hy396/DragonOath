语言切换的数据不在 DataTable，也不在蓝图数组里，主要是 C++ 动态生成的。

入口在 [LyraGameSettingRegistry_Gameplay.cpp](D:/ue_texiao/DragonOath/Plugins/Setly/Source/Setly/Settings/LyraGameSettingRegistry_Gameplay.cpp:23)：这里创建了 `LanguageCollection`，然后添加 `ULyraSettingValueDiscrete_Language` 这个设置项。

真正的语言列表在 [LyraSettingValueDiscrete_Language.cpp](D:/ue_texiao/DragonOath/Plugins/Setly/Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp:26)：它调用：

```cpp
FTextLocalizationManager::Get().GetLocalizedCultureNames(ELocalizationLoadFlags::Game)
```

也就是说语言选项来自当前项目/插件已加载的 `Game` 本地化资源，不是手写数组。然后第 35 行插入了一个空字符串，表示 `System Default`。

玩家选择后的保存逻辑在 [LyraSettingsShared.cpp](D:/ue_texiao/DragonOath/Plugins/Setly/Source/Setly/Settings/LyraSettingsShared.cpp:182)，最终会写到用户配置：

```ini
[Internationalization]
Culture=zh-Hans
```

编辑器下对应文件是 [GameUserSettings.ini](D:/ue_texiao/DragonOath/Saved/Config/WindowsEditor/GameUserSettings.ini:1)。我看了你当前这个文件，里面还没有 `Culture`，说明现在还是系统默认语言。项目里目前也没找到 `Content/Localization` 或 `.locres`，所以如果语言列表很少/没有，就是因为还没生成游戏本地化资源。


你说的是“设置界面里那个语言选项怎么加载到 UI 上”，链路在这里：

1. 设置页创建注册表：[LyraSettingScreen.cpp](D:/ue_texiao/DragonOath/Plugins/Setly/Source/Setly/UI/LyraSettingScreen.cpp:22)`CreateRegistry()` 创建 `ULyraGameSettingRegistry`。
2. 注册语言设置项：[LyraGameSettingRegistry_Gameplay.cpp](D:/ue_texiao/DragonOath/Plugins/Setly/Source/Setly/Settings/LyraGameSettingRegistry_Gameplay.cpp:23)这里创建 `LanguageCollection`，第 30 行创建 `ULyraSettingValueDiscrete_Language`。
3. UI 列表刷新时把设置项塞进列表：
   [GameSettingPanel.cpp](D:/ue_texiao/DragonOath/Plugins/GameSettings/Source/Private/Widgets/GameSettingPanel.cpp:158)
   关键是第 172 到 174 行：

```cpp
Registry->GetSettingsForFilter(FilterState, MutableView(VisibleSettings));
ListView_Settings->SetListItems(VisibleSettings);
```

4. 语言选项真正填到 UI 旋转选择器：
   [GameSettingListEntry.cpp](D:/ue_texiao/DragonOath/Plugins/GameSettings/Source/Private/Widgets/GameSettingListEntry.cpp:125)
   第 129 到 133 行：

```cpp
const TArray<FText> Options = DiscreteSetting->GetDiscreteOptions();
Rotator_SettingValue->PopulateTextLabels(Options);
Rotator_SettingValue->SetSelectedItem(DiscreteSetting->GetDiscreteOptionIndex());
```

这里的 `GetDiscreteOptions()` 对语言来说就是走 [LyraSettingValueDiscrete_Language.cpp](D:/ue_texiao/DragonOath/Plugins/Setly/Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp:132)。

对应的 UMG 资源是：

`D:\ue_texiao\DragonOath\Plugins\Setly\Content\UI\Settings\W_LyraSettingScreen.uasset`
`D:\ue_texiao\DragonOath\Plugins\Setly\Content\UI\Settings\W_SettingsPanel.uasset`
`D:\ue_texiao\DragonOath\Plugins\Setly\Content\UI\Settings\Editors\W_SettingsListEntry_Discrete.uasset`
`D:\ue_texiao\DragonOath\Plugins\Setly\Content\UI\Settings\Editors\W_SettingsRotator.uasset`

所以结论：UI 上选语言的数据入口是 `UGameSettingListEntrySetting_Discrete::Refresh()`，它从 `ULyraSettingValueDiscrete_Language::GetDiscreteOptions()` 拿语言文本数组，再喂给 `Rotator_SettingValue->PopulateTextLabels()`。
