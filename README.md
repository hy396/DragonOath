# 🐉 DragonOath（龙契战纪）

> 基于 **Unreal Engine 5.8** 的原创 **2.5D 横版动作 RPG**（联机友好架构）。项目从第一天按联机设计，先搭建 GAS 驱动的动作战斗底座，再逐步扩展角色、技能、怪物、副本、掉落与成长系统。

![UE](https://img.shields.io/badge/Unreal_Engine-5.8-0D47A1?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C)
![GAS](https://img.shields.io/badge/Combat-GameplayAbilitySystem-00A86B)
![Network](https://img.shields.io/badge/Network-ServerAuthoritative-E4405F)
![Status](https://img.shields.io/badge/Status-Development-brightgreen)

> 📸 开发中截图 / 战斗演示放这里

---

## 📑 目录

- [🎮 游戏概览](#-游戏概览)
- [🕐 创建时间](#-创建时间)
- [✨ 项目亮点](#-项目亮点)
- [🛠 技术栈](#-技术栈)
- [🚀 当前进度](#-当前进度)
- [🏃 运行](#-运行)
- [📁 目录结构](#-目录结构)
- [📚 文档](#-文档)
- [🤝 开发说明](#-开发说明)

---

## 🎮 游戏概览

2.5D 横版动作 RPG（参考经典横版动作页游的节奏与系统结构，但公开发布坚持原创名称、角色、剧情与美术资产）。首个可玩版本追求完整闭环：

```text
启动游戏 → 测试关卡 → 操作角色 → 普攻/技能 → 击败怪物 → 击败 Boss → 掉落 → 结算
```

- ⚔️ **2.5D 横版动作手感**：普攻 + 主动技能 + HP/MP/攻击/防御/暴击属性
- 🌐 **联机优先**：服务端权威的伤害、死亡、任务结算
- 🧩 **模块化扩展**：角色 → 技能 → 怪物 → 副本 → 掉落 → 成长 逐步扩展

## 🕐 创建时间

2026-07 开始开发（持续迭代中）

## ✨ 项目亮点

1. **联机优先的战斗底座** — 网络策略第一天就定：GAS 驱动、服务端权威结算（伤害/死亡/任务），本地 PIE Listen Server + Client 可验证
2. **Lyra 风格模块化工程架构** — 集成 20+ 引擎级插件（CommonGame / CommonUser / GameSettings / GameplayMessageRouter / ModularGameplay / UIExtension），组件化、高复用
3. **完整 ItemSystem 管线** — Inventory / Equipment / Usage / QuickBar / Pickup 五件套设计，配套背包系统设计文档
4. **GAS 深度定制 + StateTree** — 自定义 AbilitySystemComponent / AttributeSet / GameplayTag 结构 + SharedCoolingAbility 冷却方案 + StateTree 状态机
5. **技术选型文档化** — 脚本方案对比（UnLua vs UnrealCSharp）产出"Lua 低风险编排层"结论；文档与代码同步维护，决策可追溯

## 🛠 技术栈

| 类别    | 技术                                                                                    |
| ------- | --------------------------------------------------------------------------------------- |
| 引擎    | UE 5.8（C++ + Blueprint，数据驱动用 DataAsset）                                         |
| 战斗    | Gameplay Ability System + SharedCoolingAbility                                          |
| 网络    | 服务端权威（PIE Listen Server + Client 验证）                                           |
| 架构    | Lyra 风格：CommonGame / CommonUser / GameSettings / GameplayMessageRouter / UIExtension |
| AI/状态 | StateTree / GameplayStateTree                                                           |
| UI      | CommonUI + UMG（LGUI 集成）                                                             |
| 脚本层  | UnLua（Lua 编排层，接入评估中）                                                         |

## 🚀 当前进度

| 模块                                                       | 状态                          |
| ---------------------------------------------------------- | ----------------------------- |
| GAS 基础（ASC / AttributeSet / GEContext / GameplayTag）   | ✅ 已实现                     |
| 玩家框架（Character / PlayerState / PlayerController C++） | ✅ 已实现                     |
| 插件基础（前端 / 设置 / 本地消息路由）                     | ✅ 已实现                     |
| ItemSystem（背包 / 装备 / 拾取）                           | ✅ 设计完成，部分实现         |
| 战斗闭环（普攻 / 技能 / 怪物 / Boss / 掉落 / 结算）        | 🚧 开发中（首个可玩版本目标） |
| 横版关卡 / 美术资产                                        | ⏳ 规划中                     |

## 🏃 运行

```bash
# 环境：UE 5.8 + VS2022
# 打开工程：
UE 5.8 启动器 → 打开 DragonOath.uproject（首次自动编译）

# 默认地图：/Game/BP/Maps/TestMap
# 联机验证：PIE → Advanced Settings → Number of Players = 2+，Net Mode = Play As Listen Server
```

## 📁 目录结构

```text
Source/DragonOath/        游戏 C++ 源码（AbilitySystem / Characters / ItemSystem / Player / UI...）
Content/                  游戏内容资产（BP / DragonOath / Spear / Developers）
Plugins/                  项目插件（CommonGame、GameSettings、ItemSystem 相关、UnLua 等）
Docs/                     项目设计、架构与学习文档
Config/                   引擎配置
Script/                   脚本/生成内容
Asset/                    临时视觉资产
```

## 📚 文档

- [Docs/README.md](Docs/README.md) — 文档总览
- [Docs/01_Development_Standards.md](Docs/01_Development_Standards.md) — 开发规范（C++/蓝图边界、数据驱动）
- [Docs/02_Technical_Architecture.md](Docs/02_Technical_Architecture.md) — 技术架构
- [Docs/03_Milestone_Roadmap.md](Docs/03_Milestone_Roadmap.md) — 里程碑路线图
- [Docs/05_Scripting_Plugin_Comparison.md](Docs/05_Scripting_Plugin_Comparison.md) — 脚本方案对比（UnLua 选型依据）
- [Docs/06_Combat_Attribute_Design.md](Docs/06_Combat_Attribute_Design.md) / [Docs/07_Inventory_System_Design.md](Docs/07_Inventory_System_Design.md) — 战斗属性 / 背包设计

## 🤝 开发说明

```bash
# 包含 Git submodule，首次克隆：
git clone --recurse-submodules https://github.com/hy396/DragonOath.git
```

- 不上传生成目录（`Binaries/` `Intermediate/` `Saved/` `DerivedDataCache/` 等），由构建流程重新生成
- 核心逻辑优先 C++，蓝图负责配置与表现；联机边界尽早确定，关键结算由服务端权威处理
- 开发原则：**先完成可玩的战斗闭环，再扩展系统数量；文档和代码一起**

# DragonOath

DragonOath（中文暂定名：龙契战纪）是一款基于 Unreal Engine 5.8 开发的原创 2.5D 横版动作 RPG。项目目标是先搭建一个联机友好的动作战斗底座，再逐步扩展角色、技能、怪物、副本、掉落和成长系统。

项目可以参考经典横版动作页游的节奏和系统结构，但公开发布时坚持原创名称、角色、剧情、UI、图标、音效和美术资产。

## 项目目标

首个可玩版本追求一个完整闭环：

```text
启动游戏 -> 进入测试关卡 -> 操作角色 -> 使用普攻和技能 -> 击败怪物 -> 击败 Boss -> 获得掉落 -> 结算
```

第一阶段重点：

- 2.5D 横版动作手感
- 普通攻击和主动技能
- HP / MP / 攻击 / 防御 / 暴击等基础属性
- 服务端权威的伤害、死亡和任务结算
- PIE Listen Server + Client 联机验证
- 基础技能栏、伤害数字、掉落和结算 UI

## 技术方向

- 引擎：Unreal Engine 5.8
- 语言：C++ + Blueprint
- 战斗框架：Gameplay Ability System
- UI 方向：Common UI / UMG
- 消息通信：GameplayMessageRouter
- 网络策略：从第一天按联机架构设计，优先保证本地 Listen Server + Client 可验证
- 插件依赖：项目内置 Setly、CommonGame、CommonUser、GameSettings、GameplayMessageRouter 等扩展模块

## 当前状态

当前项目已经包含：

- DragonOath 基础工程配置
- GAS 基础能力组件、属性集、GameplayEffectContext 和 GameplayTag 结构
- 玩家角色、PlayerState、PlayerController 的基础 C++ 框架
- 前端/UI、设置、消息路由等插件基础
- 项目开发规范、技术架构、里程碑和专题学习文档

更多文档见 [Docs/README.md](Docs/README.md)。

## 目录结构

```text
Config/                 UE 项目配置
Content/                游戏内容资产
Docs/                   项目设计、架构和学习文档
Plugins/                项目插件和复用模块
Script/                 C# / 脚本生成内容
Source/DragonOath/      游戏 C++ 源码
Asset/                  临时视觉资产和项目素材
```

## 克隆方式

项目包含 Git submodule，首次克隆建议使用：

```bash
git clone --recurse-submodules https://github.com/hy396/DragonOath.git
```

如果已经普通克隆，可以在仓库目录执行：

```bash
git submodule update --init --recursive
```

## Git 说明

本仓库不上传 UE 生成目录和编译产物，例如：

- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `Plugins/**/Binaries/`
- `Plugins/**/Intermediate/`

这些内容会由 Unreal Editor、Unreal Build Tool 或本地构建流程重新生成，不应进入 Git 历史。

## 开发原则

- 先完成可玩的战斗闭环，再扩展系统数量。
- 核心逻辑优先使用 C++，蓝图主要负责配置、表现和快速迭代。
- 联机边界尽早确定，伤害、死亡、任务结算等关键结果由服务端权威处理。
- 文档和代码一起维护，影响架构、目录和网络策略的改动需要同步更新文档。
