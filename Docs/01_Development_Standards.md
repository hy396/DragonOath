# 01. Development Standards

## Git 规范

当前项目已经启用 Git。

基础规则：

- 不提交 `Binaries/`、`Intermediate/`、`Saved/`、`DerivedDataCache/`。
- 不提交个人编辑器缓存，例如 `.vs/`、`.vscode/`、`.idea/`。
- 重要代码和文档变更应形成清晰提交。
- 提交信息使用英文短句，描述本次变更目的。
- 不把临时美术、测试截图、日志、崩溃报告提交到仓库。

推荐提交粒度：

```text
Add GAS attribute foundation
Add Common UI skill bar prototype
Document first playable milestone
Configure gameplay tags for combat states
```

不推荐提交信息：

```text
fix
update
123
临时
不知道改了啥
```

## 目录规范

DragonOath 主游戏模块采用 Lyra 风格的“功能域目录”。

参考项目结论：

- LyraGame 官方项目：主模块根目录按 `AbilitySystem`、`Characters`、`Inventory`、`Messages`、`UI` 等功能域组织，不强制模块级 `Public/Private`。
- MMOARPG：模块根目录下按 `Core`、`UI` 等功能域组织，路径短，适合项目型代码。
- Aura：`Public/Private` 镜像分层很规整，适合教学和库式模块，但路径会更长。
- Crunch：只使用 `Private` 也能保持封装，但公共 API 边界不如 Lyra 式功能域直观。

当前决定：

- `Source/DragonOath/` 不建立顶层 `Public/Private`。
- 头文件和 cpp 可以按功能域放在同一目录或相邻子目录。
- 目录按系统职责命名，不按“类类型”堆叠。
- 不使用 UE4 旧式 `Classes/` 目录。
- 后续如果创建可复用插件、Runtime/Editor 拆分模块，插件或独立模块内部可以再使用 `Public/Private`。

正式 C++ 目录建议：

```text
Source/DragonOath/
  DragonOath.Build.cs
  DragonOathModule.cpp
  AbilitySystem/
    Core/
      DOGameplayTag.h
      DOGameplayTag.cpp
    Abilities/
    Attributes/
    Pipeline/
    Executions/
    Tasks/
  Animation/
  Audio/
  Camera/
  Characters/
  Combat/
  Enemy/
    AI/
    Boss/
    Spawning/
  Equipment/
  Feedback/
    DamageNumbers/
    HitImpact/
  GameModes/
  Input/
  Interaction/
  Inventory/
  Items/
  Messages/
  Player/
  SaveGame/
  Settings/
  System/
  UI/
    Common/
    HUD/
    Screens/
    Widgets/
```

目录使用原则：

- `AbilitySystem/` 放 GAS 基类、Ability、AttributeSet、ExecutionCalculation、AbilityTask。
- `Combat/` 放命中验证、伤害数据、战斗通用类型，不直接承载 GAS 全部实现。
- `Feedback/` 放命中反馈、伤害数字、镜头震动、局部表现协调。
- `Messages/` 放本地消息总线 Payload、消息 Tag 声明和消息适配器。
- `UI/` 放 Widget、Common UI 页面、HUD Adapter，不保存玩法权威数据。
- `System/` 放 AssetManager、GameInstanceSubsystem、全局初始化等系统级代码。
- `Tests/` 放自动化测试或测试辅助，不被正式玩法依赖。

正式内容目录建议：

```text
Content/DragonOath/
  Abilities/
  Audio/
  Characters/
    Heroes/
    Enemies/
  Effects/
  Data/
  Maps/
  Materials/
  UI/
    Common/
    HUD/
    Screens/
  VFX/
    Niagara/
```

临时资源目录建议：

```text
Content/_Prototype/
Content/_Temp/
```

临时目录里的资源不能长期作为正式系统依赖。

## 命名规范

C++ 类名前缀遵循 UE 习惯：

```text
A  Actor
U  UObject / Component / Subsystem
F  Struct
I  Interface
E  Enum
```

项目类命名统一使用 `DO` 前缀（而非 `DragonOath`）：

```text
ADOPlayerCharacter
ADOCharacter
UDOAbilitySystemComponent
UDOGameplayAbility
UDOHealthSet
UDOPlaySet
UDOInventoryComponent
UGA_DO_NormalAttack
UGA_DO_DashStrike
UGE_DO_Damage
```

GameplayTag 命名建议：

```text
InputTag.Jump
InputTag.Ability.Primary
InputTag.Ability.Skill1
InputTag.Ability.Dodge
Ability.Attack.Normal
Ability.Skill.DashStrike
State.Combat.Stunned
State.Combat.Invincible
Event.Death
Gameplay.AbilityInputBlocked
Data.Damage
Data.Cost.Mana
Profession.DragonFighter
```

项目 Tag 集中声明在 `AbilitySystem/Core/DOGameplayTag.h`，不要手写 `FGameplayTag::RequestGameplayTag` 字符串。

资产命名建议：

```text
GA_       GameplayAbility
GE_       GameplayEffect
GCN_      GameplayCueNotify
WBP_      Widget Blueprint
DA_       DataAsset
DT_       DataTable
MI_       Material Instance
M_        Material
NS_       Niagara System
SM_       Static Mesh
SK_       Skeletal Mesh
ABP_      Animation Blueprint
```

## C++ 与 Blueprint 分工

C++ 负责：

- 系统架构
- 网络权限
- GAS 初始化
- AttributeSet
- GameplayAbility 基类
- 伤害计算
- 背包/装备核心数据
- 副本流程状态机
- 存档结构

