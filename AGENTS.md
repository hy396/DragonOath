# AGENTS.md

本文件是 AI Agent 的快速上下文入口。完整规范见 `Docs/` 目录。

## 项目概要

- 项目名：DragonOath（龙途）
- 引擎版本：Unreal Engine 5.8
- 项目路径：`D:\ue_texiao\DragonOath`
- 游戏类型：原创幻想动作 RPG，二次元风格，联机架构
- 主开发语言：C++ + Blueprint，数据驱动配置用 DataAsset

## 构建命令

引擎路径：`D:\UE_5.8`

```powershell
# 生成项目文件（修改 .uplugin / Build.cs 后需要执行）
& "D:\UE_5.8\Engine\Build\ProjectFiles\ProjectFileGenerator.exe" "D:\ue_texiao\DragonOath\DragonOath.uproject"

# 编译（Development Editor）
& "D:\UE_5.8\Engine\Build\Build\Build.exe" DragonOathEditor Win64 Development -Project="D:\ue_texiao\DragonOath\DragonOath.uproject" -WaitMutex

# 启动编辑器
& "D:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "D:\ue_texiao\DragonOath\DragonOath.uproject"
```

## 启用插件

引擎插件：CommonUI、EnhancedInput、GameplayAbilities、StateTree、GameplayStateTree

项目插件（`Plugins/` 目录下）：
- GameSubtitles、UIExtension（已有但暂未启用，按需开启）
- Setly — Lyra 衍生的前端框架插件，提供 InputConfig、LyraInputComponent、UI Policy 等
- UnrealCSharp — C# 脚本支持（当前主要用于辅助，核心逻辑仍走 C++）
- GameplayMessageRouter — 本地消息总线
- AsyncMixin、CommonLoadingScreen、GameSettings、CommonUser、ModularGameplayActors、CommonGame — Lyra 基础设施

## 源码结构

```
Source/DragonOath/
  AbilitySystem/
    Core/           DOAbilitySystemComponent, DOGameplayTag
    Abilities/
      Core/         技能基类与配置资产：DOGameplayAbility、DOAbilitySet、DOProfessionAbilityConfig
      Common/       全职业通用基础动作：普攻、冲刺/闪避、闪避攻击
      Combat/       职业专属主动战斗技能（战技、大招）——规划中
      Buff/         辅助技能：增益/治疗/护盾（pose + VFX + GE）——规划中
      Passive/      被动技能——规划中
    Attributes/     DOHealthSet, DOResourceSet, DOCombatSet
    Pipeline/       GameplayEffectContext
  Characters/       DOCharacter 基类（玩家/怪物通用）
  Player/           DOPlayerState, DOPlayerCharacter, DOPlayerController
```

不使用顶层 `Public/Private`，按功能域组织（Lyra 风格）。详见 `Docs/01_Development_Standards.md`。

### Abilities 分类约定

`AbilitySystem/Abilities/` 下按"是否绑定职业 + 功能类型"分子目录：

- `Core/`：技能基类与配置资产（`DOGameplayAbility`、`DOAbilitySet`、`DOProfessionAbilityConfig`），不参与具体技能分类。
- `Common/`：全职业通用基础动作套件（普攻、冲刺/闪避、闪避攻击）。判定标准：每个职业都有、逻辑通用。
- `Combat/`：职业专属主动战斗技能（战技、大招）。判定标准：某职业专属、有独特机制。
- `Buff/`：辅助技能（增益/治疗/护盾），通常只是摆 pose + 特效 + GE，无弹道/命中逻辑。
- `Passive/`：被动技能，靠 `ActivationOwnedTags` + 常驻 GE，无激活流程。

`Common` 与 `Combat` 的区分维度是"是否绑定职业"，不是"是否战斗"——普攻虽是战斗动作但人人都有，故归 `Common`。`Combat/`、`Buff/`、`Passive/` 目前为空，待首个对应技能创建时再建目录。

## 关键架构约定

### ASC 放置

- 玩家 ASC 在 `ADOPlayerState` 上，Avatar 指向当前 `ADOPlayerCharacter`
- 怪物 ASC 挂在 `ADOCharacter` 自身上
- 玩家 ASC 使用 `Mixed` replication mode

### GAS 技能体系

- 技能基类 `UDOGameplayAbility`：默认 `InstancedPerActor` + `LocalPredicted`
- `CanActivateAbility` 拦截 `Level <= 0` 的技能（配合职业技能配置的 0 级未学习机制）
- `EDOAbilityActivationPolicy`：`OnInputTriggered` / `WhileInputActive` / `OnSpawn`
- ASC 输入流程：`AbilityInputTagPressed` -> 缓存 SpecHandle -> `ProcessAbilityInput` 统一激活
- `AbilityInputTagPressed` 中已过滤 `Level <= 0` 的技能

#### GameplayAbility 内置 Tag 容器（10 个）

UGameplayAbility 内置 10 个 `FGameplayTagContainer`，按功能分为五大类：

**一、技能自身标识标签**

| 容器名 | 蓝图显示 | 作用 |
|---|---|---|
| `AbilityTags` | Asset Tags | 技能本身的身份标签，用于识别、筛选、查找技能资源 |

**二、技能互斥/中断控制**

| 容器名 | 蓝图显示 | 作用 |
|---|---|---|
| `CancelAbilitiesWithTag` | Cancel Abilities With Tag | 激活时强制取消其他正在运行的同类技能 |
| `BlockAbilitiesWithTag` | Block Abilities With Tag | 激活期间禁止其他同类技能启动 |

