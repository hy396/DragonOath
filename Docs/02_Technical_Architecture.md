# 02. Technical Architecture

## 当前工程状态

项目路径：

```text
D:\ue_texiao\DragonOath
```

引擎版本：

```text
Unreal Engine 5.8
```

当前 `.uproject` 启用插件：

```text
ModelingToolsEditorMode
MCPClientToolset
ModelContextProtocol
AllToolsets
LiveCodingToolset
```

当前项目插件（`Plugins/` 目录下）：

```text
Setly                 Lyra 衍生前端框架（InputConfig、LyraInputComponent、UI Policy 等）
UnrealCSharp          C# 脚本支持（辅助用途，核心逻辑仍走 C++）
GameplayMessageRouter 本地消息总线
AsyncMixin            异步加载/异步任务辅助
CommonLoadingScreen   加载屏和加载过程管理
GameSettings          Common UI 设置界面框架
CommonUser            用户、平台、在线初始化封装
ModularGameplayActors 支持 ModularGameplay/GameFeature 扩展的 Actor 基类
CommonGame            Common UI 层级、UI Manager、LocalPlayer/PlayerController 基础设施
```

已有但暂未启用的插件：GameSubtitles、UIExtension（按需开启）。

当前项目已搭建基础框架。本文档定义后续系统落地时的推荐架构。

## 总体架构原则

DragonOath 采用“服务器权威 + 客户端预测 + 数据驱动配置”的架构。

核心方向：

```text
Gameplay Ability System 负责技能、属性、Buff、伤害
Common UI 负责正式 UI 框架
GameplayMessageRouter 负责本地跨系统事件总线
DataAsset / DataTable 负责技能、怪物、装备、关卡配置
Toolsets / MCP 负责编辑器自动化与 AI 辅助开发
```

## Lyra 基础设施插件

DragonOath 直接纳入以下 Lyra 基础设施插件：

```text
GameplayMessageRouter  本地消息总线
AsyncMixin             异步加载/异步任务辅助
CommonLoadingScreen    加载屏和加载过程管理
GameSettings           Common UI 设置界面框架
CommonUser             用户、平台、在线初始化封装
ModularGameplayActors  支持 ModularGameplay/GameFeature 扩展的 Actor 基类
CommonGame             Common UI 层级、UI Manager、LocalPlayer/PlayerController 基础设施
```

当前暂不纳入：

```text
ShooterCore / ShooterMaps / ShooterExplorer / ShooterTests
TopDownArena
PocketWorlds
UIExtension
GameSubtitles
LyraExampleContent
LyraExtTool
```

原则：

- 借用 Lyra 的通用基础设施，不直接借用 Shooter 玩法内容。
- `CommonGame` 的 `PrimaryGameLayout`、`GameUIManagerSubsystem`、`GameUIPolicy` 作为后续 Common UI 分层参考。
- `GameSettings` 等设置界面开工时再接入具体数据注册表。
- `CommonLoadingScreen` 在副本切换、地图加载和启动流程中使用。
- `CommonUser` 和在线会话能力先纳入工程，但完整大厅/匹配逻辑后续再设计。

## 运行时核心对象

推荐对象关系：

```text
PlayerController
  -> 输入路由、UI 打开关闭、玩家本地控制

PlayerState
  -> 玩家 ASC
  -> 长生命周期玩家数据
  -> 等级、经验、职业、技能点

DOPlayerCharacter
  -> AvatarActor
  -> 移动、动画、碰撞、受击表现
  -> 不保存长期成长数据

AbilitySystemComponent
  -> GameplayAbility
  -> GameplayEffect
  -> GameplayTag
  -> AttributeSet

GameState
  -> 副本阶段、全局目标、Boss 状态

GameMode
  -> 服务器侧关卡规则、刷怪、结算
```

## 本地消息总线架构

DragonOath 使用 `GameplayMessageRouter` 作为本地消息总线。

核心边界：

```text
网络同步 / GAS / RPC / Replicated Data
  -> 产生权威结果
  -> 本机收到结果
  -> GameplayMessageRouter 本地广播
  -> UI / Audio / VFX / Debug 监听
```

