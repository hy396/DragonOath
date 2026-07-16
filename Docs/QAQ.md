# Aura → GitHub 推送方案（V5 — ~85 commits，每个 commit 独立 push，按模块代码+资产交错）

仓库：`D:\ue_texiao\Aura`
远端：`git@github.com:hy396/Aura.git`（**Public**，空仓库）
SSH 密钥：`C:\Users\幻雨\.ssh\id_ed25519`（passphrase 已缓存到 ssh-agent）

策略：
- **HEAD 已删**（`git update-ref -d HEAD`），本地历史从 0 开始
- 每个 commit **单独 push**，从不积压
- **代码 commit 与对应资产 commit 交错**：每个 C++ 模块 commit 之后紧跟它的 Content 资产 commit，避免中间态引用断裂
- 总 commit 数 ~85，落在 50-96 区间（贴合 Crunch 风格）

---

## 一、覆盖范围（按模块交错 ~85 commits）

### Phase A：项目脚手架（5 commits，纯工程层）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| A1 | chore | `chore(.gitignore): 增加嵌套 .gitignore 忽略规则` | `.gitignore` |
| A2 | chore | `chore(build): 配置 Aura.Build.cs 模块依赖` | `Source/Aura/Aura.Build.cs` |
| A3 | chore | `chore(config): 更新 Config/*.ini 项目配置` | `Config/*.ini` |
| A4 | chore | `chore(project): 更新 Aura.uproject 与 .sln 工程配置` | `Aura.uproject` + `Aura.sln.DotSettings.user` + `Source/Aura.Target.cs` + `Source/AuraEditor.Target.cs` |
| A5 | chore | `chore(cleanup): 清理已删除的 .idea IDE 工作区文件` | `.idea/` 下标记为 deleted 的文件 |

### Phase B：GAS 类型与核心组件（4 commits 代码 + 0 资产）

| #  | scope         | message | 路径 |
| -- | ------------- | ------- | ---- |
| B1 | types         | `feat(types): AuraAbilityTypes 共享游戏效果上下文结构` | `Source/Aura/Private/AuraAbilityTypes.cpp` + `Public/AuraAbilityTypes.h` |
| B2 | abilitysystem | `feat(abilitysystem): AuraAbilitySystemComponent ASC 主组件` | `Source/Aura/Private/AbilitySystem/AuraAbilitySystemComponent.cpp` + `.h` |
| B3 | attributeset  | `feat(attributeset): AuraAttributeSet 属性集` | `Source/Aura/Private/AbilitySystem/AuraAttributeSet.cpp` + `.h` |
| B4 | abilitysystem | `feat(abilitysystem): Globals + Library 项目级 GAS 工具` | `Source/Aura/Private/AbilitySystem/AuraAbilitySystemGlobals.cpp` + `.h` + `AuraAbilitySystemLibrary.cpp` + `.h` + `Source/Aura/AuraLogChannels.cpp` + `Source/Aura/AuraLogChannels.h` |

### Phase C：法术系统（14 commits 代码 + 2 commits 资产）

