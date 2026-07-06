# 04. Apply、Cancel 与保存

语言设置选择后不会马上写配置。它会先暂存，等玩家 Apply 后再应用和保存。

## 选择语言时先暂存

文件：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

```cpp
Settings->SetPendingCulture(AvailableCultureNames[Index]);
```

这里写到：

```text
Source/Setly/Settings/LyraSettingsShared.h
```

字段：

```cpp
UPROPERTY(Transient)
FString PendingCulture;
```

`Transient` 表示这个字段本身不会直接序列化保存。真正保存发生在 Apply 后。

## Apply 入口

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingScreen.cpp
```

```cpp
void UGameSettingScreen::ApplyChanges()
{
	if (ChangeTracker.HaveSettingsBeenChanged())
	{
		ChangeTracker.ApplyChanges();
		ClearDirtyState();
		Registry->SaveChanges();
	}
}
```

`ChangeTracker.ApplyChanges()` 会调用每个变化设置的 `OnApply()`。

语言设置的 `OnApply()` 会弹窗提示重启：

```cpp
LOCTEXT("WarningLanguage_Title", "Language Changed")
LOCTEXT("WarningLanguage_Message", "You will need to restart the game completely for all language related changes to take effect.")
```

## Registry 保存

文件：

```text
Source/Setly/Settings/LyraGameSettingRegistry.cpp
```

```cpp
void ULyraGameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();

	if (ULyraLocalPlayer* LocalPlayer = Cast<ULyraLocalPlayer>(OwningLocalPlayer))
	{
		LocalPlayer->GetLocalSettings()->ApplySettings(false);

		LocalPlayer->GetSharedSettings()->ApplySettings();
		LocalPlayer->GetSharedSettings()->SaveSettings();
	}
}
```

这里分两类保存：

- `LocalSettings`: 本机显示、分辨率、音量等
- `SharedSettings`: 玩家共享偏好，例如语言

## 语言应用和写配置

文件：

```text
Source/Setly/Settings/LyraSettingsShared.cpp
```

关键函数：

```cpp
void ULyraSettingsShared::ApplyCultureSettings()
```

如果重置为系统默认：

```cpp
GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
GConfig->Flush(false, GGameUserSettingsIni);
```

如果选择了具体语言：

```cpp
GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *CultureToApply, GGameUserSettingsIni);
GConfig->Flush(false, GGameUserSettingsIni);
```

最终配置类似：

```ini
[Internationalization]
Culture=zh-Hans
```

编辑器下常见文件：

```text
Saved/Config/WindowsEditor/GameUserSettings.ini
```

打包后路径由平台决定。

## Cancel 入口

文件：

```text
Plugins/GameSettings/Source/Private/Widgets/GameSettingScreen.cpp
```

```cpp
void UGameSettingScreen::CancelChanges()
{
	ChangeTracker.RestoreToInitial();
	ClearDirtyState();
}
```

语言设置会清掉 `PendingCulture`：

```cpp
void ULyraSettingValueDiscrete_Language::RestoreToInitial()
{
	if (ULyraSettingsShared* Settings = CastChecked<ULyraLocalPlayer>(LocalPlayer)->GetSharedSettings())
	{
		Settings->ClearPendingCulture();
		NotifySettingChanged(EGameSettingChangeReason::RestoreToInitial);
	}
}
```

## 为什么提示重启

运行时 `SetCurrentCulture()` 可以更新一部分文本，但很多 UI、资源、加载屏、早期初始化文本可能已经缓存。为了避免半刷新状态，当前实现沿用 Lyra 的做法，应用后提示完整重启。

PIE 中语言切换通常也不可靠。建议用 Standalone 或 `-game` 测试。