它不跨网络，不替代 RPC、属性复制或 GAS。

推荐消息频道：

```text
Message.UI.Inventory.Changed
Message.UI.Skill.CooldownChanged
Message.Combat.Damage.Applied
Message.Combat.Target.Killed
Message.Audio.Music.Changed
Message.VFX.Hit.Fired
Message.Debug.Network.StateChanged
```

推荐使用方式：

- `AttributeSet` 或 ASC Adapter 收到属性变化后，广播本地 UI 消息。
- GameplayCue 到达客户端后，广播本地 VFX / Audio 消息。
- Inventory / SkillTree / Quest 等 replicated data 发生 OnRep 后，广播本地界面刷新消息。
- Common UI / UMG 只监听消息刷新表现，不通过消息修改权威数据。
- Niagara 伤害数字可以监听 `Message.Combat.Damage.Applied` 触发表现。

详细规范见 `Docs/04_Local_Message_Bus.md`。

## ASC 放置策略

玩家：

```text
OwnerActor  = PlayerState
AvatarActor = DOPlayerCharacter
```

理由：

- 玩家死亡、换角色、重生时 ASC 不丢失。
- 技能、属性、Buff 可以跨 Character 生命周期保留。
- 更适合联机项目。

怪物：

```text
OwnerActor  = DOCharacter
AvatarActor = DOCharacter
```

理由：

- 怪物生命周期短。
- 怪物死亡后 ASC 一起销毁即可。
- 后期可针对 AI 使用 Minimal replication 优化。

## GAS 模块设计

建议 C++ 目录：

```text
Source/DragonOath/AbilitySystem/
  Core/
    DOAbilitySystemComponent.h
    DOGameplayTag.h
  Abilities/
    DOGameplayAbility.h
    GA_DO_NormalAttack.h
    GA_DO_DashStrike.h
    GA_DO_RisingSlash.h
    GA_DO_DragonFlame.h
  Attributes/
    DOHealthSet.h
    DOPlaySet.h
  Pipeline/
    GameplayEffectContext
  Executions/
    DODamageExecution.h

Source/DragonOath/Combat/
  DOCombatTypes.h
  DOHitValidation.h

Source/DragonOath/Messages/
  DOGameplayMessages.h
  DOMessageGameplayTags.h
```

当前已有的 AttributeSet：

```text
DOHealthSet    Health / MaxHealth / Damage / Healing
DOPlaySet      Mana / MaxMana / Stamina / MaxStamina / AttackPower / DefensePower
```

后续属性拆分规划见 `Docs/06_Combat_Attribute_Design.md`。

第一阶段 Ability：

```text
GA_DO_NormalAttackCombo
GA_DO_DashStrike
GA_DO_RisingSlash
GA_DO_DragonFlame
```

第一阶段 GameplayEffect：

```text
GE_DO_Damage
GE_DO_ManaCost
GE_DO_Cooldown
GE_DO_Stagger
GE_DO_Invincible
```

## 敌人与 AI 架构

怪物 AI 主线不使用传统 Behavior Tree。

第一阶段推荐结构：

```text
EnemyCharacter
  -> Enemy ASC
  -> Enemy AttributeSet
  -> 简单 C++ 状态逻辑或 StateTree
```

早期目标是先跑通：

```text
Idle
  -> DetectTarget
  -> MoveToTarget
  -> Attack
  -> HitReact
  -> Return
  -> Dead
```

StateTree 适合承担：

- 怪物行为状态切换
- 条件判断
- 技能选择
- Boss 阶段切换
- 可视化调试

Mass + StateTree 适合后续大量怪物或刷怪潮：

```text
Mass
  -> 大量实体的数据组织
  -> 分片更新
  -> 距离 LOD
  -> 群体移动/感知/表现优化

StateTree
  -> 每类怪物的行为阶段
  -> 攻击、追击、返回、硬直等状态任务
```

UE 5.8 的 `MassAI` 模块包含 `MassAIBehavior`、`UMassStateTreeTrait`、`UMassStateTreeSubsystem` 等 Mass + StateTree 集成点，二者相性是可以的。

