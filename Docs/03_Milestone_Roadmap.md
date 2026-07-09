# 03. Milestone Roadmap

## 当前阶段

DragonOath 当前处于 M1-M2 之间：基础工程骨架和 GAS 基础已部分完成。

已完成：

- UE 5.8 工程搭建
- Git 初始化和 `.gitignore`
- MCP/Toolset 插件启用
- 基础开发文档整理
- Lyra 基础设施插件纳入（Setly、GameplayMessageRouter 等）
- C++ 目录结构搭建（AbilitySystem/Core、Abilities、Attributes、Pipeline、Characters、Player）
- `UDOAbilitySystemComponent`、`UDOGameplayAbility` 基类
- `DOHealthSet`、`DOPlaySet` 属性集
- `DOGameplayTag` 集中 Tag 声明
- `ADOPlayerState`、`ADOPlayerCharacter`、`ADOPlayerController`、`ADOCharacter` 基类
- Enhanced Input + GameplayTag 输入框架

下一阶段目标是建立可以长期扩展的最小技术骨架，完成 M2 剩余任务并进入 M3 基础战斗。

## M0：项目规范阶段

目标：让项目在正式写玩法前具备清晰规则。

任务：

- 整理项目定位文档
- 整理开发规范
- 整理技术架构
- 整理阶段路线
- 确认 Git 忽略规则
- 确认 Spec Kit / MCP 文档

验收：

- `Docs/` 下存在正式文档索引
- 旧实验文档不再作为正式依据
- Git 工作区干净

## M1：基础工程骨架

目标：建立 C++ 目录、基础类和输入框架。

任务：

- 按 Lyra 式功能域创建 `Source/DragonOath/` 目录
- 确认 Lyra 基础设施插件可编译
- 创建玩家 Character 基类
- 创建 PlayerState 基类
- 创建 PlayerController 基类
- 创建 GameMode / GameState 基类
- 创建 Enhanced Input 配置
- 创建基础地图
- 创建基础 HUD 占位
- 建立 `Message.*` GameplayTag 命名规则

验收：

- 能启动 PIE
- 玩家能进入测试地图
- 角色能左右移动和跳跃
- Listen Server + 1 Client 能正常运行

## M2：GAS 基础

目标：建立 ASC、AttributeSet、基础属性同步。

任务：

- 创建 `UDOAbilitySystemComponent`（已完成）
- 创建 `UDOHealthSet` 和 `UDOPlaySet`（已完成）
- 玩家 ASC 放到 PlayerState（已完成）
- Character 初始化 AvatarActor（已完成）
- 添加 Health / Mana / AttackPower / DefensePower / Damage
- 绑定属性变化到 HUD
- 属性变化后可通过本地消息通知 HUD
- 配置基础 GameplayTags（已完成）
- 实现伤害 ExecutionCalculation

验收：

- Health / Mana 能在客户端 UI 更新
- 服务器修改属性后客户端同步
- 重生或重新 Possess 时 ASC 结构不崩
- PIE Listen Server + Client 通过

## M3：基础战斗

目标：实现一个可用的横版近战攻击闭环。

任务：

- 普通攻击三段 Ability
- 攻击动画占位
- 命中检测
- TargetData 发送
- 服务器验证命中
- 伤害 GameplayEffect
- Damage ExecutionCalculation
- 怪物受击和死亡
- 伤害数字
- 通过本地消息触发客户端伤害数字表现

验收：

- 客户端按攻击键能即时播放表现
- 服务器决定最终伤害
- 怪物死亡同步到客户端
- 普攻三段不会在网络下错乱

## M4：技能原型

目标：实现三个主动技能。

技能建议：

```text
DashStrike   突进斩
RisingSlash  浮空斩
DragonFlame  范围火焰斩
```

任务：

- 技能 Ability 基类
- Mana Cost
- Cooldown
- 技能栏 UI
- 冷却遮罩
- 技能输入 Tag
- 技能命中和伤害
- 基础 GameplayCue 表现

验收：

- 三个技能均可释放
- 技能消耗和冷却正确
- 客户端预测表现顺畅
- 服务器权威伤害正确

## M5：怪物和 Boss

目标：建立一个可完成的战斗关卡。

任务：

- 怪物 Character（基于 `ADOCharacter`）
- Enemy ASC
- Enemy AttributeSet
- StateTree/简单 C++ AI：索敌、靠近、攻击、返回
- 三种普通怪
- 一个 Boss
- Boss 血条
- Boss 简单阶段逻辑

验收：

- 怪物能攻击玩家
- 玩家能击败怪物
- Boss 能完成至少两种攻击
- Boss 死亡触发通关

## M6：副本流程

目标：形成完整关卡闭环。

任务：

- DungeonManager
- 刷怪点
- 分波刷怪
- 为后续 Mass + StateTree 预留刷怪数据结构
- 区域推进
- Boss 生成
- 通关结算
- 失败条件

验收：

```text
进入副本 -> 清第一波怪 -> 清第二波怪 -> Boss -> 结算
```

流程能完整跑通。

## M7：掉落和装备原型

目标：让战斗产生基础成长反馈。

任务：

- ItemDefinition
- EquipmentDefinition
- DropTable
- InventoryComponent
- 装备穿戴
- 攻击/防御属性变化
- 简单拾取

验收：

- 怪物死亡能掉落物品
- 玩家能拾取
- 背包能显示
- 装备能影响属性

## M8：第一版可玩闭环

目标：首个可玩版本。

必须完成：

- 一个职业
- 三个技能
- 一个副本
- 三种小怪
- 一个 Boss
- 基础掉落
- 基础装备
- 基础结算
- 联机架构验证

验收标准：

- 单人本地可玩
- Listen Server + Client 可玩
- 无明显阻塞崩溃
- 核心战斗手感可继续迭代
- 后续扩展不会推翻架构

## 暂缓系统

以下内容不进入第一版：

- 多职业
- 宠物
- 坐骑
- 公会
- 排行榜
- 商城
- 充值
- 完整公网匹配
- 大型剧情
- 复杂强化/洗练/套装
- 完整 Mass 群体 AI 框架

## 阶段工作方式

每个 Milestone 开工前：

1. 写清楚目标和验收。
2. 建一个小范围任务清单。
3. 保证 Listen Server + Client 测试。
4. 结束时提交代码和文档。

不要跨多个 Milestone 同时展开大系统。
