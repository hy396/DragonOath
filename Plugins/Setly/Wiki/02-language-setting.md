# 02. 语言设置

语言设置在 `Gameplay` 分类里注册，具体选项由 `ULyraSettingValueDiscrete_Language` 动态生成。

## 注册位置

文件：

```text
Source/Setly/Settings/LyraGameSettingRegistry_Gameplay.cpp
```

关键代码：

```cpp
UGameSettingCollection* LanguageSubsection = NewObject<UGameSettingCollection>();
LanguageSubsection->SetDevName(TEXT("LanguageCollection"));
LanguageSubsection->SetDisplayName(LOCTEXT("LanguageCollection_Name", "Language"));
Screen->AddSetting(LanguageSubsection);

ULyraSettingValueDiscrete_Language* Setting = NewObject<ULyraSettingValueDiscrete_Language>();
Setting->SetDevName(TEXT("Language"));
Setting->SetDisplayName(LOCTEXT("LanguageSetting_Name", "Language"));
Setting->SetDescriptionRichText(LOCTEXT("LanguageSetting_Description", "The language of the game."));
Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

LanguageSubsection->AddSetting(Setting);
```

`SetDevName(TEXT("Language"))` 是内部识别名。蓝图或代码如果要跳转到这个设置，找的就是 `Language`。

## 语言列表来源

文件：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

初始化时枚举当前游戏可用的本地化文化：

```cpp
const TArray<FString> AllCultureNames =
	FTextLocalizationManager::Get().GetLocalizedCultureNames(ELocalizationLoadFlags::Game);

for (const FString& CultureName : AllCultureNames)
{
	if (FInternationalization::Get().IsCultureAllowed(CultureName))
	{
		AvailableCultureNames.Add(CultureName);
	}
}

AvailableCultureNames.Insert(TEXT(""), SettingSystemDefaultLanguageIndex);
```

重点：

- `AvailableCultureNames` 不是手写 DataTable
- 它来自 Unreal 当前加载到的 `Game` 本地化资源
- `""` 表示 `System Default`
- 如果项目没有 `.locres`，语言列表通常不会完整

常见本地化资源路径：

```text
Content/Localization/Game/<culture>/Game.locres
```

当前项目里还没有看到 `Content/Localization` 或 `.locres`，所以语言列表应该还没有完整资源支撑。

## 显示文本如何生成

`GetDiscreteOptions()` 把 `AvailableCultureNames` 转成 UI 用的 `TArray<FText>`。

```cpp
TArray<FText> ULyraSettingValueDiscrete_Language::GetDiscreteOptions() const
```

系统默认语言会显示为：

```text
System Default (<默认文化显示名>)
```

其他语言会优先显示 Native Name，如果 Native Name 和 Display Name 不同，则显示成：

```text
Native Name (Display Name)
```

例子：

```text
System Default (Chinese)
English
简体中文 (Chinese Simplified)
日本語 (Japanese)
```

## 当前选中项如何判断

```cpp
int32 ULyraSettingValueDiscrete_Language::GetDiscreteOptionIndex() const
```

它按这个优先级判断：

1. 如果要重置到默认语言，返回 `System Default`
2. 如果有 `PendingCulture`，优先用待应用语言
3. 如果没有待应用语言，读当前 `FInternationalization::Get().GetCurrentCulture()`
4. 先精确匹配，例如 `zh-Hans`
5. 再做优先级匹配，例如允许 `en-US` 显示成 `en`

## 用户选择时发生什么

```cpp
void ULyraSettingValueDiscrete_Language::SetDiscreteOptionByIndex(int32 Index)
```

逻辑：

```cpp
if (Index == SettingSystemDefaultLanguageIndex)
{
	Settings->ResetToDefaultCulture();
}
else if (AvailableCultureNames.IsValidIndex(Index))
{
	Settings->SetPendingCulture(AvailableCultureNames[Index]);
}

NotifySettingChanged(EGameSettingChangeReason::Change);
```

这一步只是暂存，不是最终保存。