| #   | scope   | message | 路径 |
| --- | ------- | ------- | ---- |
| C1  | ability | `feat(ability): AuraGameplayAbility 主动技能基类` | `Source/Aura/Private/AbilitySystem/Abilities/AuraGameplayAbility.cpp` + `Public/AbilitySystem/Abilities/AuraGameplayAbility.h` |
| C2  | ability | `feat(ability): AuraDamageGameplayAbility 伤害技能基类` | `Source/Aura/Private/AbilitySystem/Abilities/AuraDamageGameplayAbility.cpp` + `Public/AbilitySystem/Abilities/AuraDamageGameplayAbility.h` |
| C3  | spell   | `feat(spell): AuraFireBolt 火球投射法术` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/AuraFireBolt.cpp` + `Public/AbilitySystem/Abilities/Spells/AuraFireBolt.h` |
| C4  | spell   | `feat(spell): AuraFireBlast 火球爆 AOE` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/AuraFireBlast.cpp` + `Public/AbilitySystem/Abilities/Spells/AuraFireBlast.h` |
| C5  | spell   | `feat(spell): AuraBeamSpell 火柱连续伤害` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/AuraBeamSpell.cpp` + `Public/AbilitySystem/Abilities/Spells/AuraBeamSpell.h` |
| C6  | spell   | `feat(spell): Electrocute 闪电链` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/Electrocute.cpp` + `Public/AbilitySystem/Abilities/Spells/Electrocute.h` |
| C7  | spell   | `feat(spell): ArcaneShards 奥术碎片` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/ArcaneShards.cpp` + `Public/AbilitySystem/Abilities/Spells/ArcaneShards.h` |
| C8  | spell   | `feat(spell): ProjectileSpell 投射法术基类` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/ProjectileSpell.cpp` + `Public/AbilitySystem/Abilities/Spells/ProjectileSpell.h` |
| C9  | spell   | `feat(spell): ProjectileBarrage 连发投射` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/ProjectileBarrage.cpp` + `Public/AbilitySystem/Abilities/Spells/ProjectileBarrage.h` |
| C10 | spell   | `feat(spell): AuraSummonAbility 召唤生物` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/AuraSummonAbility.cpp` + `Public/AbilitySystem/Abilities/Spells/AuraSummonAbility.h` |
| C11 | spell   | `feat(spell): AuraMeleeAttack 近战攻击` | `Source/Aura/Private/AbilitySystem/Abilities/Spells/AuraMeleeAttack.cpp` + `Public/AbilitySystem/Abilities/Spells/AuraMeleeAttack.h` |
| C12 | passive | `feat(passive): AuraPassiveAbility 被动技能` | `Source/Aura/Private/AbilitySystem/Abilities/Passive/AuraPassiveAbility.cpp` + `Public/AbilitySystem/Abilities/Passive/AuraPassiveAbility.h` |
| C13 | tasks   | `feat(abilitytasks): TargetDataUnderMouse 自定义 AbilityTask` | `Source/Aura/Private/AbilitySystem/AbilityTasks/TargetDataUnderMouse.cpp` + `Public/AbilitySystem/AbilityTasks/TargetDataUnderMouse.h` |
| C14 | tasks   | `feat(asynctasks): WaitCooldownChange 异步任务` | `Source/Aura/Private/AbilitySystem/AsyncTasks/WaitCooldownChange.cpp` + `Public/AbilitySystem/AsyncTasks/WaitCooldownChange.h` |
| C15 | content | `feat(content): Spells 法术数据资产 + Fire/Combat 通用特效` | `Content/Assets/Spells/` + `Content/Assets/Effects/Fire/` + `Content/Assets/Effects/Combat/` (~17 MB) |
| C16 | content | `feat(content): Abilities 音效库 + BlinkAndDashVFX 位移技能特效` | `Content/Assets/Sounds/Abilities/` + `Content/Assets/BlinkAndDashVFX/` (~41 MB) |

### Phase D：数据资产（5 commits 代码 + 0 资产，资产在 Phase H 角色资产里一并处理）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| D1 | data  | `feat(data): AbilityInfo 技能数据资产` | `Source/Aura/Private/AbilitySystem/Data/AbilityInfo.cpp` + `Public/AbilitySystem/Data/AbilityInfo.h` |
| D2 | data  | `feat(data): AttributeInfo 属性 UI 数据` | `Source/Aura/Private/AbilitySystem/Data/AttributeInfo.cpp` + `Public/AbilitySystem/Data/AttributeInfo.h` |
| D3 | data  | `feat(data): CharacterClassInfo 职业数据` | `Source/Aura/Private/AbilitySystem/Data/CharacterClassInfo.cpp` + `Public/AbilitySystem/Data/CharacterClassInfo.h` |
| D4 | data  | `feat(data): LevelUpInfo 升级数据` | `Source/Aura/Private/AbilitySystem/Data/LevelUpInfo.cpp` + `Public/AbilitySystem/Data/LevelUpInfo.h` |
| D5 | data  | `feat(data): LootTiers 掉落表` | `Source/Aura/Private/AbilitySystem/Data/LootTiers.cpp` + `Public/AbilitySystem/Data/LootTiers.h` |

### Phase E：伤害公式与属性修饰（3 commits 代码 + 0 资产）

| #  | scope      | message | 路径 |
| -- | ---------- | ------- | ---- |
| E1 | execcalc   | `feat(execcalc): ExecCalc_Damage 自定义伤害执行计算` | `Source/Aura/Private/AbilitySystem/ExecCalc/ExecCalc_Damage.cpp` + `Public/AbilitySystem/ExecCalc/ExecCalc_Damage.h` |
| E2 | modmagcalc | `feat(modmagcalc): MMC_MaxHealth 最大生命属性修饰` | `Source/Aura/Private/AbilitySystem/ModMagCalc/MMC_MaxHealth.cpp` + `Public/AbilitySystem/ModMagCalc/MMC_MaxHealth.h` |
| E3 | modmagcalc | `feat(modmagcalc): MMC_MaxMana 最大法力属性修饰` | `Source/Aura/Private/AbilitySystem/ModMagCalc/MMC_MaxMana.cpp` + `Public/AbilitySystem/ModMagCalc/MMC_MaxMana.h` |

