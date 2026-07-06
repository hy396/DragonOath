# 18. 全流程维护手册

这篇是 `Setly` 的长期维护清单，适合修 bug、交接、评审、加功能前后使用。

## 模块所有权

| 模块 | 主要目录 | 维护重点 |
| --- | --- | --- |
| 插件元数据 | `Setly.uplugin` | 模块、依赖、是否包含 Content |
| Runtime 构建 | `Source/Setly/Setly.Build.cs` | 依赖模块、Include 路径 |
| Editor 构建 | `Source/SetlyEditor` | Project Settings 注册 |
| 设置系统 | `Source/Setly/Settings` | Registry、Local/Shared、Apply/Save |
| Audio | `Source/Setly/Audio`, `Content/Audio` | ControlBus、Mix、Submix、音量滑条 |
| Input | `Source/Setly/Input`, `Content/Input` | Mapping Context、InputTag、按键重映射 |
| UI | `Source/Setly/UI`, `Content/UI` | CommonUI、Layer、弹窗、设置界面 |
| Player/前端 | `Source/Setly/Player`, `Source/Setly/Setly` | LocalPlayer、Controller、前端组件 |
| Performance | `Source/Setly/Performance` | 帧率、画质、性能统计 |
| Development | `Source/Setly/Development` | PIE 调试、平台模拟、开发者配置 |
| Content | `Content` | 资产引用、Redirector、打包烘焙 |

## 改动前判断

先问自己这个改动属于哪一类：

| 改动类型 | 应该优先看 |
| --- | --- |
| 新增设置项 | `05-extension-guide.md`, `09-new-setting-example.md` |
| 修设置 UI | `03-ui-binding-flow.md`, `07-visual-data-mapping.md` |
| 修保存/Apply | `04-apply-and-save.md`, `10-maintenance-checklist.md` |
| 修语言/本地化 | `02-language-setting.md`, `08-localization-workflow.md` |
| 修音量 | `12-audio-system.md` |
| 修输入/改键 | `13-input-system.md` |
| 修菜单/HUD/弹窗 | `14-ui-framework.md` |
| 修前端启动 | `15-player-front-end-flow.md` |
| 修画质/帧率/统计 | `16-performance-development.md` |
| 修资产引用 | `17-content-assets-map.md` |

## 新增功能通用流程

1. 确认模块边界。
2. 找到现有同类实现。
3. 新增 C++ 数据或函数。
4. 新增或更新资产。
5. 注册到对应系统。
6. 接入 UI 或配置。
7. 实测 Apply、Cancel、保存、重启。
8. 检查 PIE、Standalone、打包差异。
9. 更新 Wiki。

## 设置项维护流程

新增或修改设置项时检查：

1. 设置属于 `Local` 还是 `Shared`。
2. Getter/Setter 是否存在。
3. Setter 是否标记 Dirty 或触发事件。
4. Registry 是否注册到正确分类。
5. UI Entry 映射是否存在。
6. 默认值是否合理。
7. Apply/Cancel 是否符合预期。
8. 重启后是否保留。
9. 多语言文本是否进入本地化流程。

## Audio 维护流程

修改音频时检查：

1. `ULyraAudioSettings` 软引用是否有效。
2. Control Bus 名称和设置含义是否一致。
3. User Mix 是否包含用户可调 Bus。
4. Loading Screen Mix 是否只在加载界面生效。
5. HDR/LDR Submix Chain 是否能加载。
6. 音量设置 Apply 后是否立即生效。
7. 打包后资产是否被烘焙。

## Input 维护流程

修改输入时检查：

1. Input Action 是否存在。
2. Mapping Context 是否绑定默认按键。
3. Mapping Context 是否被 PlayerController 或 FrontUIComponent 添加。
4. GameplayTag 是否和 `LyraInputConfig` 一致。
5. Native/Ability 绑定是否走正确路径。
6. Player Mappable 是否能出现在设置界面。
7. 改键保存是否生效。
8. 键鼠、手柄、CommonUI 图标都能工作。

## UI 维护流程

修改 UI 时检查：

