# 05. UE 脚本插件选型对比

信息基准日期：2026-06-12

对比对象：

- [Tencent/puerts](https://github.com/Tencent/puerts)
- [Tencent/UnLua](https://github.com/Tencent/UnLua)
- [crazytuzi/UnrealCSharp](https://github.com/crazytuzi/UnrealCSharp)

## 结论先行

DragonOath 当前使用 Unreal Engine 5.8 内测版本，并且项目基础设施正在向 Lyra / CommonUI / GAS / GameplayMessageRouter 方向推进。脚本插件的选型不能只看语言喜好，更要看维护活跃度、UE 5.8 适配速度、跨平台打包、调试体验和后续维护成本。

重新评估后，推荐结论调整为：

```text
主线优先验证：UnrealCSharp
强备选方案：PuerTS
谨慎保留：UnLua
```

原因：

- UnrealCSharp 当前发布和维护节奏更活跃，官方 README 标注支持 UE 5.0 - UE 5.7 不能直接理解为落后，因为 DragonOath 的 UE 5.8 仍处于内测阶段，第三方插件没有公开标 5.8 很正常。它真正要验证的是能否在本项目的 UE 5.8 内测环境中编译、启动、打包和稳定调试。
- PuerTS 仍然是非常强的方案，尤其适合 TypeScript / JavaScript 工程化团队。它的仓库活跃度、文档、生态和类型系统优势明显，但引入 JS Runtime 后，包体、移动端、调试链和工程规范成本更高。
- UnLua 的 UE 贴合度仍然很好，Lua 也适合游戏热更和内容逻辑，但公开仓库的发布节奏明显偏旧。官方 GitHub 当前最新 Release 显示为 2023-11-07 的 2.3.6，维护活跃度需要降权。除非项目明确要 Lua 技术栈，否则不建议把它作为 DragonOath 的第一主线。

一句话建议：

```text
如果你想在 DragonOath 里选一个主线先试，先 POC UnrealCSharp。
如果 UnrealCSharp 在 UE 5.8 内测上卡住，再 POC PuerTS。
UnLua 只在你明确需要 Lua 或能接受维护风险时再考虑。
```

## DragonOath 的脚本边界

无论选择哪款插件，脚本层都不应该替代 C++ 核心架构。DragonOath 更适合采用：

```text
C++ 定规则和边界
脚本做编排和变化
蓝图做表现和拼装
数据表做配置和内容
```

脚本层适合负责：

- 设置界面、活动界面、商店界面、剧情界面等 UI 逻辑。
- 任务、剧情、引导、活动、运营配置等变化频繁的内容逻辑。
- 非高频的玩法编排，例如副本流程、刷怪波次、关卡事件、NPC 对话。
- DataAsset / DataTable / JSON 等数据驱动内容的读取、校验和轻量处理。
- 蓝图原型逻辑向脚本迁移。
- 编辑器辅助工具、数据检查工具、批量处理工具。

C++ 仍应负责：

- GAS Ability、Attribute、GameplayEffect、GameplayCue 的核心框架。
- 伤害结算、技能命中、网络预测、服务器权威逻辑。
- Actor 生命周期、复制、权限校验、反作弊敏感逻辑。
- 高频 Tick、动画底层、移动、寻路、感知等性能敏感路径。
- CommonGame / CommonUI / GameSettings / GameplayMessageRouter 等基础设施封装。

不建议脚本承担：

- 核心伤害公式的最终权威结算。
- 高频网络复制状态。
- 大量每帧 Tick 的对象逻辑。
- 底层资源加载和生命周期管理。
- 服务器安全边界内的信任逻辑。

## 横向对比

评分说明：

```text
5 = 很适合 / 风险低 / 成熟度高
3 = 可用但需要验证
1 = 风险高 / 不推荐直接上主线
```

| 维度 | PuerTS | UnLua | UnrealCSharp |
| --- | --- | --- | --- |
| DragonOath UE 5.8 适配预期 | 4 | 2 | 4 |
| 公开维护活跃度 | 5 | 2 | 5 |
| 学习成本 | 3 | 4 | 4 |
| UE 反射互操作 | 4 | 5 | 5 |
| 蓝图 / UMG 贴合度 | 4 | 5 | 4 |
| 热重载 / 热更新潜力 | 4 | 4 | 5 |
| 类型系统 | 5 | 2 | 5 |
| 运行时性能可控性 | 4 | 4 | 3 |
| 调试工具链 | 4 | 3 | 5 |
| 多平台和移动端 | 3 | 4 | 4 |
| 社区与资料 | 5 | 3 | 3 |
| 长期维护成本 | 3 | 2 | 4 |
| 对 GAS / Lyra 项目贴合度 | 3 | 4 | 4 |

简化判断：

```text
想要 C#、强类型、活跃维护：UnrealCSharp
想要 TS 工程化、npm 生态、前端式工具链：PuerTS
想要 Lua、UE 事件覆盖、传统游戏热更经验：UnLua，但要接受维护风险
```

## PuerTS

### 定位

PuerTS 是 Unity / Unreal / DotNet 下的脚本编程解决方案。对 Unreal 来说，官方 README 当前明确写到 Unreal 侧支持 JavaScript / TypeScript；Lua / Python 后端主要是 Unity 侧能力。JS / TS 后端可在 V8、QuickJS、Node.js 之间取舍。

它的核心吸引力是 TypeScript 类型系统、声明文件生成、现代 JS 工具链，以及 npm 生态。如果团队里有前端、Node.js、TypeScript 经验，PuerTS 会很顺手。

### 优点

- TypeScript 有静态类型检查，适合做较大规模脚本工程。
- 可以生成声明文件，让脚本侧访问引擎 API 时更容易获得类型提示。
- npm 生态强，代码格式化、Lint、单元测试、模块化管理都比较成熟。
- V8 性能强，QuickJS 包体更小，Node.js API 更完整，可以按平台和需求取舍。
- 官方仓库活跃度高，资料和社区体量比另外两者更大。
- 适合复杂 UI 状态管理、配置校验、活动逻辑和工具脚本。

### 缺点

- Unreal 侧当前按 JS / TS 方案评估，不应误以为 UE 也完整支持 Lua / Python 后端。
- 引入 JS Runtime 后，包体、平台兼容、初始化时机、崩溃栈、性能剖析都会变复杂。
- Node.js 后端能力强，但包体和移动端限制要重点验证。
- TypeScript 编译产物、声明生成、模块加载、资源打包需要额外工程规范。
- 团队如果没有 TS 经验，学习成本会高于 Lua 和 C#。
- 脚本层和 UE 对象生命周期之间需要明确约束，避免 GC、引用保活、异步回调导致难查问题。

### 适合 DragonOath 的场景

- 团队愿意使用 TypeScript 作为长期脚本语言。
- 希望脚本工程可以做类型检查、模块化治理和 CI 检查。
- UI、运营活动、任务、剧情等逻辑规模会比较大。
- 需要复用 npm 生态中的工具库。
- 愿意投入时间做打包、调试、性能和平台适配规范。

### 不适合的情况

- 团队主要是 UE / 蓝图 / C++ 背景，没有 TS 经验。
- 当前最重要目标是快速稳定落地玩法。
- 目标平台包含移动端，但暂时没有时间处理 Runtime、包体和热更新策略。
- 只想做少量简单脚本逻辑，没必要引入完整 JS 工程链。

## UnLua

### 定位

UnLua 是适用于 UE 的 Lua 脚本方案。官方 README 强调它遵循 UE 编程模式，可以直接访问 UCLASS、UPROPERTY、UFUNCTION、USTRUCT、UENUM，不需要胶水代码，并且支持覆盖蓝图实现、处理 Replication / Animation / Input 等事件。

Lua 在游戏行业里的使用历史很长，热更、配置、任务、剧情、活动逻辑都很常见。UnLua 的技术定位本身仍然优秀，但重新评估后，它的问题不是能力，而是维护节奏。

### 优点

- 和 UE 反射系统贴合度高，直接访问常见反射类型。
- 可以覆盖蓝图 Event / Function，适合从蓝图原型平滑迁移。
- 支持 Replication、Animation、Input、Delegate、Latent Function 等常见 UE 场景。
- Lua 语言小，学习快，适合策划和玩法程序共同维护轻量逻辑。
- 游戏行业 Lua 热更经验多，团队更容易找到参考资料和工程范式。
- 有 Lyra with UnLua 示例项目，可以作为理解参考。

### 缺点

- 公开 Release 节奏偏旧，官方 GitHub 当前最新 Release 是 2023-11-07 的 2.3.6。
- 即使 README 写着支持 Unreal Engine 5.x，也不能等同于已经适配 DragonOath 的 UE 5.8 内测版本。
- Lua 缺少 TypeScript / C# 这种强静态类型系统，大规模工程需要额外规范。
- IDE 智能提示和重构能力通常弱于 TS / C#，需要约定目录、注解、Lint 和代码生成。
- Lua 表结构灵活，写法不收敛时容易出现运行期错误。
- 如果后续引擎升级遇到插件问题，可能需要项目组自己维护 C++ 适配。

### 适合 DragonOath 的场景

- 你明确想要 Lua 技术栈。
- 玩法、UI、剧情、任务、活动需要轻量热迭代。
- 团队愿意接受并维护 UnLua 在 UE 5.8 内测上的适配问题。
- 项目只把 Lua 用在低风险编排层，不碰核心战斗和网络权威。

### 不适合的情况

- 当前就要选择维护风险最低的主线脚本方案。
- 团队希望依赖活跃上游快速跟进新引擎。
- 脚本逻辑规模很大，但不准备建立严格工程规范。
- 计划在脚本侧承载复杂底层系统、核心战斗权威逻辑或性能热点。

## UnrealCSharp

### 定位

UnrealCSharp 是 UE 下的 C# 编程插件，官方 README 当前写明基于 .NET 10，并支持反射类型代码生成、静态导出、动态类、调试、Pak 热更新和编辑器热重载。

它当前最值得重视的点是维护节奏。官方 GitHub 当前显示最新 Release 为 2026-06-03 的 v1.2.0。README 标注支持 UE 5.0 - UE 5.7，但考虑到 UE 5.8 目前是内测版本，这个上限不应该被当成明显缺点，而应该被当成 POC 验证项。

### 优点

- C# 类型系统、泛型、LINQ、IDE 重构和调试体验好。
- 对 Unity / .NET 背景开发者学习成本低。
- 官方说明支持全部反射类型自动生成 C# 代码。
- 支持静态导出各种数据类型和函数。
- 支持动态类，可以通过 C# 直接生成 UClass、UInterface、UStruct、UEnum，并且不需要蓝图载体。
- 官方说明支持跨平台、代码调试、Pak 热更新和编辑器热重载。
- 维护节奏活跃，当前更适合作为 DragonOath 的第一 POC 对象。

### 缺点

- DragonOath 使用 UE 5.8 内测版本，仍必须本地验证编译、启动和打包。
- 引入 .NET 10 / CoreCLR / Mono 后，构建链、包体、平台兼容和运行时问题会变复杂。
- 相比 Lua，UE 游戏项目里的 C# 运行时方案普及度较低，遇到项目级问题时可参考案例可能更少。
- 社区体量小于 PuerTS。
- 如果项目核心大量依赖 C#，后续引擎升级、平台适配和插件维护仍需要固定负责人。
- 移动端、主机平台、热更新和应用商店政策需要单独验证。

### 适合 DragonOath 的场景

- 想要强类型脚本语言，而不是 Lua 的动态类型。
- 想要比 TypeScript 更贴近传统游戏程序员的语言体验。
- 希望有更好的 IDE、断点调试、重构和工程组织能力。
- 想用脚本承担 UI、任务、剧情、活动、编辑器工具和非核心玩法编排。
- 愿意把 UE 5.8 适配作为第一轮 POC 的主要目标。

### 不适合的情况

- 团队完全没有 C# / .NET 维护经验。
- 短期没有时间处理 .NET Runtime、打包、平台和热更新细节。
- 项目想直接把核心战斗、网络复制、服务器权威都搬进脚本层。

## 推荐路线

### 阶段 1：先做 UnrealCSharp POC

建议先用 3 至 5 天验证 UnrealCSharp，不直接全项目铺开。

POC 范围：

- 新建独立实验分支，不污染主线。
- 接入 UnrealCSharp 插件并保证 UE 5.8 Editor 能启动。
- 验证 C# 反射代码生成。
- 做一个 C++ 调 C#、C# 调 C++ 的双向调用示例。
- 做一个 UMG 设置界面或测试面板，用 C# 响应按钮、切换选项、读取 DataTable。
- 做一个 Actor / Component 示例，用 C# 处理 BeginPlay、输入、委托和简单事件。
- 做一个 GameplayMessageRouter 消息触发 UI 刷新的示例。
- 做一个 GAS 只读展示示例，例如从 AttributeSet 读取属性并更新 UI。
- 验证编辑器热重载、日志、断点、错误栈和对象释放。
- 打 Windows Development 和 Shipping 包。
- 如果目标有移动端，至少验证 Android 或 iOS 之一。

验收标准：

```text
UE 5.8 Editor 可用
Development 包可用
Shipping 包可用
C++ / 蓝图 / C# 互调稳定
UMG 示例稳定
对象释放无明显泄漏
错误栈可定位到 C# 文件和行号
编辑器热重载流程可复现
不影响现有 Lyra 基础设施
```

如果这个 POC 通过，UnrealCSharp 可以进入第二阶段，作为 DragonOath 的主线脚本候选。

### 阶段 2：同时保留 PuerTS 作为强备选

如果 UnrealCSharp 在 UE 5.8 内测上遇到底层适配问题，或者你更想要 TypeScript / npm 生态，再开 PuerTS POC。

PuerTS POC 应重点验证：

- UE 5.8 编译和启动。
- TypeScript 声明生成。
- TS 编译、模块加载、资源打包路径。
- V8 / QuickJS / Node.js 后端选择。
- Windows + 目标移动平台打包。
- 脚本异常栈、断点调试和性能采样。
- UObject 引用生命周期和异步回调保活。
- UMG / CommonUI / GameplayMessageRouter 示例。
- 包体增量和启动耗时。

如果 PuerTS 被选为主线，必须建立：

- `Scripts/` 目录规范。
- TS 编译配置。
- 模块命名规范。
- 声明文件生成流程。
- Lint / Format / CI 检查。
- 脚本资源 Cook / Pak / 热更新规范。

### 阶段 3：UnLua 仅做谨慎验证

UnLua 不建议现在作为 DragonOath 第一主线。可以在以下情况下做 POC：

- 你明确更喜欢 Lua。
- 你想验证传统 Lua 热更路线。
- 你能接受上游维护节奏偏慢。
- 你愿意自己处理 UE 5.8 内测适配问题。

UnLua POC 应验证：

- UE 5.8 Editor 编译和启动。
- Development / Shipping 打包。
- C++ / 蓝图 / Lua 互调。
- UMG / CommonUI 示例。
- Replication / Input / Delegate / Latent Function 示例。
- Lua 错误栈和热重载。
- UObject 生命周期和 GC。

只有 POC 结果明显好于 UnrealCSharp 和 PuerTS，才考虑重新提高 UnLua 优先级。

## 架构落地建议

推荐目录形态：

```text
Content/
  Script/
    UI/
    Quest/
    Dialogue/
    Activity/
    Dungeon/
    Tutorial/
    Debug/

Source/DragonOath/
  Scripting/
    DragonOathScriptBridge.h
    DragonOathScriptBridge.cpp
```

如果选择 UnrealCSharp，可以进一步规划：

```text
Script/
  DragonOath.Gameplay/
  DragonOath.UI/
  DragonOath.Quest/
  DragonOath.Tools/
```

如果选择 PuerTS，可以进一步规划：

```text
Script/
  src/
    ui/
    quest/
    dialogue/
    activity/
    dungeon/
    bridge/
  generated/
  package.json
  tsconfig.json
```

如果选择 UnLua，可以进一步规划：

```text
Content/
  Script/
    UI/
    GameModes/
    Actors/
    Quest/
    Activity/
```

推荐 C++ 封装：

- 不让脚本直接到处访问复杂子系统，优先通过 `UBlueprintFunctionLibrary` 或专门的 `Subsystem` 暴露稳定 API。
- 对 GAS 暴露只读查询和受控请求，不暴露任意修改 Attribute / GameplayEffect 的能力。
- 对网络请求做服务器校验，脚本只表达意图，不直接决定权威结果。
- 对 UI 提供 ViewModel 或数据快照，避免 UI 脚本直接抓复杂 Actor 引用。
- 对脚本错误建立统一日志分类，例如 `LogDragonOathScript`。

推荐代码边界：

| 类型 | 建议归属 |
| --- | --- |
| 技能释放规则 | C++ / GAS |
| 技能表现编排 | 蓝图 / 脚本 |
| 伤害最终结算 | C++ 服务器权威 |
| UI 按钮响应 | 脚本 / 蓝图 |
| 设置面板逻辑 | 脚本 / C++ 桥接 |
| 任务流程 | 脚本 |
| 剧情对话 | 脚本 + 数据表 |
| 活动规则 | 脚本 + C++ 校验 |
| Boss AI 核心行为 | C++ / BehaviorTree |
| Boss 阶段编排 | 脚本 / 数据 |
| 网络复制 | C++ |
| 反作弊敏感逻辑 | C++ |

## 热更新注意事项

脚本插件常被用来做热更新，但需要区分几个概念：

- Editor 热重载：开发时不重启编辑器即可刷新脚本。
- 本地热加载：运行时从指定路径重新加载脚本文件。
- Pak 热更新：通过补丁包更新脚本资源。
- 线上热更新：发布后向玩家下发内容。

线上热更新不只取决于插件能力，还受以下因素约束：

- Epic / 平台 / 应用商店政策。
- iOS 对可执行代码和脚本下发的审核要求。
- Android 渠道包和补丁策略。
- 服务器协议兼容。
- 资源版本管理。
- 崩溃回滚机制。
- 安全和反作弊策略。

因此，文档或宣传里写“支持热更新”，不等于项目可以不受限制地更新任意代码。DragonOath 应把热更新用于 UI、配置、活动、剧情等低风险逻辑，核心战斗和服务器权威逻辑仍走正常版本发布与服务器校验。

## POC 检查清单

正式决定前，三款插件都应该按同一套清单验证。

基础接入：

- UE 5.8 Editor 编译通过。
- Editor 启动无崩溃。
- 插件启用 / 禁用流程清晰。
- 不污染项目核心模块。
- 与 Live Coding、Hot Reload、Cook 流程没有明显冲突。

打包验证：

- Windows Development 打包通过。
- Windows Shipping 打包通过。
- 目标移动平台打包通过。
- Pak 中脚本资源路径可控。
- 首次启动耗时可接受。
- 包体增长可接受。

互操作验证：

- C++ 调脚本。
- 脚本调 C++。
- 蓝图调脚本或脚本响应蓝图事件。
- UMG / CommonUI 交互。
- GameplayMessageRouter 消息收发。
- DataAsset / DataTable 读取。
- UObject 生命周期和 GC。
- Delegate 绑定和解绑。
- 异步加载或延迟回调。

玩法验证：

- 一个设置界面示例。
- 一个任务或剧情示例。
- 一个活动配置示例。
- 一个 Actor 行为示例。
- 一个 GAS 属性展示示例。
- 一个服务器 / 客户端权限边界示例。

调试验证：

- 日志可定位脚本文件。
- 异常栈可读。
- 断点或等价调试方式可用。
- 性能采样可用。
- 崩溃后能从日志判断脚本上下文。
- 热重载失败时可恢复。

维护验证：

- 有明确目录规范。
- 有最小示例。
- 有编码规范。
- 有脚本 API 白名单。
- 有版本升级记录。
- 有禁用方案和回滚方案。

## 最终建议

当前最合理的路线是：

```text
1. 不急着把脚本插件合进主线。
2. 先用 UnrealCSharp 做 3 至 5 天 POC。
3. POC 只验证 UE 5.8 编译、Editor、打包、C# 互调、UI、消息、数据、Actor、GAS 展示。
4. 如果 UnrealCSharp 满足需求，就作为 DragonOath 第一阶段脚本方案。
5. 如果 UnrealCSharp 卡在 UE 5.8 内测适配或 Runtime 问题，再验证 PuerTS。
6. UnLua 降为谨慎保留，不再作为第一推荐。
```

推荐的主线决策：

```text
DragonOath 先 POC UnrealCSharp。
```

理由：

- 维护和发布节奏当前更活跃。
- C# 类型系统、IDE、调试和工程组织能力强。
- 官方说明支持反射代码生成、动态类、Pak 热更新和编辑器热重载。
- UE 5.7 上限标注在 UE 5.8 内测背景下不能直接判死刑。
- 比 UnLua 更适合当前“跟随新引擎继续升级”的风险控制。
- 比 PuerTS 更适合传统 UE / 游戏程序员的语言习惯。

但这个推荐有前提：

```text
UnrealCSharp 必须先通过 UE 5.8 编译、Editor、Development 包、Shipping 包和目标平台验证。
```

如果 POC 没过，不要强行接入。脚本层是为了提高迭代效率，不应该成为项目基础架构的新风险源。

## 官方资料

- PuerTS GitHub：https://github.com/Tencent/puerts
- PuerTS 文档：https://puerts.github.io
- UnLua GitHub：https://github.com/Tencent/UnLua
- UnLua 功能清单：https://github.com/Tencent/UnLua/blob/master/Docs/CN/Features.md
- UnLua 编程指南：https://github.com/Tencent/UnLua/blob/master/Docs/CN/UnLua_Programming_Guide.md
- Lyra with UnLua：https://github.com/xuyanghuang-tencent/LyraWithUnLua
- UnrealCSharp GitHub：https://github.com/crazytuzi/UnrealCSharp
- UnrealCSharp 文档：https://unrealcsharp.github.io/docs/document/getting-started