项目落地顺序：

1. M5 先用 `EnemyCharacter + ASC + StateTree/简单 C++ 状态` 做少量怪物。
2. M6/M8 跑通副本刷怪和 Boss 闭环。
3. 当同屏怪物数量、刷怪潮或性能压力出现后，再引入 Mass + StateTree。

不建议在第一只怪物阶段直接上完整 Mass。Mass 的价值在“规模化实体管理”，不是替代普通怪物 Actor 的全部职责。

## 伤害管线

推荐流程：

```text
Ability 激活
  -> CommitAbility 扣 Cost / CD
  -> 客户端预测播放动画和表现
  -> 客户端生成 TargetData
  -> 发送 TargetData 到服务器
  -> 服务器验证命中距离、方向、目标合法性
  -> 应用 GE_DO_Damage
  -> DODamageExecution 计算最终伤害
  -> 写入 Damage Meta Attribute
  -> AttributeSet::PostGameplayEffectExecute 扣 Health
  -> Health 复制给客户端
  -> UI 更新
```

伤害数字表现：

- 原型或调试阶段可以使用 `WBP_DamageNumber_Debug`。
- 正式版本优先通过 GameplayCue 触发 `NS_DamageNumber`。
- Niagara 伤害数字应尽量使用轻量发射器和对象池/批量参数更新，避免每次伤害都创建重型 Widget。

第一版伤害公式可以简单：

```text
BaseDamage = AttackPower + SkillBaseDamage
MitigatedDamage = max(1, BaseDamage - DefensePower)
FinalDamage = MitigatedDamage * SkillDamageMultiplier
CriticalDamage = FinalDamage * CritDamageRate
```

详细属性设计和伤害公式见 `Docs/06_Combat_Attribute_Design.md`。

后续再加入：

- 元素
- 破防
- 护盾
- 霸体
- 异常状态
- 部位伤害

## 输入架构

推荐使用 Enhanced Input + GameplayTag 激活 Ability。

输入 Tag：

```text
InputTag.Jump
InputTag.Ability.Primary
InputTag.Ability.Secondary
InputTag.Ability.Skill1
InputTag.Ability.Skill2
InputTag.Ability.Skill3
InputTag.Ability.Skill4
InputTag.Ability.Ultimate
InputTag.Ability.Dodge
```

项目 Tag 集中声明在 `AbilitySystem/Core/DOGameplayTag.h`，不要手写 `FGameplayTag::RequestGameplayTag` 字符串。

流程：

```text
InputAction
  -> InputTag
  -> ASC TryActivateAbilitiesByTag
  -> GameplayAbility 激活
```

技能树解锁技能时，为 `FGameplayAbilitySpec` 附加对应输入 Tag。

## 技能树架构

技能树不属于 UI，也不直接属于 Common UI。

推荐组件：

```text
USkillTreeComponent
```

职责：

- 保存技能点
- 保存已解锁节点
- 判断节点能否升级
- 扣除技能点
- 通知 ASC 授予或升级 Ability
- 支持 SaveGame / replicated data

UI 只调用：

```text
CanUpgradeNode(NodeId)
UpgradeNode(NodeId)
GetNodeState(NodeId)
```

数据结构：

```text
SkillNodeId
SkillId
RequiredLevel
RequiredNodeIds
MaxLevel
SkillPointCost
AbilityClass
InputTag
TreePosition
```

## UI 架构

DragonOath 的 UI 分为两类：常驻战斗显示和玩家主动打开的交互界面。

常驻战斗显示可以使用普通 UMG：

```text
WBP_HUD
WBP_SkillBar
WBP_HealthManaBar
WBP_BossHealthBar
```

伤害数字属于战斗表现，不作为正式 UI 层级的一部分：

```text
NS_DamageNumber
WBP_DamageNumber_Debug
```

玩家主动打开的交互界面默认使用 Common UI：