1. Widget 是否继承合适的 CommonUI 基类。
2. 是否推入正确 Layer。
3. 弹窗类是否在 Project Settings 中配置。
4. Escape/Back 输入是否可用。
5. 默认焦点是否正确。
6. UIExtension Handle 是否释放。
7. 设置界面 Entry 是否和 VisualData 匹配。
8. 字体、图标、图片是否能打包加载。

## Player/FrontEnd 维护流程

修改玩家或前端流程时检查：

1. `ULyraLocalPlayer` 是否能拿到 Local/Shared Settings。
2. Shared Settings 异步加载时 UI 是否能处理临时值。
3. PlayerController BeginPlay/EndPlay 是否成对注册和清理输入。
4. `USetlyForntUIComponent` 是否只注册前端需要的输入。
5. `USetlyUIExtensionComponent` 是否只注入目标玩家的 UI。
6. Controller 切换后输入和设置仍然正常。

## Performance 维护流程

修改性能相关逻辑时检查：

1. 设置是否受平台特性控制。
2. Desktop/Console/Mobile 三种 FramePacing 是否区分清楚。
3. Device Profile 是否会覆盖用户选择。
4. `ULyraSettingsLocal` 是否正确应用 CVar/GameUserSettings。
5. 性能统计新增枚举后 UI 能显示。
6. PIE 平台模拟设置是否影响测试结论。

## Content 维护流程

修改资产后检查：

1. 使用 Unreal Editor 保存所有相关资产。
2. Fix Up Redirectors。
3. 检查软引用和硬引用。
4. 打开引用查看器确认没有断链。
5. 运行一次 PIE。
6. 有条件时跑 Standalone 或打包。
7. 地图和 `__ExternalActors__` 一起维护。

## 回归测试矩阵

| 测试项 | 必测内容 |
| --- | --- |
| 设置界面 | 分类、显示、修改、Apply、Cancel、恢复默认 |
| 保存 | 退出重进后 Local/Shared 设置是否保留 |
| 语言 | 语言列表、切换、重启、本地化文本 |
| Audio | 总音量、音乐、音效、加载界面混音 |
| Input | 键鼠、手柄、改键、输入图标 |
| UI | 菜单、弹窗、返回、焦点、HUD |
| Performance | 分辨率、画质、帧率、性能统计 |
| FrontEnd | 进入、退出、切图、Controller 切换 |
| Packaging | 软引用资产、字体图标、音频和 UI 资产 |

## 常见高风险点

| 风险 | 为什么危险 |
| --- | --- |
| 软引用资产路径变化 | C++ 不会编译报错，但运行时加载失败 |
| Local/Shared 放错 | 设置可能跨机器同步错误或本机无法保存 |
| Apply 只改 UI 不改底层 | 用户看到值变了，实际系统没变 |
| UIExtension 不释放 | 切图或重新进入后 Widget 重复 |
| Input Mapping 优先级冲突 | 菜单输入和游戏输入互相抢 |
| Device Profile 覆盖 | 用户设置看似保存，运行时又被平台配置改掉 |
| PIE 设置影响判断 | 开发者本机设置可能掩盖真实平台问题 |

## 提交前检查

提交 `Setly` 相关改动前建议确认：

1. 只改了本次任务需要的模块。
2. 没有误提交本机临时配置。
3. 代码能编译。
4. 新资产没有丢引用。
5. 设置类新增字段有默认值。
6. UI 文本有本地化考虑。
7. Wiki 已同步更新。
8. 变更说明写清楚影响范围。

## 交接模板

交接一个 Setly 改动时建议写：

```text
改动目标：
影响模块：
涉及文件：
涉及资产：
设置保存位置：
UI 入口：
测试过的平台/模式：
已知风险：
后续维护点：
```

## 推荐维护习惯

1. 不要只改 C++，要同步查资产。
2. 不要只测 PIE，至少测一次 Standalone。
3. 不要把玩家设置和机器设置混在一起。
4. 不要新增设置后忘记 VisualData。
5. 不要移动 `__ExternalActors__`。
6. 每次扩展一个模块，都在对应 Wiki 里补一段“如何扩展”。

