# 非 Content/ 改动的分内容 Git 提交与推送清单

> 生成依据：2026-08-05 工作区状态。当前分支为 master，HEAD 与 origin/master 均位于 711ef9b。
>
> 适用范围：本清单只处理源码、配置、工具和文档，不处理项目根目录 Content/ 下的 uasset、umap、纹理、材质和其他资产。
>
> 当前开发进度：GAS 冷却辅助函数已从插件工具中抽到项目源码；物品系统已统一归档到 Source/DragonOath/ItemSystem/；背包、装备、消耗品、快捷栏、拾取、存档和原生 Slate UI 已完成第一版 C++ 实现；蓝图待办和 Slate 外部资产方案已经补齐。
>
> 重要原则：本文件只提供提交命令，不会自动执行提交或推送。每组都必须先检查，再暂存，再提交，最后推送。不要使用 git add . 或 git add -A，避免把 Content/、本地构建日志和 UnLua 编译缓存带入提交。

---

## 0. 开始前：确认分支、远端和暂存区

从 PowerShell 执行：

```powershell
Set-Location "D:\ue_texiao\DragonOath"

git status --short
git branch --show-current
git remote -v
git log -1 --oneline
git diff --cached --name-status
```

预期：

- 当前分支是 master；
- 暂存区为空；
- 当前工作区的 Content/ 改动保持未暂存；
- 不要因为暂存区已有内容就直接执行 git reset，先逐项确认其来源。

每一组正式暂存前都执行一次以下检查，防止把 Content 资产混入：

```powershell
$stagedContent = git diff --cached --name-only | Where-Object { $_ -match "(^|/)Content/" }
if ($stagedContent) {
    Write-Error "暂存区包含 Content 资产，请先确认并撤回：$($stagedContent -join ', ')"
}
```

---

## 1. GAS 冷却与技能蓝图辅助函数

提交目的：将动态减少 GameplayEffect 剩余时间、查询技能冷却、查询技能实例和基础技能操作接口放入项目自己的 C++ 蓝图函数库，减少对 SharedCoolingAbility 插件工具函数的依赖。

包含路径：

- Source/DragonOath/AbilitySystem/BlueprintLibrary/DOAbilitySystemBlueprintLibrary.h
- Source/DragonOath/AbilitySystem/BlueprintLibrary/DOAbilitySystemBlueprintLibrary.cpp

本组内容：

- 按 GameplayEffect 类、ActiveGameplayEffectHandle 或效果标签修改剩余时间；
- 查询效果总时长、开始时间和当前冷却剩余时间；
- 根据 AbilitySpecHandle、技能类获取主技能实例；
- 提供项目自己的授予、激活和清除技能辅助节点；
- 服务器权威校验和参数有效性检查；
- 不修改 Plugins/SharedCoolingAbility，也不提交任何插件 Content 资产。

```powershell
$Group = @(
    "Source/DragonOath/AbilitySystem/BlueprintLibrary/DOAbilitySystemBlueprintLibrary.h",
    "Source/DragonOath/AbilitySystem/BlueprintLibrary/DOAbilitySystemBlueprintLibrary.cpp"
)

git diff -- $Group
git add -- $Group
git diff --cached --check
git diff --cached --stat
git diff --cached --name-status

git commit `
    -m "feat(gas): 补充技能冷却与 GAS 蓝图辅助函数" `
    -m "新增按 GameplayEffect 类、Handle 和标签修改剩余时间的蓝图节点" `
    -m "补充技能冷却时间查询、技能实例查询以及授予/激活/清除辅助接口" `
    -m "统一服务器权威校验，减少对 SharedCoolingAbility 工具函数的依赖"
git push origin master
```

提交后确认：蓝图函数库能够独立编译；暂存内容只有上述两个文件。

---

## 2. 物品系统、玩家组件与存档基础

提交目的：提交物品定义、背包、装备、消耗品、快捷栏、拾取、物品专属 GAS 桥接、玩家状态组件和第一版 SaveGame 基础。

包含路径：

- Config/DefaultGame.ini
- Source/DragonOath/DragonOath.Build.cs
- Source/DragonOath/AbilitySystem/Core/DOGameplayTag.h
- Source/DragonOath/AbilitySystem/Core/DOGameplayTag.cpp
- Source/DragonOath/ItemSystem/
- Source/DragonOath/SaveGame/
- Source/DragonOath/Player/DOPlayerState.h
- Source/DragonOath/Player/DOPlayerState.cpp