### Phase F：Actor 体系（7 commits 代码 + 1 commit 资产）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| F1 | actor | `feat(actor): AuraProjectile 投射物基类` | `Source/Aura/Private/Actor/AuraProjectile.cpp` + `Public/Actor/AuraProjectile.h` |
| F2 | actor | `feat(actor): AuraFireBall 火球 Actor` | `Source/Aura/Private/Actor/AuraFireBall.cpp` + `Public/Actor/AuraFireBall.h` |
| F3 | actor | `feat(actor): MagicCircle 法阵 Actor` | `Source/Aura/Private/Actor/MagicCircle.cpp` + `Public/Actor/MagicCircle.h` |
| F4 | actor | `feat(actor): AuraEffectActor 通用效果 Actor` | `Source/Aura/Private/Actor/AuraEffectActor.cpp` + `Public/Actor/AuraEffectActor.h` |
| F5 | actor | `feat(actor): PointCollection 点集合 Actor` | `Source/Aura/Private/Actor/PointCollection.cpp` + `Public/Actor/PointCollection.h` |
| F6 | actor | `feat(actor): EnemySpawnPoint 敌人生成点` | `Source/Aura/Private/Actor/EnemySpawnPoint.cpp` + `Public/Actor/EnemySpawnPoint.h` |
| F7 | actor | `feat(actor): EnemySpawnVolume 敌人生成范围` | `Source/Aura/Private/Actor/EnemySpawnVolume.cpp` + `Public/Actor/EnemySpawnVolume.h` |
| F8 | content | `feat(content): MagicCircle 法阵资产 + Pickups 可拾取道具` | `Content/Assets/MagicCircles/` + `Content/Assets/Pickups/` (~54 MB) |

### Phase G：AI 体系（3 commits 代码 + 1 commit 资产）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| G1 | ai    | `feat(ai): AuraAIController 敌人 AI 控制器` | `Source/Aura/Private/AI/AuraAIController.cpp` + `Public/AI/AuraAIController.h` |
| G2 | ai    | `feat(ai): BTService_FindNearestPlayer BT 服务` | `Source/Aura/Private/AI/BTService_FindNearestPlayer.cpp` + `Public/AI/BTService_FindNearestPlayer.h` |
| G3 | ai    | `feat(ai): BTTask_Attack BT 攻击任务` | `Source/Aura/Private/AI/BTTask_Attack.cpp` + `Public/AI/BTTask_Attack.h` |
| G4 | content | `feat(content): Enemies 敌人资产（模型 + 动画 + 材质）` | `Content/Assets/Enemies/` (~207 MB) |

### Phase H：角色体系（3 commits 代码 + 5 commits 资产，按角色拆分）

| #   | scope     | message | 路径 | 大小 |
| --- | --------- | ------- | ---- | ---- |
| H1  | character | `feat(character): AuraCharacterBase 角色基类` | `Source/Aura/Private/Character/AuraCharacterBase.cpp` + `Public/Character/AuraCharacterBase.h` | < 1 MB |
| H2  | character | `feat(character): AuraCharacter 玩家角色` | `Source/Aura/Private/Character/AuraCharacter.cpp` + `Public/Character/AuraCharacter.h` | < 1 MB |
| H3  | character | `feat(character): AuraEnemy 敌人角色` | `Source/Aura/Private/Character/AuraEnemy.cpp` + `Public/Character/AuraEnemy.h` | < 1 MB |
| H4  | content   | `feat(content): Rokoko 主角资产（骨骼 + 动画 + 材质 + 武器）` | `Content/Assets/Characters/Rokoko/`（Anim/Materials/Skeleta/Mesh/wuqi 等） | ~280 MB |
| H5  | content   | `feat(content): Momo + Aura 角色资产（IK/RTG 绑定）` | `Content/Assets/Characters/Momo/` + `Content/Assets/Characters/Aura/` | ~30 MB |
| H6  | content   | `feat(content): Kerota 角色资产（VRM 模型 + 材质）` | `Content/Assets/Characters/Kerota/`（中文文件名 uasset） | ~70 MB |
| H7  | content   | `feat(content): Feibi + FeiBi2 角色资产` | `Content/Assets/Characters/Feibi/` + `Content/Assets/Characters/FeiBi2/` | ~35 MB |
| H8  | content   | `feat(content): 角色通用 Materials 材质库` | `Content/Assets/Characters/Materials/` | ~10 MB |

### Phase I：存档检查点（2 commits 代码 + 1 commit 资产）

