# UE 5.8 MCP 功能笔记

日期：2026-06-09
项目：DragonOath
来源：本机 UE 5.8 源码 `D:\UE_5.8`

## 1. 结论

UE 5.8 里的 MCP 不是一个单独的游戏功能，而是一套编辑器/工具协议集成。它的核心作用是让 AI 工具和 Unreal Editor 互相调用能力。

当前项目启用的相关插件包括：

- `ModelContextProtocol`
- `MCPClientToolset`
- `AllToolsets`
- `LiveCodingToolset`

它们可以分成两个方向理解：

1. UE 作为 MCP Server：让外部 AI 客户端调用 Unreal Editor 暴露出来的工具。
2. UE 作为 MCP Client：让 Unreal Editor 连接本地或私有 MCP Server，并把外部工具注册进 UE 的 Toolset Registry。

## 2. ModelContextProtocol

插件路径：

```text
D:\UE_5.8\Engine\Plugins\Experimental\ModelContextProtocol
```

`.uplugin` 描述：

```text
FriendlyName: Unreal MCP
Description: Anthropic MCP (Model Context Protocol) server implementation for Unreal Engine.
IsExperimentalVersion: true
```

也就是说，它是 Unreal Engine 内置的 MCP Server 实现。它通过 HTTP 暴露 MCP 协议接口。

默认服务地址：

```text
http://localhost:8000/mcp
```

默认配置来自：

```cpp
ServerUrlPath = "/mcp";
ServerPortNumber = 8000;
bAutoStartServer = false;
```

对应设置类：

```text
UModelContextProtocolSettings
```

配置位置：

```text
Editor Preferences > Plugins > Model Context Protocol
```

## 3. ModelContextProtocol 能提供什么

从公开接口看，它支持：

- 注册 MCP Tools
- 移除 MCP Tools
- 查找已注册 Tool
- 刷新 Tool 列表
- 启动/停止 MCP HTTP Server
- 注册 Resource Provider
- 列出 Resource
- 读取 Resource
- 输出文本、图片、音频、结构化 JSON、资源链接等 Tool Result

核心接口：

```cpp
IModelContextProtocolModule::AddTool()
IModelContextProtocolModule::RemoveTool()
IModelContextProtocolModule::RefreshTools()
IModelContextProtocolModule::StartServer()
IModelContextProtocolModule::StopServer()
IModelContextProtocolModule::AddResourceProvider()
```

Tool 接口：

```cpp
IModelContextProtocolTool
```

一个 Tool 至少要提供：

```cpp
GetName()
GetDescription()
GetInputJsonSchema()
Run() 或 RunAsync()
```

也就是说，可以把 UE 里的某些编辑器能力包装成 AI 可调用的 MCP 工具。例如：

- 查询当前关卡 Actor
- 创建资源
- 修改 DataAsset
- 扫描 GameplayTags
- 验证 GAS Ability 配置
- 运行自动化测试
- 读取项目资源信息

## 4. 控制台命令

UE 5.8 MCP 插件提供了这些控制台命令：

```text
ModelContextProtocol.StartServer
ModelContextProtocol.StartServer 8000
ModelContextProtocol.StopServer
ModelContextProtocol.RefreshTools
ModelContextProtocol.GenerateClientConfig Codex
ModelContextProtocol.GenerateClientConfig All
```

说明：

- `StartServer`：显式启动 MCP HTTP Server。
- `StopServer`：停止 MCP Server。
- `RefreshTools`：清空并重新注册工具列表。
- `GenerateClientConfig`：生成外部 MCP 客户端配置，支持 `ClaudeCode`、`Cursor`、`VSCode`、`Gemini`、`Codex`、`All`。

## 5. MCPClientToolset

插件路径：

```text
D:\UE_5.8\Engine\Plugins\Experimental\Toolsets\MCPClientToolset
```

`.uplugin` 描述：

```text
FriendlyName: MCP Client Toolset
Description: An adapter that allows toolset registry customers (like the EDA) to connect to local/private MCP servers.
EditorOnly: true
IsBetaVersion: true
IsExperimentalVersion: true
```

这个插件的方向和 `ModelContextProtocol` 相反：

- `ModelContextProtocol`：UE 暴露工具给外部 AI。
- `MCPClientToolset`：UE 连接外部 MCP Server，把外部工具变成 UE Toolset Registry 的工具。

配置类：

```text
UMCPToolsetSettings
```

配置位置：

```text
Editor Preferences > Plugins > MCP Toolset Servers
```

每个外部 MCP Server 配置包含：

```text
Name
Description
ServerUrl
ApiKey
bEnabled
Transport
Auth
OAuthClientId
OAuthScope
```

支持的 Transport：

```text
Legacy SSE (HTTP+SSE)
Streamable HTTP
```

支持的认证方式：

```text
None
Bearer Token (API Key)
OAuth 2.0 Authorization Code + PKCE
```