本组内容：

- ItemSystem/Core：DOItemDefinition、类型化属性和物品 Fragment；
- ItemSystem/Inventory：FastArray 背包、堆叠、移动、交换、拆分、整理、删除、消耗和 Owner-only 复制；
- ItemSystem/Equipment：装备槽、穿戴/卸下事务、属性 GE 和 ActiveGameplayEffectHandle；
- ItemSystem/Usage：即时回复、限时属性修改和复杂道具入口；
- ItemSystem/QuickBar：四格快捷栏组件和 ViewModel；
- ItemSystem/Pickup：世界掉落物与服务器拾取校验；
- ItemSystem/AbilitySystem：原生 GE、动态 FGameplayEffectSpec 和使用道具 Ability；
- ItemSystem/Tests：背包、装备、消耗品、快捷栏和存档自动化测试；
- SaveGame：背包、装备、快捷栏快照以及版本校验和恢复回滚；
- ADOPlayerState：持有背包、装备和快捷栏组件，并负责存档生命周期；
- DOGameplayTag：物品类型、分类、品质、装备槽、物品效果和 UI 消息标签；
- DefaultGame.ini：注册 ItemDefinition Primary Asset 扫描目录；
- DragonOath.Build.cs：补充物品系统所需模块依赖。

明确不包含：Content/DragonOath/Items/Definitions 下的测试 DataAsset、图标和其他 Content 资产。它们由第 4 组工具生成后，另行进行资产审查和提交。

```powershell
$Group = @(
    "Config/DefaultGame.ini",
    "Source/DragonOath/DragonOath.Build.cs",
    "Source/DragonOath/AbilitySystem/Core/DOGameplayTag.h",
    "Source/DragonOath/AbilitySystem/Core/DOGameplayTag.cpp",
    "Source/DragonOath/ItemSystem",
    "Source/DragonOath/SaveGame",
    "Source/DragonOath/Player/DOPlayerState.h",
    "Source/DragonOath/Player/DOPlayerState.cpp"
)

git diff -- $Group
git add -- $Group
git diff --cached --check
git diff --cached --stat
git diff --cached --name-status

git commit `
    -m "feat(item): 完成物品系统、装备事务与玩家存档基础" `
    -m "新增 ItemDefinition、FastArray 背包、装备槽、消耗品、快捷栏和世界拾取流程" `
    -m "使用 C++ 原生 GameplayEffect 与动态 Spec 处理装备属性和简单道具效果" `
    -m "将背包、装备和快捷栏挂载到 PlayerState，并加入 SaveGame 快照、校验与回滚" `
    -m "补充 ItemDefinition AssetManager 扫描配置、GameplayTag 以及物品系统自动化测试"
git push origin master
```

提交前验证建议：确认 ItemDefinition 扫描路径与实际 Content 路径一致；确认测试源码没有引用临时绝对路径；确认装备属性通过 PlayerState ASC 生效，而不是由 UI 或角色外观直接累加。

---

## 3. 原生 Slate 背包 UI 与玩家控制器接入

提交目的：提交纯 Slate 背包、装备、快捷栏和角色预览 UI，以及 PlayerController 的页面打开、快捷栏输入、拾取请求和资源生命周期接入。

包含路径：

- Source/DragonOath/UI/Inventory/
- Source/DragonOath/Player/DOPlayerCharacter.h
- Source/DragonOath/Player/DOPlayerCharacter.cpp
- Source/DragonOath/Player/DOPlayerController.h
- Source/DragonOath/Player/DOPlayerController.cpp

本组内容：

- UDOInventoryScreen：CommonUI 页面壳；
- DOInventorySlateWidgets：背包格子、装备槽、Tooltip、分页、拖拽和操作栏；
- DOInventoryViewModel：背包和装备只读状态到 Slate 的适配；
- DOInventoryStyle：字体、颜色、边框、品质和选中样式；
- DOItemQuickBarSlateWidgets：本地玩家底部快捷栏；
- DOInventoryPreviewComponent：角色预览基础组件；
- DOCombatRatingConfig：装备属性摘要和战力显示配置；
- ADOPlayerController：I/B 背包快捷键、CommonUI Menu Layer、快捷栏 HUD 和服务器拾取请求；
- ADOPlayerCharacter：玩家角色上的背包预览组件接入。

UI 约束：背包主体不使用 UMG Designer，不在 Blueprint 中直接扣数量、装备、施加 GE 或处理服务器结果。装备图标只用于 UI，不驱动 Mesh、武器 Actor 或服饰外观。

```powershell
$Group = @(
    "Source/DragonOath/UI/Inventory",
    "Source/DragonOath/Player/DOPlayerCharacter.h",
    "Source/DragonOath/Player/DOPlayerCharacter.cpp",
    "Source/DragonOath/Player/DOPlayerController.h",
    "Source/DragonOath/Player/DOPlayerController.cpp"
)