| #  | scope      | message | 路径 |
| -- | ---------- | ------- | ---- |
| I1 | checkpoint | `feat(checkpoint): Checkpoint 检查点 Actor` | `Source/Aura/Private/Checkpoint/Checkpoint.cpp` + `Public/Checkpoint/Checkpoint.h` |
| I2 | checkpoint | `feat(checkpoint): MapEntrance + LoadScreenSaveGame 地图入口与存档` | `Source/Aura/Private/Checkpoint/MapEntrance.cpp` + `Source/Aura/Private/Checkpoint/LoadScreenSaveGame.cpp` + `Public/Checkpoint/MapEntrance.h` + `Public/Checkpoint/LoadScreenSaveGame.h` |
| I3 | content    | `feat(content): Checkpoint 检查点资产 + Beacon 灯塔材质` | `Content/Assets/Dungeon/Checkpoint/` + `Content/Assets/Dungeon/Beacon/` |

### Phase J：Game 核心（3 commits 代码 + 1 commit 资产）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| J1 | game  | `feat(game): AuraGameInstance 游戏实例` | `Source/Aura/Private/Game/AuraGameInstance.cpp` + `Public/Game/AuraGameInstance.h` |
| J2 | game  | `feat(game): AuraGameModeBase 游戏模式基类` | `Source/Aura/Private/Game/AuraGameModeBase.cpp` + `Public/Game/AuraGameModeBase.h` |
| J3 | game  | `feat(game): MyAssetManager + MyGameplayTags 单例资产与标签管理` | `Source/Aura/Private/MyAssetManager.cpp` + `Source/Aura/Private/MyGameplayTags.cpp` + `Public/MyAssetManager.h` + `Public/MyGameplayTags.h` |
| J4 | content | `feat(content): Maps 地图资产（Dungeon 主城 + MainMenu + LoadMenu + EQS_TestingMap）` | `Content/Maps/`（Dungeon.umap / MainMenu.umap / LoadMenu.umap / EQS_TestingMap.umap） |

### Phase K：输入（1 commit）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| K1 | input | `feat(input): AuraInputComponent + InputConfig 输入组件与配置` | `Source/Aura/Private/Input/AuraInputComponent.cpp` + `Source/Aura/Private/Input/InputConfig.cpp` + `Public/Input/AuraInputComponent.h` + `Public/Input/InputConfig.h` |

### Phase L：交互接口（5 commits）

| #  | scope     | message | 路径 |
| -- | --------- | ------- | ---- |
| L1 | interface | `feat(interface): CombatInterface 战斗接口` | `Source/Aura/Private/Interaction/CombatInterface.cpp` + `Public/Interaction/CombatInterface.h` |
| L2 | interface | `feat(interface): EnemyInterface 敌人接口` | `Source/Aura/Private/Interaction/EnemyInterface.cpp` + `Public/Interaction/EnemyInterface.h` |
| L3 | interface | `feat(interface): HighlightInterface 高亮接口` | `Source/Aura/Private/Interaction/HighlightInterface.cpp` + `Public/Interaction/HighlightInterface.h` |
| L4 | interface | `feat(interface): PlayerInterface 玩家接口` | `Source/Aura/Private/Interaction/PlayerInterface.cpp` + `Public/Interaction/PlayerInterface.h` |
| L5 | interface | `feat(interface): SaveInterface 存档接口` | `Source/Aura/Private/Interaction/SaveInterface.cpp` + `Public/Interaction/SaveInterface.h` |

### Phase M：Player 控制（2 commits）

| #  | scope  | message | 路径 |
| -- | ------ | ------- | ---- |
| M1 | player | `feat(player): AuraPlayerController 玩家控制器` | `Source/Aura/Private/Player/AuraPlayerController.cpp` + `Public/Player/AuraPlayerController.h` |
| M2 | player | `feat(player): AuraPlayerState 玩家状态` | `Source/Aura/Private/Player/AuraPlayerState.cpp` + `Public/Player/AuraPlayerState.h` |

### Phase N：UI 体系（8 commits 代码 + 1 commit 资产）