它在编辑器启动时读取配置，创建 `FMCPClientToolset`，并注册到 `UToolsetRegistrySubsystem`。

## 6. AllToolsets

插件路径：

```text
D:\UE_5.8\Engine\Plugins\Experimental\Toolsets\AllToolsets
```

它是一个聚合插件，会启用一批 Toolset 插件。

当前包括：

```text
AIModuleToolset
AnimationAssistantToolset
AutomationTestToolset
ConversationToolset
DataflowAgent
GameFeaturesToolset
GameplayTagsToolset
GASToolsets
MCPClientToolset
NiagaraToolsets
PhysicsToolsets
SlateInspectorToolset
StateTreeToolset
UMGToolSet
WorldConditionsToolset
```

对 DragonOath 比较有价值的有：

- `GASToolsets`：辅助 GAS 相关检查和工具化。
- `GameplayTagsToolset`：辅助 GameplayTag 查询、维护、验证。
- `UMGToolSet`：辅助 UI 资源和控件相关操作。
- `GameFeaturesToolset`：如果后续用 Game Feature Plugin，可以辅助配置。
- `AutomationTestToolset`：后续做自动化测试时有用。
- `MCPClientToolset`：接入外部私有 MCP 工具。

## 7. LiveCodingToolset

插件路径：

```text
D:\UE_5.8\Engine\Plugins\Experimental\Toolsets\LiveCodingToolset
```

`.uplugin` 描述：

```text
Description: Live Coding compile toolset.
EditorOnly: true
IsExperimentalVersion: true
```

它把 Live Coding 编译能力注册进 Toolset Registry。源码里说明可以通过 CVar 开关：

```text
LiveCodingToolset.Enable
```

作用：让支持 Toolset Registry 的 AI/工具入口触发或管理 Live Coding 编译相关操作。

## 8. 对 DragonOath 的实际价值

对 DragonOath 来说，MCP/Toolset 最大价值不是直接做玩法，而是提高编辑器自动化能力。

可以用在这些方向：

1. GAS 工具化

```text
检查 Ability 是否配置 InputTag
检查 Cooldown/Cost GE 是否缺失
扫描 AttributeSet 字段
检查 GameplayEffect 是否绑定 ExecCalc
验证技能树数据和 Ability 是否一致
```

2. GameplayTags 管理

```text
列出项目 GameplayTags
查找未使用 Tag
检查技能、Buff、Debuff、Input Tag 命名是否规范
```

3. UMG/UI 辅助

```text
检查 Common UI Widget 配置
扫描技能栏 Widget
检查按钮是否绑定 InputAction
```

4. 自动化测试

```text
运行编辑器 Automation Tests
整理测试结果
检测 GAS 网络预测/同步问题
```

5. 编辑器资源操作

```text
读取资源列表
生成或修改 DataAsset
创建技能配置资产
批量检查资源命名
```

## 9. 它不是什么

MCP 不是：

- 不是自动生成游戏的插件
- 不是替代 C++/蓝图的系统
- 不是直接生成美术资产的系统
- 不是网络联机框架
- 不是 GAS 本身

它更像是：

```text
AI <-> Unreal Editor 的工具调用桥梁
```

只有当 UE 侧已经注册了 Tool，或者你写了自己的 Tool，AI 才能调用对应能力。

## 10. 安全注意事项

因为 MCP 能让外部客户端调用 UE 工具，所以要注意：

- 不要把 MCP Server 暴露到公网。
- 默认建议只监听本机 `localhost`。
- 不要随便连接不可信 MCP Server。
- Bearer Token / OAuth Token 不要提交到 Git。
- `MCP Toolset Servers` 配置属于 `EditorPerProjectUserSettings`，更偏本机个人配置。
- 当前这些插件是 Experimental/Beta，接口后续可能变。

## 11. 建议用法

当前阶段建议：

1. 保留项目里的 MCP 插件启用项。
2. 不要急着写自定义 MCP Tool。
3. 先用 `AllToolsets` 和 `GASToolsets` 探索 UE 内置工具能力。
4. 等 DragonOath 的 GAS 框架成型后，再写项目专属 MCP Tool。

后续可以做一个 `DragonOathToolset`，暴露这些能力：

```text
ListDragonOathAbilities
ValidateAbilityInputTags
ValidateSkillTreeNodes
ListGameplayTagsByPrefix
CreateGameplayAbilityDataAsset
RunGASAutomationTests
```

这样 AI 就能更直接地辅助项目开发，而不是只靠读文件和猜结构。

## 12. 一句话总结

UE 5.8 的 MCP 是编辑器自动化入口。它能让 AI 调用 UE 工具，也能让 UE 连接外部 MCP 工具。对 DragonOath 最有价值的方向，是把 GAS、GameplayTags、UMG、自动化测试和资源配置变成可被 AI 调用的工具链。
