# 09. 新增设置项开发模板

本页用“新增战斗飘字大小”作为完整模板。它展示从设置字段、Getter/Setter、Registry 注册、UI 显示到验证的完整流程。

> 注意：本页是开发模板，不代表已经改了 C++ 源码。

## 目标

新增一个设置：

```text
Gameplay -> Combat Text -> Damage Number Size
```

选项：

```text
Small
Medium
Large
```

保存位置：

```text
ULyraSettingsShared
```

原因：这是玩家偏好，未来可能跟账号或存档同步，不属于单纯机器配置。

## 第一步：定义枚举和字段

文件：

```text
Source/Setly/Settings/LyraSettingsShared.h
```

示例：

```cpp
UENUM(BlueprintType)
enum class ELyraDamageNumberSize : uint8
{
	Small,
	Medium,
	Large,
};
```

在 `ULyraSettingsShared` 里添加：

```cpp
public:
	UFUNCTION()
	ELyraDamageNumberSize GetDamageNumberSize() const { return DamageNumberSize; }

	UFUNCTION()
	void SetDamageNumberSize(ELyraDamageNumberSize NewValue)
	{
		ChangeValueAndDirty(DamageNumberSize, NewValue);
	}

private:
	UPROPERTY()
	ELyraDamageNumberSize DamageNumberSize = ELyraDamageNumberSize::Medium;
```

这里的 `ChangeValueAndDirty()` 会在值变化时标记设置为脏，后续 Apply/Save 才知道需要保存。

## 第二步：在 Registry 注册设置

文件：

```text
Source/Setly/Settings/LyraGameSettingRegistry_Gameplay.cpp
```

需要 include：

```cpp
#include "GameSettingValueDiscreteDynamic.h"
#include "LyraSettingsShared.h"
```

在 `InitializeGameplaySettings()` 里添加一个 Collection：

```cpp
UGameSettingCollection* CombatTextSubsection = NewObject<UGameSettingCollection>();
CombatTextSubsection->SetDevName(TEXT("CombatTextCollection"));
CombatTextSubsection->SetDisplayName(LOCTEXT("CombatTextCollection_Name", "Combat Text"));
Screen->AddSetting(CombatTextSubsection);
```

添加设置项：

```cpp
UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
Setting->SetDevName(TEXT("DamageNumberSize"));
Setting->SetDisplayName(LOCTEXT("DamageNumberSize_Name", "Damage Number Size"));
Setting->SetDescriptionRichText(LOCTEXT("DamageNumberSize_Description", "Changes the display size of floating damage numbers."));

Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetDamageNumberSize));
Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetDamageNumberSize));
Setting->SetDefaultValue(GetDefault<ULyraSettingsShared>()->GetDamageNumberSize());

Setting->AddEnumOption(ELyraDamageNumberSize::Small, LOCTEXT("DamageNumberSize_Small", "Small"));
Setting->AddEnumOption(ELyraDamageNumberSize::Medium, LOCTEXT("DamageNumberSize_Medium", "Medium"));
Setting->AddEnumOption(ELyraDamageNumberSize::Large, LOCTEXT("DamageNumberSize_Large", "Large"));

Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

CombatTextSubsection->AddSetting(Setting);
```

## 第三步：为什么 Getter/Setter 可以这样绑定

`GET_SHARED_SETTINGS_FUNCTION_PATH` 定义在：

```text
Source/Setly/Settings/LyraGameSettingRegistry.h
```

它会构造动态路径：

```text
ULyraLocalPlayer::GetSharedSettings()
  -> ULyraSettingsShared::<FunctionOrPropertyName>()
```

所以 Getter/Setter 必须满足：

- 是 `UFUNCTION()`
- Getter 无参数，返回当前值
- Setter 只有一个参数，类型和选项值匹配

## 第四步：UI 会自动显示吗

如果 `GameSettingRegistryVisuals.uasset` 里已经有：

```text
UGameSettingValueDiscrete -> W_SettingsListEntry_Discrete
```

或更具体的离散类映射，那么它会自动用离散设置行显示。

数据流：

```text
DamageNumberSize Setting
  -> UGameSettingListEntrySetting_Discrete::Refresh()
  -> GetDiscreteOptions()
  -> Rotator_SettingValue->PopulateTextLabels(Options)
```

## 第五步：业务系统如何读取

比如伤害数字显示系统可以从本地玩家拿 SharedSettings：

```cpp
if (const ULyraLocalPlayer* LyraLocalPlayer = Cast<ULyraLocalPlayer>(LocalPlayer))
{
	if (const ULyraSettingsShared* SharedSettings = LyraLocalPlayer->GetSharedSettings())
	{
		const ELyraDamageNumberSize Size = SharedSettings->GetDamageNumberSize();
	}
}
```

如果业务系统不是 LocalPlayer 上下文，需要通过 PlayerController、HUD、Subsystem 或用户设置服务拿到对应 LocalPlayer。

## 第六步：验证

开发完成后检查：

1. 编译通过。
2. 设置界面里出现 `Gameplay -> Combat Text -> Damage Number Size`。
3. Rotator 显示 `Small / Medium / Large`。
4. 切换选项后 Apply 可用。
5. Apply 后 `SharedGameSettings.sav` 更新。
6. 重启后选择仍然保留。
7. 业务系统读取到正确值。
8. Cancel 能恢复到初始值。
9. Reset to Default 回到 `Medium`。

## 参考现有实现

字幕大小已经是一个很好的现有例子：

```text
Source/Setly/Settings/LyraGameSettingRegistry_Audio.cpp
SubtitleTextSize
```

它使用：

```cpp
UGameSettingValueDiscreteDynamic_Enum
GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextSize)
GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextSize)
AddEnumOption(...)
```

新增枚举设置时优先照这个模式写。