| #  | scope | message | 路径 |
| -- | ----- | ------- | ---- |
| N1 | ui    | `feat(ui): AuraHUD HUD 基类` | `Source/Aura/Private/UI/HUD/AuraHUD.cpp` + `Public/UI/HUD/AuraHUD.h` |
| N2 | ui    | `feat(ui): AuraUserWidget 用户 Widget 基类` | `Source/Aura/Private/UI/Widget/AuraUserWidget.cpp` + `Public/UI/Widget/AuraUserWidget.h` |
| N3 | ui    | `feat(ui): DamageTextComponent 伤害数字组件` | `Source/Aura/Private/UI/Widget/DamageTextComponent.cpp` + `Public/UI/Widget/DamageTextComponent.h` |
| N4 | ui    | `feat(ui): AuraWidgetController Widget 控制器基类` | `Source/Aura/Private/UI/WidgetController/AuraWidgetController.cpp` + `Public/UI/WidgetController/AuraWidgetController.h` |
| N5 | ui    | `feat(ui): OverlayWidgetController 覆盖层控制器` | `Source/Aura/Private/UI/WidgetController/OverlayWidgetController.cpp` + `Public/UI/WidgetController/OverlayWidgetController.h` |
| N6 | ui    | `feat(ui): AttributeMenuWidgetController 属性菜单控制器` | `Source/Aura/Private/UI/WidgetController/AttributeMenuWidgetController.cpp` + `Public/UI/WidgetController/AttributeMenuWidgetController.h` |
| N7 | ui    | `feat(ui): SpellMenuWidgetController 技能菜单控制器` | `Source/Aura/Private/UI/WidgetController/SpellMenuWidgetController.cpp` + `Public/UI/WidgetController/SpellMenuWidgetController.h` |
| N8 | ui    | `feat(ui): LoadScreen HUD + MVVM ViewModels + LoadScreen Widget` | `Source/Aura/Private/UI/HUD/LoadScreenHUD.cpp` + `Source/Aura/Private/UI/ViewModel/*.cpp` + `Source/Aura/Private/UI/Widget/LoadScreenWidget.cpp` + 对应 `Public/UI/.../*.h` |
| N9 | content | `feat(content): UI 界面资源（Borders + Globes + HealthMana + Rings + My_Ui + V1 + V2）` | `Content/Assets/UI/`（除 Spells/ 外全部子目录） | ~70 MB |

### Phase O：特效组件（2 commits 代码 + 1 commit 资产）

| #  | scope     | message | 路径 |
| -- | --------- | ------- | ---- |
| O1 | component | `feat(component): DeBuffNiagaraComponent Debuff 特效组件` | `Source/Aura/Private/AbilitySystem/DeBuff/DeBuffNiagaraComponent.cpp` + `Public/AbilitySystem/DeBuff/DeBuffNiagaraComponent.h` |
| O2 | component | `feat(component): PassiveNiagaraComponent 被动技能特效组件` | `Source/Aura/Private/AbilitySystem/Passive/PassiveNiagaraComponent.cpp` + `Public/AbilitySystem/Passive/PassiveNiagaraComponent.h` |
| O3 | content   | `feat(content): _Niagara 特效框架 + Shock/Cursor 通用特效` | `Content/Assets/_Niagara/` + `Content/Assets/Effects/Shock/` + `Content/Assets/Effects/Cursor/` | ~750 MB |

### Phase P：Plugins 集成（3 commits，纯插件层）

| #  | scope  | message | 路径 |
| -- | ------ | ------- | ---- |
| P1 | plugin | `feat(plugin): 集成 RiderLink JetBrains Rider UE 集成插件` | `Plugins/Developer/RiderLink/` |
| P2 | plugin | `feat(plugin): 集成 KawaiiPhysics 物理模拟插件` | `Plugins/KawaiiPhysics/` |
| P3 | plugin | `feat(plugin): 集成 VRM4U VRM 模型导入插件` | `Plugins/VRM4U/`（注意：排除 `Plugins/VRM4U/ThirdParty/assimp/` `rapidjson/` 这两个第三方库子目录，已加 ignore） |

### Phase Q：独立小资产（5 commits）

| #  | scope   | message | 路径 | 大小 |
| -- | ------- | ------- | ---- | ---- |
| Q1 | content | `feat(content): Materials 基础材质库` | `Content/Assets/Materials/` | ~96 KB |
| Q2 | content | `feat(content): _Text 本地化文本` | `Content/Assets/_Text/` | ~15 MB |
| Q3 | content | `feat(content): Fonts 字体资源` | `Content/Assets/Fonts/` | ~6 MB |
| Q4 | content | `feat(content): UI Spells 子目录（技能图标）` | `Content/Assets/UI/Spells/` | ~10 MB |
| Q5 | content | `feat(content): 通用 Sounds 音效库（非 Abilities 部分）` | `Content/Assets/Sounds/Enemies/` 等 | ~12 MB |

### Phase R：Dungeon 关卡与中资产（3 commits）

| #  | scope   | message | 路径 | 大小 |
| -- | ------- | ------- | ---- | ---- |
| R1 | content | `feat(content): Dungeon Tileset 瓦片集与楼梯材质` | `Content/Assets/Dungeon/Materials/` | ~50 MB |
| R2 | content | `feat(content): Dungeon 静态网格体（Tile/Corner/Stairs）` | `Content/Assets/Dungeon/SM_*.uasset` | ~120 MB |
| R3 | content | `feat(content): UnfCleric 牧师职业包` | `Content/Assets/UnfCleric/` | ~68 MB |

### Phase S：大型独立资产（4 commits，按目录分推）