Blueprint 负责：

- 技能参数配置
- 动画和特效挂接
- UI 视觉布局
- 简单关卡脚本
- 数据资产配置
- 原型验证

不要把服务器权威逻辑只写在 Blueprint 里。Blueprint 可以配置和表现，但关键战斗结果必须由服务器侧 C++ 或明确的 GAS 流程决定。

## 网络开发规范

项目从第一天按联机架构写。

基础原则：

- 服务器决定伤害、死亡、掉落、经验和关卡推进。
- 客户端负责输入、预测和表现。
- 玩家 Ability 优先使用 `LocalPredicted`。
- 纯服务器逻辑使用 `ServerOnly` 或服务器侧系统触发。
- `CanActivateAbility` 的客户端和服务器判断必须尽量一致。
- TargetData 必须做服务器验证。
- UI 不直接相信客户端本地临时值，关键状态来自 ASC / PlayerState / replicated data。

第一阶段测试要求：

```text
PIE: Listen Server + 1 Client
PIE: Listen Server + 2 Clients
```

每个核心玩法都要至少在 Listen Server + Client 下验证一次。

## 本地消息总线规范

项目使用 `GameplayMessageRouter` 作为本地消息总线。

基础原则：

- 它只负责同一台机器、同一个运行实例内的消息广播。
- 它不负责服务器到客户端同步，也不负责客户端到服务器请求。
- 服务器权威状态仍然走 GAS、RPC、Replicated Property、PlayerState、GameState 或后端服务。
- 客户端收到复制结果或 GameplayCue 后，可以再 Broadcast 本地消息给 UI、Audio、VFX、Debug 系统。
- Payload 使用 `USTRUCT(BlueprintType)`。
- 频道使用 GameplayTag，统一以 `Message.` 开头。
- 不用它广播 Tick 级高频数据。
- 不用它承载“请求服务器执行”的操作。

推荐流程：

```text
网络/GAS/复制结果到达本机
  -> Broadcast GameplayMessage
  -> 本地 UI / VFX / Audio / Debug 监听并表现
```

## GAS 规范

GAS 是 DragonOath 的核心战斗框架。

建议：

- 玩家 ASC 放在 PlayerState 上，Avatar 指向当前 Character。
- 怪物 ASC 可放在 Character 上。
- 玩家 ASC 使用 Mixed replication。
- AI/怪物 ASC 后期可考虑 Minimal replication。
- 伤害使用 Meta Attribute，例如 `Damage`。
- 复杂伤害用 ExecutionCalculation。
- 消耗和简单动态数值可用 MMC 或 SetByCaller。
- 视觉表现优先使用 GameplayCue。

当前 AttributeSet：

```text
DOHealthSet
  Health
  MaxHealth
  Damage
  Healing

DOPlaySet
  Mana
  MaxMana
  Stamina
  MaxStamina
  AttackPower
  DefensePower
```

后续属性拆分规划见 `Docs/06_Combat_Attribute_Design.md`。

## AI 规范

怪物 AI 不以传统 Behavior Tree 作为默认方案。

建议路线：

- 早期普通怪和 Boss 原型可以使用简单 C++ 状态逻辑或 StateTree。
- 当需要可视化编排索敌、靠近、攻击、返回、硬直、死亡等状态时，优先使用 StateTree。
- 后续怪物数量增加、需要群体刷新或低成本更新时，再引入 Mass + StateTree。
- Mass 负责大量实体的数据组织、分片更新、LOD 和批处理。
- StateTree 负责怪物行为阶段、条件切换和任务执行。
- Behavior Tree 只作为参考或兼容方案，不作为项目主线 AI 架构。

不要为了第一只怪物过早接入完整 Mass 框架。Mass 更适合“很多小怪同时存在”的阶段，而不是最初验证战斗手感的阶段。

## UI 规范

玩家主动打开的界面默认使用 Common UI。原因是这类界面通常需要返回、关闭、层级导航、焦点管理、手柄/键鼠输入适配和弹窗栈，纯 UMG 手写返回逻辑后期维护成本较高。

优先使用 Common UI 的界面：

- 主菜单
- 暂停菜单
- 设置界面
- 背包
- 装备界面
- 技能树
- 任务/副本面板
- 结算界面
- 确认弹窗

可以继续使用普通 UMG 的界面：

- 常驻战斗 HUD
- 血条 / 蓝条
- 伤害数字原型（正式版本优先 Niagara 飘字）
- Boss 血条
- 冷却遮罩
- 临时调试面板

技能栏如果只是常驻显示，可以先用普通 UMG；如果要支持手柄焦点、技能选择、拖拽换位、打开详情面板，则应迁移到 Common UI。

Common UI 只负责 UI 导航、焦点、输入路由、返回操作和界面层级，不负责技能树、背包、GAS 等核心逻辑。

## 文档规范

正式文档在 `Docs/` 下维护。

- 新系统开工前，应先写设计说明。
- 重大架构变更必须更新文档。
- 文档标题使用英文编号，正文使用中文。
- 旧实验文档必须明确标记，不得和正式规范混用。

## 美术资源规范

当前美术资产优先服务于原型验证。

早期资源分级：

```text
Prototype  只用于手感验证
Draft      可用于阶段演示
Production 正式资产
```

你偏好的美术方向：

- 二次元美少女角色
- 原创幻想动作 RPG
- 清晰技能图标
- 页游式但不照搬旧作的 UI 气质

正式资源必须原创，不能直接复用旧游戏素材。
