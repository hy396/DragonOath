# 07. VisualData 与 UI Entry 映射

设置数据能显示到哪个 UI 行控件，不是写死在每个 Setting 里，而是通过 `UGameSettingVisualData` 做映射。

## 核心资源

项目里的映射资产：

```text
Content/UI/Settings/GameSettingRegistryVisuals.uasset
```

相关 C++：

```text
Plugins/GameSettings/Source/Public/Widgets/GameSettingVisualData.h
Plugins/GameSettings/Source/Private/Widgets/GameSettingVisualData.cpp
Plugins/GameSettings/Source/Private/Widgets/GameSettingListView.cpp
```

## ListView 如何选择 Entry

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingListView.cpp
```

关键流程：

```cpp
if (VisualData)
{
	if (const TSubclassOf<UGameSettingListEntryBase> EntryClassSetting = VisualData->GetEntryForSetting(SettingItem))
	{
		SettingEntryClass = EntryClassSetting;
	}
}

UGameSettingListEntryBase& EntryWidget = GenerateTypedEntry<UGameSettingListEntryBase>(SettingEntryClass, OwnerTable);
EntryWidget.SetSetting(SettingItem);
```

也就是说：

1. `ListView_Settings` 拿到一个 `UGameSetting`
2. 找 `VisualData`
3. 调 `GetEntryForSetting()`
4. 生成对应的 `UGameSettingListEntryBase` 子类 Widget
5. 调 `SetSetting()` 把设置对象交给 UI 行

## VisualData 的匹配优先级

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingVisualData.cpp
```

优先级如下：

### 1. 自定义逻辑

```cpp
TSubclassOf<UGameSettingListEntryBase> CustomEntry = GetCustomEntryForSetting(InSetting);
if (CustomEntry)
{
	return CustomEntry;
}
```

默认实现返回空。如果以后做项目特化的 `UGameSettingVisualData` 子类，可以在这里写更复杂的选择逻辑。

### 2. 按 DevName 精确匹配

```cpp
TSubclassOf<UGameSettingListEntryBase> EntryWidgetClassPtr =
	EntryWidgetForName.FindRef(InSetting->GetDevName());
```

这适合极少数特殊设置，例如某个 DevName 必须使用独立 UI。

### 3. 按 Setting 类匹配

```cpp
for (UClass* Class = InSetting->GetClass(); Class; Class = Class->GetSuperClass())
{
	if (TSubclassOf<UGameSetting> SettingClass = TSubclassOf<UGameSetting>(Class))
	{
		TSubclassOf<UGameSettingListEntryBase> EntryWidgetClassPtr =
			EntryWidgetForClass.FindRef(SettingClass);
		if (EntryWidgetClassPtr)
		{
			return EntryWidgetClassPtr;
		}
	}
}
```

这是最常用路径。它会沿着类继承链找最合适的 Entry。

比如语言设置类是：

```cpp
ULyraSettingValueDiscrete_Language : public UGameSettingValueDiscrete
```

如果 `GameSettingRegistryVisuals.uasset` 里配置了：

```text
UGameSettingValueDiscrete -> W_SettingsListEntry_Discrete
```

那么语言设置就会自动使用：

```text
Content/UI/Settings/Editors/W_SettingsListEntry_Discrete.uasset
```

## 常见 Entry 类型

```text
W_SettingsListEntry_Discrete
  离散选项，语言、开关、枚举、质量档位等

W_SettingsListEntry_Scalar
  滑条，音量、亮度、灵敏度等

W_SettingsListEntry_Action
  按钮动作，重置、安全区编辑等

W_SettingsListEntry_KBMBinding
  键鼠按键绑定

W_SettingsListEntry_SubCollection
  子页面或子分类入口
```

## 新增 UI Entry 的流程

1. 新建一个 UMG Widget，继承合适的 C++ Entry 基类。
2. 按 C++ `BindWidget` 名字放控件。
3. 打开 `GameSettingRegistryVisuals.uasset`。
4. 添加 `EntryWidgetForClass` 或 `EntryWidgetForName` 映射。
5. 打开设置界面测试。

如果只是新增普通离散设置，不需要新增 UI Entry，复用 `W_SettingsListEntry_Discrete` 即可。

## 维护建议

- 优先按 Setting 类映射，少用 DevName 特例。
- 如果 DevName 特例越来越多，说明应该新增一个设置子类或新的 Entry 类型。
- 修改 UMG 绑定名时必须同步 C++ `BindWidget` 字段。
- 设置不显示时，先检查 `GameSettingRegistryVisuals.uasset` 是否配置了对应类映射。