| #  | scope   | message | 路径 | 大小 |
| -- | ------- | ------- | ---- | ---- |
| S1 | content | `feat(content): CelesAnimeShader 卡通渲染 Shader` | `Content/Assets/CelesAnimeShader/` | ~411 MB |
| S2 | content | `feat(content): ShieldFXwithBPv2 护盾特效` | `Content/Assets/ShieldFXwithBPv2/` | ~463 MB |
| S3 | content | `feat(content): Mage 主角法术体系资产` | `Content/Assets/Mage/`（含 Demo/Mannequins/Rigs/） | ~888 MB ⚠️ |
| S4 | content | `feat(content): Dungeon 关卡资源（最终集）` | `Content/Assets/Dungeon/`（除 Phase I / R 已推子目录外剩余） | ~131 MB |

---

## 合计

| Phase | commits | 类型 |
| ----- | ------- | ---- |
| A 工程层 | 5 | 纯工程 |
| B GAS 核心 | 4 | 代码 |
| C 法术 | 16（14 代码 + 2 资产） | 代码+资产 |
| D 数据 | 5 | 代码 |
| E 计算 | 3 | 代码 |
| F Actor | 8（7 代码 + 1 资产） | 代码+资产 |
| G AI | 4（3 代码 + 1 资产） | 代码+资产 |
| H 角色 | 8（3 代码 + 5 资产） | 代码+资产 |
| I 检查点 | 3（2 代码 + 1 资产） | 代码+资产 |
| J Game | 4（3 代码 + 1 资产） | 代码+资产 |
| K 输入 | 1 | 代码 |
| L 接口 | 5 | 代码 |
| M Player | 2 | 代码 |
| N UI | 9（8 代码 + 1 资产） | 代码+资产 |
| O 特效组件 | 3（2 代码 + 1 资产） | 代码+资产 |
| P Plugins | 3 | 纯插件 |
| Q 小资产 | 5 | 资产 |
| R 中资产 | 3 | 资产 |
| S 大资产 | 4 | 资产 |
| **合计** | **85 commits** | |

落在 50-96 区间。**单个 commit 最大的**是 S3（888 MB Mage），其余都 < 700 MB。

---

## 二、Push 调度（每个 commit 立即单独 push）

```powershell
cd "D:\ue_texiao\Aura"
git push origin master
```

每个 commit 都跑这条。预计 85 次 push，总耗时 ~60-120 分钟（取决于 O3/S1/S2/S3/S4 几个大头的网络）。

### 各阶段累计耗时预估

| 阶段 | commits | 单 commit 大小范围 | 累计耗时 |
| ---- | ------- | ----------------- | -------- |
| A1–A5 | 5 | < 1 MB | < 5 秒 |
| B1–B4 | 4 | < 200 KB | < 5 秒 |
| C1–C14 | 14 | < 100 KB | < 30 秒 |
| C15 | 1 | ~17 MB | 5-10 秒 |
| C16 | 1 | ~41 MB | 10-30 秒 |
| D1–E3 | 8 | < 50 KB | < 10 秒 |
| F1–F7 | 7 | < 100 KB | < 10 秒 |
| F8 | 1 | ~54 MB | 30-60 秒 |
| G1–G3 | 3 | < 100 KB | < 5 秒 |
| G4 | 1 | ~207 MB | 3-5 min |
| H1–H3 | 3 | < 100 KB | < 5 秒 |
| H4 | 1 | ~280 MB | 5-10 min |
| H5 | 1 | ~30 MB | 30-60 秒 |
| H6 | 1 | ~70 MB | 1-2 min |
| H7 | 1 | ~35 MB | 30-60 秒 |
| H8 | 1 | ~10 MB | 10-20 秒 |
| I1–I2 | 2 | < 100 KB | < 5 秒 |
| I3 | 1 | ~10 MB | 10-20 秒 |
| J1–J3 | 3 | < 100 KB | < 5 秒 |
| J4 | 1 | ~5 MB | 5-10 秒 |
| K1 | 1 | < 100 KB | < 5 秒 |
| L1–L5 | 5 | < 100 KB | < 5 秒 |
| M1–M2 | 2 | < 100 KB | < 5 秒 |
| N1–N8 | 8 | < 100 KB | < 10 秒 |
| N9 | 1 | ~70 MB | 1-2 min |
| O1–O2 | 2 | < 100 KB | < 5 秒 |
| O3 | 1 | **~750 MB** ⚠️ | **10-20 min** |
| P1–P3 | 3 | < 2 MB | 5-30 秒 |
| Q1–Q5 | 5 | 96 KB ~ 15 MB | 30-60 秒 |
| R1–R3 | 3 | 50 ~ 120 MB | 2-5 min |
| S1 | 1 | ~411 MB | 5-10 min |
| S2 | 1 | ~463 MB | 5-10 min |
| S3 | 1 | **~888 MB** ⚠️ | **20-60 min** |
| S4 | 1 | ~131 MB | 2-5 min |

