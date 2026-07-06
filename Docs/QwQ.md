# DragonOath 单模块扁平化开发规范（龙斗士复刻项目）

## 04. DragonOath 单模块扁平化开发规范

既然所有内容都在 DragonOath 这一个模块里实现，并且不拆分 Public/Private，核心采用基于功能的扁平化目录结构。
该结构对个人求职项目十分整洁，全部代码逻辑统一存放在 `Source/DragonOath/` 目录下，层级清晰一目了然。

## 1. 目录结构

所有代码统一存放于 `Source/DragonOath/`，按功能域划分子文件夹

```
Source/DragonOath/
├── DragonOath.Build.cs
├── DragonOath.cpp
├── DragonOath.h
├── Core/                    # 全局基础层
│   ├── DOTypes.h            # 全局结构体、枚举
│   ├── DOLogMacros.h        # 日志宏定义
│   ├── DOGameplayTags.h     # C++侧GameplayTag声明
│   ├── DOGameplayTags.cpp
│   └── DOInterfaces.h       # 通用接口声明
├── AbilitySystem/           # GAS战斗系统
│   ├── DOAbilitySystemComponent.h
│   ├── DOAbilitySystemComponent.cpp
│   ├── DOAttributeSet.h
│   ├── DOAttributeSet.cpp
│   ├── DOGameplayAbility.h  # 技能基类
│   ├── Abilities/           # 具体技能实现
│   │   ├── DOHeroAttackAbility.h
│   │   └── DOHeroSkillAbility.h
│   └── Effects/             # 伤害、增益GameplayEffect
│       └── DOMMC_Damage.h   # 伤害计算类
├── Characters/              # 角色实体
│   ├── ADOCharacter.h       # 角色/怪物基类
│   ├── ADOCharacter.cpp
│   ├── ADOHeroCharacter.h   # 玩家角色
│   ├── ADOHeroCharacter.cpp
│   ├── ADOEnemyCharacter.h  # 怪物角色
│   └── ADOEnemyCharacter.cpp
├── Components/              # 解耦功能组件
│   ├── DOCombatComponent.h      # 战斗组件（锁定、硬直）
│   ├── DOCombatComponent.cpp
│   ├── DOInventoryComponent.h   # 背包组件
│   ├── DOInteractionComponent.h # 交互组件
│   └── UDOTargetingComponent.h # 索敌组件
├── AI/                      # AI逻辑
│   ├── ADOAIController.h
│   ├── ADOAIController.cpp
│   └── DOAITasks.h          # 自定义StateTree Task
├── Data/                    # 数据驱动资产定义
│   ├── DOCharacterData.h    # 角色配置DataAsset
│   ├── DOItemData.h         # 物品配置DataAsset
│   └── UDOSkillData.h       # 技能配置（动画、GE引用）
├── Game/                    # 游戏流程框架
│   ├── ADOGameModeBase.h
│   ├── ADOGameModeBase.cpp
│   ├── ADOGameState.h
│   ├── ADOPlayerController.h
│   └── ADOPlayerState.h     # 存储ASC核心载体
└── UI/                      # 界面系统
    ├── UDOHUD.h
    ├── UDOHUD.cpp
    └── UDOCommonUserWidget.h # CommonUI扩展基类
```

## 2. DragonOath.Build.cs 模块配置

单模块统一管理全部依赖，面试可体现引擎模块依赖认知

```csharp
using UnrealBuildTool;

public class DragonOath : ModuleRules
{
    public DragonOath(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
        // 基础公共依赖
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "GameplayAbilities",      // GAS核心
            "GameplayTags",           // Tag标签系统
            "GameplayTasks",          // AbilityTask任务
            "CommonUI",               // UI通用框架
            "GameplayMessageRouter"   // 游戏消息总线
        });

        // 私有可选依赖（粒子、AI状态树）
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Niagara",
            "StateTree"
        });

        // 启用C++20标准
        CppStandard = CppStandardVersion.Cpp20;
    }
}
```

## 3. C++ 编码与头文件引用规范（单模块专用）

### 3.1 头文件引用原则

1. 同文件夹文件直接引用

```cpp
// ADOHeroCharacter.cpp 同目录头文件
#include "ADOHeroCharacter.h"
```

2. 跨功能域必须携带文件夹路径作为逻辑命名空间

```cpp
// 角色中引用GAS组件
#include "AbilitySystem/DOAbilitySystemComponent.h"
#include "AbilitySystem/DOGameplayAbility.h"
// 角色中引用战斗组件
#include "Components/DOCombatComponent.h"
```

### 3.2 统一类命名规范（统一前缀 DO，简历加分）

- Actor基类：`ADOCharacter`、玩家`ADOHeroCharacter`、怪物`ADOEnemyCharacter`
- UObject/组件/数据资产：`UDOCombatComponent`、`UDOGameplayAbility`、`UDOCharacterData`
- 全局枚举/结构体：`EDOInputID`、`FDOXXX`

## 4. 核心功能实现检查清单（龙斗士复刻手感核心）

1. `Core/DOTypes.h`
   - 定义枚举 `EDOInputID`，将增强输入Action与GAS技能InputID绑定
2. `AbilitySystem/DOGameplayAbility.h`
   - 提供接口函数 `GetAbilityInputID()`
   - 处理输入按下事件，自动激活技能
3. `Characters/ADOHeroCharacter.cpp`
   - `PossessedBy`：服务器初始化AbilitySystemComponent
   - `OnRep_PlayerState`：客户端同步初始化ASC
4. `Components/DOCombatComponent.cpp`
   - `TraceHit()`：球体/射线检测前方目标
   - `ApplyDamage()`：调用GameplayEffect完成伤害计算与施加

## 目录使用小贴士

1. 功能隔离：编写UI仅修改`UI`文件夹，伤害逻辑仅修改`AbilitySystem/Effects`，禁止跨目录乱堆代码；
2. 数据驱动亮点：`Data`文件夹为项目核心，面试可现场展示DataAsset配置，体现数据驱动开发思想；
3. 轻量化单模块结构，无多层Public/Private拆分，适合求职Demo快速迭代开发。
