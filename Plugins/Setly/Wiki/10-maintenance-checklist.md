# 10. 维护清单

本页用于后续开发、Review、交接和问题排查。

## 新增设置前

确认这些问题：

- 这个设置是本机设置还是玩家共享设置？
- 它需要 Apply 后生效，还是改完立即生效？
- 它需要重启吗？
- 它是否只允许主玩家修改？
- 它是否需要平台条件，例如手柄、移动端、HDR、音频输出设备？
- 它是否需要本地化文本？
- 它是否需要新 UI Entry，还是复用已有 Entry？

## 代码维护规则

优先遵守现有模式：

- 分类注册放在 `LyraGameSettingRegistry_*.cpp`
- 本机设置放 `ULyraSettingsLocal`
- 玩家共享设置放 `ULyraSettingsShared`
- 离散枚举优先用 `UGameSettingValueDiscreteDynamic_Enum`
- 布尔值优先用 `UGameSettingValueDiscreteDynamic_Bool`
- 滑条优先用 `UGameSettingValueScalarDynamic`
- 特殊设置才新增自定义 Setting 类

命名约定：

- `DevName` 用稳定英文，不要轻易改。
- `LOCTEXT` key 保持唯一且有意义。
- Collection 命名使用 `XxxCollection`。
- Page 命名使用 `XxxPage`。
- 设置项命名直接表达业务，例如 `SubtitleTextSize`。

## UI 维护规则

修改 UI Entry 时检查：

- `BindWidget` 名字是否和 C++ 一致。
- `GameSettingRegistryVisuals.uasset` 是否映射到正确 Entry。
- 键鼠、手柄导航是否可用。
- Disabled 状态是否正确同步。
- Reset 默认值显示是否正确。
- 文本长度在中文、英文下都不溢出。

## 本地化维护规则

新增或修改设置文本后：

- 所有 `SetDisplayName()` 使用 `LOCTEXT`。
- 所有 `SetDescriptionRichText()` 使用 `LOCTEXT`。
- 所有选项显示文本使用 `LOCTEXT`。
- 运行 Gather Text。
- 更新翻译。
- Compile Text。
- Standalone 或 `-game` 验证语言切换。

## 保存维护规则

修改保存字段后检查：

- 默认值是否合理。
- Getter/Setter 是否是 `UFUNCTION()`。
- Setter 是否调用 `ChangeValueAndDirty()`。
- Apply 时是否真正调用到对应设置。
- SaveGame 兼容性是否需要版本升级。
- 重启后值是否保留。

注意：`ULyraSettingsShared::GetLatestDataVersion()` 当前返回 `1`。如果以后改动保存结构并需要迁移旧数据，应考虑版本处理。

## 测试清单

每次新增设置至少测：

- 设置显示在正确分类。
- 当前值显示正确。
- 修改后 Dirty 状态变更。
- Apply 后生效。
- Cancel 后恢复。
- Reset to Default 正确。
- 重启后保存值仍在。
- 主玩家限制正确。
- 手柄和鼠标都能操作。
- PIE 限制已知，最终用 Standalone 或 `-game` 验证。

## 常见风险

### DevName 改名

DevName 可能被导航、Name Override、VisualData 特例或调试工具引用。改名前全局搜索。

### Getter/Setter 没有 UFUNCTION

动态数据源会解析失败，设置可能显示但无法读写。

### 只更新 UI 没更新 VisualData

新 Entry 不会被使用。检查 `GameSettingRegistryVisuals.uasset`。

### 只添加语言名没添加 locres

UI 能选语言，但文本不会翻译。

### PIE 里验证语言切换

PIE 对语言切换不可靠，不能作为最终结论。

## 交接建议

新增或改动设置后，同步更新：

- 对应 Wiki 页面
- 变更说明
- 新增设置的 DevName
- 保存位置
- 默认值
- 是否需要重启
- 测试结果

这样后续维护者可以快速知道这个设置为什么存在、在哪里注册、如何保存、出了问题从哪里查。