---

## 三、开始前配置（已就绪）

- [x] HEAD 已删（`git update-ref -d HEAD`）
- [x] SSH 密钥 passphrase 已缓存到 ssh-agent
- [x] 远端 `git@github.com:hy396/Aura.git` 已配（远端空仓库）
- [x] `http.postBuffer = 2147483648`（2 GB）已设

**UE 编辑器必须关** —— 否则会持续写 `Intermediate/` 与 `Saved/`，弄乱 git status。

---

## 四、单 commit 命令模板（统一）

```powershell
cd "D:\ue_texiao\Aura"

# 1) add 本 commit 的文件（精确路径，禁止 git add .）
git add <path1> <path2> ...

# 2) 验证 staged 列表
git status --short
# 期望：只看到本 commit 该加的文件，且都是 "A " 开头

# 3) commit
git commit -m "<type>(<scope>): <中文主题>" `
  -m "- <要点 1>" `
  -m "- <要点 2>"

# 4) 立刻 push
git push origin master

# 5) 验证
git log --oneline -1 origin/master
```

---

## 五、Push 中断 / 失败恢复

| 现象 | 操作 |
| ---- | ---- |
| 网络断在 push 中 | 重新 `git push origin master`，自动续传 |
| 收到 `RPC failed; HTTP 504` | 加 `--no-thin`：`git push origin master --no-thin` |
| 大 commit（S3 888 MB）中途挂 | 跑 `git status` 看是否落地；落地就续传；没落地就再 push |
| 误把别文件 add 进 commit | `git reset HEAD` 撤回 staging，重新 add + commit |
| 想合并几个小 commit | `git reset HEAD~N` 撤回到工作区，重组 |

---

## 六、收尾验证

```powershell
cd "D:\ue_texiao\Aura"

# 1) 最新 15 个 commit
git log --oneline -15

# 2) origin/master 与本地 HEAD 一致
git status
# 期望：Your branch is up to date with 'origin/master'.

# 3) 完整性验证（强烈推荐）
cd "D:\ue_texiao"
git clone "D:/ue_texiao/Aura" test-clone-aura
cd test-clone-aura
ls Content/Assets/
du -sh .   # 期望 ~4 GB

cd "D:\ue_texiao"
Remove-Item -Recurse -Force test-clone-aura
```

期望 commit log（最新 → 最旧，节选关键节点）：