git diff -- $Group
git add -- $Group
git diff --cached --check
git diff --cached --stat
git diff --cached --name-status

git commit `
    -m "feat(ui): 接入原生 Slate 背包、装备与快捷栏界面" `
    -m "新增 CommonUI 页面壳和 C++ Slate 背包主体，支持格子、装备槽、Tooltip、分页与拖拽" `
    -m "新增 ViewModel、统一 Style、异步资源显示、快捷栏 HUD 和角色预览组件" `
    -m "接入 PlayerController 的背包快捷键、Menu Layer、快捷栏输入和服务器拾取请求" `
    -m "保持装备属性、库存事务和未来服饰外观系统相互解耦"
git push origin master
```

提交前验证建议：至少完成一次编辑器编译；确认 UDOInventoryScreen 使用 UI.Layer.Menu；确认快捷栏只在本地玩家创建；确认关闭页面时不会累积失活 CommonUI 页面实例。

---

## 4. ItemDefinition 测试资产生成工具

提交目的：独立提交 Unreal Editor Python 工具，用于批量创建第一版测试 DOItemDefinition。脚本本身属于非 Content 文件，但脚本执行产生的 uasset 不在本清单内。

包含路径：

- Tools/CreateInventoryTestAssets.py

工具职责：

- 在 /Game/DragonOath/Items/Definitions 创建或更新测试 ItemDefinition；
- 配置物品名称、描述、图标、类型、品质、堆叠和售价；
- 创建背包、装备和消耗品 Fragment；
- 为第一版测试资产填入占位图标和类型化效果数据；
- 只用于编辑器一次性生成，不属于运行时 Lua 或游戏逻辑。

```powershell
$Group = @(
    "Tools/CreateInventoryTestAssets.py"
)

Get-Content -LiteralPath "Tools/CreateInventoryTestAssets.py" -Encoding UTF8 -TotalCount 40
git add -- $Group
git diff --cached --check
git diff --cached --stat
git diff --cached --name-status

git commit `
    -m "tools(item): 增加 ItemDefinition 测试资产生成脚本" `
    -m "新增 Unreal Editor Python 工具，批量创建和更新第一版测试物品定义" `
    -m "自动配置物品基础字段、类型品质、背包 Fragment、装备 Fragment 和消耗品效果" `
    -m "脚本只负责生成 Content 测试资产，不参与运行时逻辑，也不替代资产审查"
git push origin master
```

执行脚本前要确认 C++ 模块已编译、编辑器 Python 可用、目标目录已经注册为 ItemDefinition Primary Asset。脚本生成的 Content 资产必须另行检查，不要因为脚本提交而自动提交整个 Content/。

---

## 5. 物品系统、Slate UI 与项目规范文档

提交目的：同步当前已经落地的源码目录、背包装备架构、纯 Slate UI 方案、蓝图待办和 Agent 开发约束。

包含路径：

- AGENTS.md
- Docs/01_Development_Standards.md
- Docs/02_Technical_Architecture.md
- Docs/README.md
- Docs/07_Inventory_System_Design.md
- Docs/蓝图需要做的事情/背包改造_蓝图待办.md
- Docs/方案/纯Slate背包UI与外部资产自动化落地方案.md
- Docs/Git提交中转区/非Content分内容提交与推送清单.md

本组内容：

- 物品源码目录重组后的开发规范和项目结构；
- PlayerState 持有背包、装备和快捷栏的架构说明；
- 原生 GE、动态 Spec、装备属性和消耗品使用流程；
- CommonUI 页面壳与 Slate 主体的职责边界；
- Blueprint 资产配置步骤和暂未完成的编辑器验证项；
- 外部图片、材质、字体、音效和 RenderTarget 的 Slate 使用方案；
- 当前非 Content 文件的安全分组、提交和推送规则。

