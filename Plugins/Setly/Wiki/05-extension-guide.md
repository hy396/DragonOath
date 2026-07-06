# 05. 拓展指南

本页记录如何基于现有 Settings 框架新增功能。

## 新增语言

正式推荐路线：

1. 打开 Unreal Localization Dashboard
2. 创建或配置 `Game` Localization Target
3. 添加文化，例如 `zh-Hans`、`en`、`ja`
4. Gather Text
5. 翻译文本
6. Compile Text，生成 `.locres`
7. 确认打包设置包含这些文化
8. 运行游戏，语言设置会通过 `GetLocalizedCultureNames(ELocalizationLoadFlags::Game)` 枚举出来

不推荐只在代码里硬塞语言名。UI 里显示了语言，不代表翻译资源存在。

临时测试可以这样追加：

```cpp
AvailableCultureNames.Add(TEXT("zh-Hans"));
AvailableCultureNames.Add(TEXT("en"));
```

但正式项目应该由本地化资源驱动列表。

## 新增普通离散设置

适合：

- 自动拾取：Off / On
- 字幕大小：Small / Normal / Large
- 战斗飘字大小：Small / Medium / Large
- 服务器区域：Asia / Europe / America

优先使用：

```cpp
UGameSettingValueDiscreteDynamic
```

示例：

```cpp
UGameSettingValueDiscreteDynamic* Setting = NewObject<UGameSettingValueDiscreteDynamic>();
Setting->SetDevName(TEXT("SubtitleSize"));
Setting->SetDisplayName(LOCTEXT("SubtitleSize_Name", "Subtitle Size"));
Setting->SetDescriptionRichText(LOCTEXT("SubtitleSize_Description", "Changes subtitle text size."));

Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitleSize));
Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitleSize));
Setting->SetDefaultValueFromString(TEXT("Normal"));

Setting->AddDynamicOption(TEXT("Small"), LOCTEXT("SubtitleSize_Small", "Small"));
Setting->AddDynamicOption(TEXT("Normal"), LOCTEXT("SubtitleSize_Normal", "Normal"));
Setting->AddDynamicOption(TEXT("Large"), LOCTEXT("SubtitleSize_Large", "Large"));

SomeCollection->AddSetting(Setting);
```

然后在 `ULyraSettingsShared` 或 `ULyraSettingsLocal` 里提供 Getter/Setter 和保存字段。

## 放 LocalSettings 还是 SharedSettings

放 `ULyraSettingsLocal`：

- 分辨率
- 画质
- 音量
- 输入设备
- 本机显示相关选项

放 `ULyraSettingsShared`：

- 语言
- 字幕偏好
- 账号级设置
- 可能跨设备同步的玩家偏好

不要放设置系统：

- 临时 UI 状态
- 当前界面展开/折叠状态
- 只在本次运行存在的调试值

## 新增自定义设置类

语言设置就是自定义设置类：

```cpp
class ULyraSettingValueDiscrete_Language : public UGameSettingValueDiscrete
```

你通常需要实现：

```cpp
virtual void OnInitialized() override;
virtual void StoreInitial() override;
virtual void ResetToDefault() override;
virtual void RestoreToInitial() override;
virtual void SetDiscreteOptionByIndex(int32 Index) override;
virtual int32 GetDiscreteOptionIndex() const override;
virtual TArray<FText> GetDiscreteOptions() const override;
virtual void OnApply() override;
```

适合自定义类的场景：

- 选项来自系统 API
- 选项来自平台能力
- 选项来自服务器
- 应用时需要弹窗
- 应用时需要重启、重新加载资源或重新初始化子系统
- 当前值匹配逻辑不是简单字符串相等

## 新增设置分类

在：

```text
Source/Setly/Settings/LyraGameSettingRegistry.cpp
```

添加：

```cpp
InterfaceSettings = InitializeInterfaceSettings(LyraLocalPlayer);
RegisterSetting(InterfaceSettings);
```

新增文件：

```text
Source/Setly/Settings/LyraGameSettingRegistry_Interface.cpp
```

示例：

```cpp
UGameSettingCollection* ULyraGameSettingRegistry::InitializeInterfaceSettings(ULyraLocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("InterfaceCollection"));
	Screen->SetDisplayName(LOCTEXT("InterfaceCollection_Name", "Interface"));
	Screen->Initialize(InLocalPlayer);

	// Add settings here

	return Screen;
}
```

还要在 `LyraGameSettingRegistry.h` 里声明函数和成员变量。

## 换 UI 外观

如果只是换语言设置的外观，不需要改 `ULyraSettingValueDiscrete_Language`。

优先看这些资产：

```text
Content/UI/Settings/W_SettingsPanel.uasset
Content/UI/Settings/Editors/W_SettingsListEntry_Discrete.uasset
Content/UI/Settings/Editors/W_SettingsRotator.uasset
```

如果要给某类设置换独立条目，需要查 `GameSettingRegistryVisuals.uasset` 和 `GameSettingVisualData` 的映射方式。