```
<hash> feat(content): Dungeon 关卡资源（最终集）
<hash> feat(content): Mage 主角法术体系资产
<hash> feat(content): ShieldFXwithBPv2 护盾特效
<hash> feat(content): CelesAnimeShader 卡通渲染 Shader
<hash> feat(content): UnfCleric 牧师职业包
<hash> feat(content): Dungeon 静态网格体
<hash> feat(content): Dungeon Tileset 瓦片集
<hash> feat(content): 通用 Sounds 音效库
<hash> feat(content): UI Spells 技能图标
<hash> feat(content): Fonts 字体资源
<hash> feat(content): _Text 本地化文本
<hash> feat(content): Materials 基础材质库
<hash> feat(plugin): 集成 VRM4U
<hash> feat(plugin): 集成 KawaiiPhysics
<hash> feat(plugin): 集成 RiderLink
<hash> feat(content): _Niagara 特效框架
<hash> feat(component): PassiveNiagaraComponent
<hash> feat(component): DeBuffNiagaraComponent
<hash> feat(content): UI 界面资源
<hash> feat(ui): LoadScreen HUD + MVVM
<hash> feat(ui): SpellMenuWidgetController
<hash> feat(ui): AttributeMenuWidgetController
<hash> feat(ui): OverlayWidgetController
<hash> feat(ui): AuraWidgetController
<hash> feat(ui): DamageTextComponent
<hash> feat(ui): AuraUserWidget
<hash> feat(ui): AuraHUD
<hash> feat(player): AuraPlayerState
<hash> feat(player): AuraPlayerController
<hash> feat(interface): SaveInterface
<hash> feat(interface): PlayerInterface
<hash> feat(interface): HighlightInterface
<hash> feat(interface): EnemyInterface
<hash> feat(interface): CombatInterface
<hash> feat(input): AuraInputComponent + InputConfig
<hash> feat(content): Maps 地图资产
<hash> feat(game): MyAssetManager + MyGameplayTags
<hash> feat(game): AuraGameModeBase
<hash> feat(game): AuraGameInstance
<hash> feat(content): Checkpoint 资产
<hash> feat(checkpoint): MapEntrance + LoadScreenSaveGame
<hash> feat(checkpoint): Checkpoint
<hash> feat(content): 角色通用 Materials
<hash> feat(content): Feibi 角色资产
<hash> feat(content): Kerota 角色资产
<hash> feat(content): Momo + Aura 角色资产
<hash> feat(content): Rokoko 主角资产
<hash> feat(character): AuraEnemy
<hash> feat(character): AuraCharacter
<hash> feat(character): AuraCharacterBase
<hash> feat(content): Enemies 敌人资产
<hash> feat(ai): BTTask_Attack
<hash> feat(ai): BTService_FindNearestPlayer
<hash> feat(ai): AuraAIController
<hash> feat(content): MagicCircle + Pickups
<hash> feat(actor): EnemySpawnVolume
<hash> feat(actor): EnemySpawnPoint
<hash> feat(actor): PointCollection
<hash> feat(actor): AuraEffectActor
<hash> feat(actor): MagicCircle
<hash> feat(actor): AuraFireBall
<hash> feat(actor): AuraProjectile
<hash> feat(modmagcalc): MMC_MaxMana
<hash> feat(modmagcalc): MMC_MaxHealth
<hash> feat(execcalc): ExecCalc_Damage
<hash> feat(data): LootTiers
<hash> feat(data): LevelUpInfo
<hash> feat(data): CharacterClassInfo
<hash> feat(data): AttributeInfo
<hash> feat(data): AbilityInfo
<hash> feat(content): Abilities 音效库 + BlinkAndDashVFX
<hash> feat(content): Spells + Fire/Combat 通用特效
<hash> feat(asynctasks): WaitCooldownChange
<hash> feat(abilitytasks): TargetDataUnderMouse
<hash> feat(passive): AuraPassiveAbility
<hash> feat(spell): AuraMeleeAttack
<hash> feat(spell): AuraSummonAbility
<hash> feat(spell): ProjectileBarrage
<hash> feat(spell): ProjectileSpell
<hash> feat(spell): ArcaneShards
<hash> feat(spell): Electrocute
<hash> feat(spell): AuraBeamSpell
<hash> feat(spell): AuraFireBlast
<hash> feat(spell): AuraFireBolt
<hash> feat(ability): AuraDamageGameplayAbility
<hash> feat(ability): AuraGameplayAbility
<hash> feat(abilitysystem): Globals + Library
<hash> feat(attributeset): AuraAttributeSet
<hash> feat(abilitysystem): AuraAbilitySystemComponent
<hash> feat(types): AuraAbilityTypes
<hash> chore(cleanup): 清理已删除 .idea IDE 文件
<hash> chore(project): 更新 Aura.uproject 与 .sln
<hash> chore(config): 更新 Config/*.ini
<hash> chore(build): 配置 Aura.Build.cs
<hash> chore(.gitignore): 增加嵌套 .gitignore 忽略规则
```

---

## 七、风险与注意

| 风险 | 缓解 |
| ---- | ---- |
| 85 个 commit 手动过程长 | 已给完整模板；按 § 四复制略改路径即可 |
| Push 中途断 | git 默认续传 |
| UE 编辑器开着扰 git status | § 三已要求关编辑器 |
| Plugins/Binaries 被错误入库 | 已有 `Plugins/**/Binaries/*` ignore；VRM4U 的 `ThirdParty/assimp/` `rapidjson/` 需在 P3 之前补 .gitignore |
| 单 commit 太大 push 失败 | 撤回 `git reset HEAD~1`，按更细粒度重切 |

---

## 八、当前阻塞

✅ **无阻塞**。HEAD 已删，ssh-agent 已缓存，远端空仓库已就绪。

可以直接开始 A1 → A2 → ... → S4 顺序推进。

---

## 历史归档

### DragonOath Step 5（2026-07-16）

7 个原子 commit 全部成功推送至 `hy396/DragonOath` master。涉及 GAS 三层重构（数值 / 行为 / 流程）、Lyra GameplayMessageRouter 移植、本地消息总线文档改写。

经验：网络不通时改用 `git reset HEAD~N` + 操作手册 + 90 天 `git reflog` 安全网。

### V4 → V5（2026-07-16）

V4 是「先全部代码 → 再全部资产」分两段推送。中间态引用断裂。

V5 改为「按模块代码+资产交错」：每个 C++ 模块 commit 完后紧跟对应 Content 资产 commit。commit 数从 90 调整到 85。

### V3 → V4 演进

V3：33 commits / 1 push。  
V4：90 commits / 12 pushes（合并 push）。  
V5：85 commits / 每 commit 单独 push（用户决定）。