```powershell
$Group = @(
    "AGENTS.md",
    "Docs/01_Development_Standards.md",
    "Docs/02_Technical_Architecture.md",
    "Docs/README.md",
    "Docs/07_Inventory_System_Design.md",
    "Docs/蓝图需要做的事情/背包改造_蓝图待办.md",
    "Docs/方案/纯Slate背包UI与外部资产自动化落地方案.md",
    "Docs/Git提交中转区/非Content分内容提交与推送清单.md"
)

git diff -- $Group
git add -- $Group
git diff --cached --check
git diff --cached --stat
git diff --cached --name-status

git commit `
    -m "docs(item): 同步物品系统与 Slate UI 设计文档" `
    -m "同步 ItemSystem 目录重组、PlayerState 组件关系和背包装备联机架构" `
    -m "补充原生 GE、动态 Spec、存档、消耗品和装备属性的配置说明" `
    -m "更新纯 Slate 页面结构、外部资产流程、Blueprint 待办和职责边界" `
    -m "记录非 Content 文件的分组提交、资产排除和推送检查规则"
git push origin master
```

---

## 6. 2.5D 摄像机与纵深移动方案文档

提交目的：将当前 2.5D 摄像机和纵深移动设计单独提交，避免与背包代码和 UI 资产配置混在一起。

包含路径：

- Docs/方案/2.5D摄像机方案.md
- Docs/方案/2.5D纵深移动方案.md

```powershell
$Group = @(
    "Docs/方案/2.5D摄像机方案.md",
    "Docs/方案/2.5D纵深移动方案.md"
)

git diff -- $Group
git add -- $Group
git diff --cached --check
git diff --cached --stat
git diff --cached --name-status

git commit `
    -m "docs(camera): 增加 2.5D 摄像机与纵深移动方案" `
    -m "说明 2.5D 摄像机跟随、边界、缩放和镜头状态管理方案" `
    -m "说明角色纵深移动、碰撞约束、输入映射和后续联机扩展方向" `
    -m "将摄像机设计与物品系统、UI 资产和运行时代码分开记录"
git push origin master
```

---

## 7. 当前明确排除的文件

### 7.1 根 Content/ 资产

本轮工作区中以下内容暂不纳入非 Content 提交：

- Content/BP/Characters/Player/BP_BasePlayerCharacter.uasset
- Content/BP/Characters/Player/Inputs/DA_InputConfig.uasset
- Content/BP/Characters/Player/Inputs/IMC_Default.uasset
- Content/BP/GAS/Ability/GA_Dash.uasset
- Content/BP/Maps/TestMap.umap
- Content/BP/Characters/Player/Inputs/IA/IA_Dash.uasset
- Content/BP/Player/
- Content/Assets/
- Content/DragonOath/
- Content/Spear/

这些资产涉及闪避输入、玩家蓝图、测试地图、物品测试 DataAsset 和未来 UI 资源，应在编辑器配置、Data Validation、PIE Listen Server + 1 Client 验证完成后，按资产主题另行提交。

### 7.2 本地生成物

以下文件不应提交：

- build_log.txt
- build_log2.txt
- Plugins/UnLua/Source/UnLuaDefaultParamCollectorUbtPlugin/obj/
- Binaries/
- Intermediate/
- Saved/
- DerivedDataCache/

检查忽略状态：

```powershell
git status --ignored --short -- build_log.txt build_log2.txt Plugins/UnLua/Source/UnLuaDefaultParamCollectorUbtPlugin/obj
```

### 7.3 没有实际内容差异的 Target 文件

工作区可能显示 Source/DragonOathEditor.Target.cs 为修改，但当前 git diff 没有实际内容差异，通常是换行或文件时间状态造成的。不要仅凭 git status 就把它加入某个提交：

```powershell
git diff --quiet -- Source/DragonOathEditor.Target.cs
if ($LASTEXITCODE -eq 0) {
    Write-Host "Source/DragonOathEditor.Target.cs 没有实际内容差异，不暂存。"
} else {
    git diff -- Source/DragonOathEditor.Target.cs
}
```

---

## 8. 每组提交后的统一确认

每次 git push 后执行：

```powershell
git status --short
git log --oneline -8
git log origin/master..HEAD --oneline
git ls-remote --heads origin master
```

预期：

1. 当前组的源码、文档或工具已经提交并推送；
2. git log origin/master..HEAD --oneline 无输出；
3. 未提交的内容主要是根 Content/ 资产或明确列出的本地生成物；
4. 暂存区不残留其他主题文件。

如果推送被远端更新拒绝，先审阅远端提交，再执行：

```powershell
git pull --rebase origin master
git push origin master
```

不要对 master 使用 git push --force 或 git push --force-with-lease。
