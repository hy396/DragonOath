# 08. 本地化资源与语言列表维护

语言设置的列表来自 Unreal 本地化资源，不推荐手写语言数组作为正式方案。

## 语言列表枚举来源

文件：

```text
Source/Setly/Settings/CustomSettings/LyraSettingValueDiscrete_Language.cpp
```

代码：

```cpp
FTextLocalizationManager::Get().GetLocalizedCultureNames(ELocalizationLoadFlags::Game)
```

这个函数会枚举当前加载到的 `Game` 本地化资源对应的 Culture。

## 推荐资源结构

常见目标路径：

```text
Content/Localization/Game
```

编译后常见资源：

```text
Content/Localization/Game/<culture>/Game.locres
```

常见文化名：

```text
en
zh-Hans
zh-Hant
ja
ko
fr
de
```

## 从 0 建立语言资源

在 Unreal Editor 中：

1. 打开 Localization Dashboard。
2. 创建或启用 `Game` Localization Target。
3. 添加需要支持的 Cultures。
4. Gather Text，收集 `LOCTEXT`、`NSLOCTEXT` 和可本地化资产文本。
5. 导出或直接填写翻译。
6. Compile Text，生成 `.locres`。
7. 确认项目打包配置会包含这些 Cultures。
8. 用 Standalone 或 `-game` 测试语言设置。

## 为什么不要只改代码数组

临时可以这样做：

```cpp
AvailableCultureNames.Add(TEXT("zh-Hans"));
AvailableCultureNames.Add(TEXT("en"));
```

但这只能让 UI 出现选项。没有 `.locres` 时，文本不会真正翻译。

正式流程必须让本地化资源存在，语言设置只是读取资源并显示。

## 默认语言和用户选择

用户选择 `System Default` 时，会移除配置：

```ini
[Internationalization]
Culture=...
```

用户选择具体语言时，会写入：

```ini
[Internationalization]
Culture=zh-Hans
```

编辑器常见路径：

```text
Saved/Config/WindowsEditor/GameUserSettings.ini
```

## 测试建议

测试语言列表：

1. 启动游戏。
2. 打开设置界面。
3. 进入 Gameplay -> Language。
4. 确认可选语言包含新增 Culture。
5. 切换语言并 Apply。
6. 重启游戏。
7. 确认文本资源按新语言显示。

测试启动参数：

```text
-culture=zh-Hans
-culture=en
```

PIE 不适合作为最终验证。语言设置源码里已经提示 PIE 不会可靠响应语言切换。

## 维护清单

每次新增语言时检查：

- `Game` Localization Target 是否包含该 Culture。
- `.locres` 是否生成。
- 打包设置是否包含该 Culture。
- 运行时设置 UI 是否枚举到该 Culture。
- Apply 后是否写入 `GameUserSettings.ini`。
- 重启后是否仍然使用选择的 Culture。
- 加载屏、菜单、设置页、HUD 的文本是否都被翻译。