**三、角色自身释放校验**

| 容器名 | 蓝图显示 | 作用 |
|---|---|---|
| `ActivationRequiredTags` | Activation Required Tags | 施法者 ASC 必须全部拥有这些标签才能释放（AND） |
| `ActivationBlockedTags` | Activation Blocked Tags | 施法者 ASC 只要拥有任意一个标签就禁止释放（OR） |
| `ActivationOwnedTags` | Activation Owned Tags | 技能激活时自动添加，结束时自动移除，默认网络同步 |

**四、施法源（Source）附加校验**

| 容器名 | 作用 |
|---|---|
| `SourceRequiredTags` | 施法源 ASC 必须全部持有这些标签才能释放 |
| `SourceBlockedTags` | 施法源 ASC 只要持有任意一个标签就阻断释放 |

**五、目标（Target）命中校验**

| 容器名 | 作用 |
|---|---|
| `TargetRequiredTags` | 目标 ASC 必须全部持有这些标签，技能才能生效 |
| `TargetBlockedTags` | 目标 ASC 只要持有任意一个标签，技能无法作用 |

**判定执行顺序**：
1. 校验 Source/Activation 释放条件
2. 激活时执行 Cancel/Block 互斥逻辑
3. 施加 ActivationOwnedTags
4. 命中时校验 Target 标签

**示例用法**：

```cpp
// 冲刺技能：激活期间自动添加 Dashing 标签
ActivationOwnedTags.AddTag(DragonOathGameplayTags::Status::Dashing);

// 冲刺中不能再冲刺
ActivationBlockedTags.AddTag(DragonOathGameplayTags::Status::Dashing);

// 冲刺激活时取消正在进行的攻击
CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack")));

// 大招期间禁止冲刺
BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Move.Dash")));
```

**不要做的事**：

- 不要手动用 GE 施加技能激活期间的临时状态标签，用 `ActivationOwnedTags`
- 不要在 `CanActivateAbility` 中手动检查状态标签，用 `ActivationBlockedTags`
- 不要手写 `FGameplayTag::RequestGameplayTag` 字符串，引用 `DOGameplayTag.h` 中的变量

### 输入系统

- Enhanced Input + GameplayTag
- `DA_InputConfig` 资产配置 InputAction -> GameplayTag 映射
- `IMC_Default` 输入映射上下文
- 蓝图中用 `ULyraInputComponent::BindAbilityActions` 绑定技能输入

### GameplayTag 管理

- 项目 Tag 集中声明在 `DOGameplayTag.h` / `.cpp`
- 已有命名空间：`Gameplay`、`Event`、`InputTag`、`Profession`
- 不要直接手写 `FGameplayTag::RequestGameplayTag` 字符串，引用 C++ 变量

### 网络架构

- 从第一天按联机架构写
- 服务器权威：伤害、死亡、掉落、经验由服务器决定
- 玩家技能优先 `LocalPredicted`
- 测试要求：PIE Listen Server + 1 Client 起步

## 命名规范

- C++ 类前缀：`A` Actor、`U` UObject/Component、`F` Struct、`I` Interface、`E` Enum
- 项目类前缀用 `DO` 而非 `DragonOath`：`UDOAbilitySystemComponent`、`ADOPlayerCharacter`
- 资产前缀：`GA_` Ability、`GE_` Effect、`WBP_` Widget、`DA_` DataAsset、`DT_` DataTable
- 蓝图技能名：`GA_DO_技能名`

## 文档索引

| 文档 | 内容 |
|---|---|
| `Docs/01_Development_Standards.md` | Git、目录、命名、C++/BP 分工、网络、GAS、AI、UI 规范 |
| `Docs/02_Technical_Architecture.md` | 运行时对象关系、ASC 放置、GAS 模块设计、伤害管线、技能树、存档 |
| `Docs/03_Milestone_Roadmap.md` | 里程碑路线图 |
| `Docs/04_Local_Message_Bus.md` | GameplayMessageRouter 使用规范 |
| `Docs/06_Combat_Attribute_Design.md` | 战斗属性设计 |
| `Docs/ProfessionAbilityConfigDesign.md` | 职业技能配置方案 |

## AI Agent 注意事项

1. **不要创建顶层 Public/Private 目录**，按功能域放在 `Source/DragonOath/` 下
2. **不要用 Behavior Tree** 做 AI，项目主线是 StateTree，后期引入 Mass
3. **不要用枚举做职业标识**，使用 GameplayTag（`Profession.*`）
4. **不要在 Blueprint 里写服务器权威逻辑**，战斗结果必须由 C++ 或 GAS 流程决定
5. **不要直接修改 `Binaries/`、`Intermediate/`、`Saved/`、`DerivedDataCache/`**
6. **修改 .h 文件后可能需要重新生成项目文件**，修改 .cpp 可用 Live Coding 热重载
7. **GameplayTag 注释用中文**，`UE_DEFINE_GAMEPLAY_TAG_COMMENT` 第三个参数是注释
8. **代码注释用中文**，文档正文用中文，文档标题用英文编号

## Spec Kit

<!-- SPECKIT START -->
For additional context about technologies to be used, project structure,
shell commands, and other important information, read the current plan
<!-- SPECKIT END -->

Spec Kit 工具链生成的设计文档（`spec.md`、`plan.md`、`tasks.md`）如存在，应放在 `.specify/` 目录下。当前未使用 Spec Kit 工作流。
