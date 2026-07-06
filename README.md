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