```text
WBP_MainMenuScreen       -> UCommonActivatableWidget
WBP_PauseScreen          -> UCommonActivatableWidget
WBP_InventoryScreen      -> UCommonActivatableWidget
WBP_EquipmentScreen      -> UCommonActivatableWidget
WBP_SkillTreeScreen      -> UCommonActivatableWidget
WBP_SettingsScreen       -> UCommonActivatableWidget
WBP_DungeonResultScreen  -> UCommonActivatableWidget
WBP_ConfirmDialog        -> UCommonActivatableWidget
```

推荐 Common UI 基础类型：

```text
UCommonActivatableWidget
UCommonButtonBase
UCommonActivatableWidgetStack
CommonInputActionDataBase
CommonUIInputData
```

Common UI 负责：

- 页面栈
- 返回逻辑
- 手柄/键鼠输入适配
- 按钮焦点
- 弹窗
- 玩家打开界面的输入模式切换

Common UI 不负责：

- 技能树规则
- 背包规则
- GAS 逻辑
- 装备属性计算

推荐分层：

```text
HUD Layer       常驻战斗信息，普通 UMG 即可
Menu Layer      背包、装备、技能树、设置，Common UI
Modal Layer     确认弹窗、错误提示，Common UI
Debug Layer     开发调试面板，普通 UMG 或编辑器工具均可
```

当 Common UI 界面激活时，应明确处理 gameplay input 的暂停、屏蔽或保留策略，避免玩家打开背包时仍然触发技能。

## 数据资产架构

推荐使用 `PrimaryDataAsset` 管理核心配置。

建议数据类型：

```text
UDA_AbilityDefinition
UDA_SkillTreeDefinition
UDA_EnemyDefinition
UDA_EquipmentDefinition
UDA_DungeonDefinition
UDA_DropTable
```

DataTable 可以用于简单表格，但复杂引用关系更适合 DataAsset。

## 关卡和副本架构

推荐副本管理器：

```text
ADungeonManager
```

职责：

- 读取副本配置
- 控制刷怪波次
- 控制区域推进
- 生成 Boss
- 监听怪物死亡
- 触发通关结算

服务器负责：

- 怪物生成
- 怪物死亡
- 掉落归属
- 结算奖励

客户端负责：

- UI 展示
- 特效表现
- 镜头反馈

## 存档架构

单机/本地测试阶段可以使用 SaveGame。

推荐保存：

```text
职业
等级
经验
技能树节点
装备
背包
当前主线进度
```

联机正式化后，这些数据应迁移到服务器账号数据或后端服务。

## MCP 与 Toolset 架构

当前项目已经启用 UE 5.8 MCP 和 Toolsets。

短期用途：

- 阅读和理解 UE 内置 Toolset 能力
- 使用 MCP 生成 Codex 客户端配置
- 辅助调试 GAS、GameplayTags、UMG

中期可以创建项目专属工具：

```text
DragonOathToolset
  ListAbilities
  ValidateAbilityDefinitions
  ValidateGameplayTags
  ValidateSkillTree
  RunCombatAutomationTests
```

这些工具可以让 AI 更直接地检查项目数据，而不是只读源码。

## 测试策略

第一阶段最低测试：

- PIE 单客户端运行
- PIE Listen Server + 1 Client
- 普攻三段同步
- 技能冷却同步
- Health/Mana UI 更新
- 怪物死亡同步
- Boss 击败结算

后续增加：

- 自动化测试
- GAS Ability 激活测试
- GameplayTag 一致性检查
- DataAsset 校验工具

## 架构风险

当前主要风险：

- 过早做太多系统导致闭环迟迟不能玩。
- 后期才修网络权限导致大返工。
- 技能树和 UI 混在一起导致维护困难。
- 第一只怪物就接入完整 Mass，导致 AI 原型过重。
- 伤害公式过早复杂化。
- 美术资源未分 Prototype / Production，导致临时资源污染正式工程。

应对方式：

- 第一阶段只做一个职业、一个关卡、一个 Boss。
- 所有核心玩法在 Listen Server + Client 下验证。
- UI 只展示和输入，不拥有规则。
- 怪物 AI 先用 StateTree/简单状态跑通，Mass 留给同屏数量和刷怪潮优化。
- GAS 和 DataAsset 先建立小而稳定的结构。
