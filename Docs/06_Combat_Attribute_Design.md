# 06. Combat Attribute Design

状态：基于龙斗士原游戏复刻设计，已通过技术评审，可按阶段实施。

## 原游戏概要

龙斗士是百田 2011 年上线的横版动作页游，已于 2024 年前后停止更新。核心战斗是 2D 横版动作格斗，强调连招、走位、技能释放节奏。

七大基础职业：战士、弓箭手、法师、骑士、枪炮师、魔剑士、召唤师。每个职业可觉醒为进阶形态（如战士觉醒为炎龙战士）。

职业属性排名（原游戏数据）：

```text
攻击：法师 > 魔剑士 > 弓箭手 > 枪炮师 > 战士 > 骑士
生命：骑士 > 战士 > 枪炮师 > 魔剑士 > 弓箭手 > 法师
防御：骑士 > 战士 > 枪炮师 > 魔剑士 > 弓箭手 > 法师
法力：法师 > 魔剑士 > 弓箭手 > 枪炮师 > 战士 > 骑士
速度：魔剑士 > 战士 > 法师 > 弓箭手 > 枪炮师 > 骑士
```

原游戏的角色面板属性只有五项：**攻击、防御、生命、魔法、致命**。装备和宠物守护都加这五项，不区分物理攻击和魔法攻击。

## 设计目标

复刻龙斗士原游戏的属性体系，同时适配 UE5 GAS 架构。

核心原则：

```text
忠实复刻原游戏的属性分层和来源系统。
统一攻击力和防御力，不区分物理/魔法。
宠物守护神是核心成长系统，不是附属功能。
装备强化（+1 到 +15）是主要属性来源。
横版动作格斗：操作 > 数值，属性决定下限，操作决定上限。
```

## 属性分层

### 一级属性（基础属性）

原游戏有三项基础属性，影响二级属性的转换：

| 属性 | 英文 | 影响的二级属性 |
|---|---|---|
| 力量 | Strength | 攻击力、少量生命、少量防御 |
| 敏捷 | Agility | 攻击速度、命中率、闪避率、移动速度 |
| 智力 | Intelligence | 魔法值、魔法回复、少量攻击 |

一级属性主要给玩家使用。怪物直接配置最终战斗属性，不走一级属性转换。

转换比例（起始值，后续根据手感调整）：

```text
1 力量  -> 1.5 AttackPower + 0.5 DefensePower + 2 MaxHealth
1 敏捷  -> 0.3% HitChance + 0.1% EvasionChance + 0.2 AttackSpeed + 0.5 MoveSpeed
1 智力  -> 5 MaxMana + 0.5 ManaRegen + 0.3 AttackPower
```

### 动作资源

体力（Stamina）是独立的动作资源条，只用于冲刺/闪避消耗，不受一级属性影响。

```text
Stamina / MaxStamina    冲刺、闪避等动作消耗，消耗后自动回复
```

MaxStamina 的值固定或随角色等级成长，不通过一级属性转换。这与原游戏一致：体力只是一个动作限制条，不是成长属性。

### 二级属性（战斗属性）

核心战斗属性（原游戏面板五项）：

| 属性 | 英文 | 说明 |
|---|---|---|
| 最大生命 | MaxHealth | 生命值上限 |
| 最大魔法 | MaxMana | 魔法值上限，用于技能消耗 |
| 攻击力 | AttackPower | 伤害基础值，所有技能共用 |
| 防御力 | DefensePower | 减少受到的所有伤害 |
| 致命 | CriticalRating | 致命一击的数值，通过公式换算为暴击率 |

进阶战斗属性：

| 属性 | 英文 | 说明 |
|---|---|---|
| 暴击伤害倍率 | CritDamageRate | 暴击时伤害倍率，默认 1.5 |
| 命中 | HitRating | 命数值，换算为命中率 |
| 闪避 | EvasionRating | 闪避数值，换算为闪避率 |
| 攻击速度 | AttackSpeed | 影响攻击动画速度和技能前摇 |
| 移动速度 | MoveSpeed | 影响角色移动速度 |
| 吸血 | LifeStealRate | 按最终伤害比例回复生命 |

回复属性：

| 属性 | 英文 | 说明 |
|---|---|---|
| 生命回复 | HealthRegen | 每秒回复生命值 |
| 魔法回复 | ManaRegen | 每秒回复魔法值 |

### 元素属性

原游戏有五系元素攻击，每系独立配置：

| 属性 | 英文 | 说明 |
|---|---|---|
| 炎火攻击 | FireAttack | 火属性伤害附加 |
| 雷电攻击 | LightningAttack | 雷属性伤害附加 |
| 冰冻攻击 | IceAttack | 冰属性伤害附加，可能附带减速 |
| 光明攻击 | LightAttack | 光属性伤害附加 |
| 黑暗攻击 | DarkAttack | 暗属性伤害附加 |

元素抗性（第一阶段统一，后期可拆分五系）：

| 属性 | 英文 | 说明 |
|---|---|---|
| 元素抗性 | ElementResistance | 减少受到的元素伤害 |

## 等级系统

### 等级来源

攻击者等级和防御者等级在伤害公式中频繁使用，需要统一获取方式：

```text
玩家等级：存在 ADOPlayerState 中，通过 ASC 的 AvatarActor -> PlayerState 获取
怪物等级：存在 ADOCharacter 的成员变量中，由怪物数据表配置
```

ExecutionCalculation 中获取等级的统一接口：

```cpp
// 在 ExecutionCalculation 中
const AActor* SourceActor = Spec.GetEffectContext().GetOriginalInstigator();
// 玩家：Cast 到 APlayerState 或通过 AvatarActor -> PlayerState
// 怪物：Cast 到 ADOCharacter 读取 Level 成员

int32 GetAttackerLevel(const FGameplayEffectSpec& Spec) const
{
    if (const AActor* Avatar = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetAvatarActor())
    {
        if (const APlayerState* PS = Cast<APlayerState>(Avatar->GetOwner()))
        {
            return PS->GetPlayerId(); // 或自定义 Level 字段
        }
        if (const ADOCharacter* Character = Cast<ADOCharacter>(Avatar))
        {
            return Character->GetCharacterLevel();
        }
    }
    return 1; // 保底
}
```

后续可以在 `UDOAbilitySystemComponent` 中提供 `GetCharacterLevel()` 辅助函数统一访问。

## 伤害公式

### 技能参数传递

技能的基础伤害和倍率通过 **SetByCaller** 传递给伤害 GE 的 ExecutionCalculation。

SetByCaller Tag 定义（加入 `DOGameplayTag.h`）：

```text
Data.Damage             技能基础伤害
Data.DamageMultiplier   技能伤害倍率
```

技能激活时创建伤害 GE 并设置 SetByCaller：

```cpp
// 在 GA 中创建伤害 GE
FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageGEClass, GetAbilityLevel(), ASC->MakeEffectContext());
SpecHandle.Data->SetSetByCallerMagnitude(DragonOathGameplayTags::Data::Damage, SkillBaseDamage);
SpecHandle.Data->SetSetByCallerMagnitude(DragonOathGameplayTags::Data::DamageMultiplier, SkillDamageMultiplier);
ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
```

ExecutionCalculation 中通过 `Spec.GetSetByCallerMagnitude(Tag)` 读取。

### 基础伤害

原游戏的伤害公式是简单减法，复刻时保留这一特点但加入保底机制：

```text
RawDamage = SkillBaseDamage + AttackPower
MitigatedDamage = max(1, RawDamage - DefensePower)
FinalDamage = MitigatedDamage * SkillDamageMultiplier
```

`max(1, ...)` 保证最低 1 点伤害，避免防御过高导致打不动。

所有技能都读取统一的 AttackPower 和 DefensePower，不同职业的差异通过技能本身的 SkillBaseDamage 和 SkillDamageMultiplier 体现。法师的技能基础伤害和倍率更高，所以面板攻击力排名最高。

### 暴击（致命）

致命是数值型属性，通过公式换算为暴击率：

```text
CritChance = CriticalRating / (CriticalRating + CritScale)
```

其中 `CritScale = 200 + AttackerLevel * 10`，保证暴击率有收益递减。

暴击时伤害：

```text
FinalDamage = FinalDamage * CritDamageRate
```

默认 `CritDamageRate = 1.5`。

UI 显示策略：面板上**显示换算后的暴击率百分比**（如 "33%"），后台用 Rating 计算。这样玩家理解直观，系统保留收益递减的数值深度。宠物守护的"致命守护"给的是 Rating 数值。

示例（等级 10 时 CritScale = 300）：

```text
CriticalRating = 30  -> 暴击率约 9%
CriticalRating = 75  -> 暴击率约 20%
CriticalRating = 150 -> 暴击率约 33%
CriticalRating = 300 -> 暴击率约 50%
```

### 命中和闪避

**设计原则**：横版动作游戏中，玩家通过走位和技能范围决定是否命中。命中/闪避主要用于自动攻击、召唤物、怪物攻击玩家。

三种命中模式：

```text
1. 玩家攻击怪物：不走命中判定，必中（靠走位和技能范围决定）
2. 怪物攻击玩家：走命中/闪避判定
3. 自动战斗/召唤物：走命中/闪避判定
```

命中公式（仅模式 2、3 使用）：

```text
FinalHitChance = Clamp(BaseHitChance + AttackerHitRating / HitScale - DefenderEvasionRating / EvasionScale, 0.05, 0.98)
```

初始参数：

```text
BaseHitChance = 0.90
HitScale = 200 + AttackerLevel * 10
EvasionScale = 200 + DefenderLevel * 10
```

ExecutionCalculation 中通过伤害 GE 的 `EffectContext` 或 `SetByCaller` 标记伤害来源类型，决定是否走命中判定。建议用 GameplayTag 区分：

```text
Damage.Type.Player       玩家造成的伤害，跳过命中判定
Damage.Type.Monster      怪物造成的伤害，走命中判定
Damage.Type.Pet          召唤物造成的伤害，走命中判定
```

### 元素伤害

元素伤害独立于基础伤害，不吃 DefensePower：

```text
ElementDamage = FireAttack + LightningAttack + IceAttack + LightAttack + DarkAttack
FinalElementDamage = ElementDamage * (1 - ElementReduction)
```

元素抗性公式：

```text
ElementReduction = ElementResistance / (ElementResistance + ResistanceScale)
```

其中 `ResistanceScale = 100 + DefenderLevel * 10`。

**平衡限制**：元素攻击属性有上限约束，通过 GE 的 Modifier 上限或 PreAttributeChange Clamp 控制。建议单系元素攻击上限不超过 `AttackerLevel * 20`，避免无元素抗性的目标受到过高伤害。PVP 场景中可额外乘以 0.5 系数。

### 吸血

```text
Heal = FinalDamage * LifeStealRate
```

只有带 `Damage.CanLifeSteal` 标签的伤害才触发吸血。持续伤害、反伤、环境伤害默认不触发。

### 计算顺序

```text
1. 读取攻击者 AttackPower 和技能基础伤害/倍率（SetByCaller）
2. 判定伤害来源类型（玩家/怪物/召唤物）
3. 命中判定（仅怪物/召唤物攻击走判定）
4. 计算基础伤害 = SkillBaseDamage + AttackPower
5. 防御减免 = max(1, 基础伤害 - DefensePower)
6. 暴击判定，如果暴击乘以 CritDamageRate
7. 元素伤害单独计算（不吃防御）
8. 最终伤害 = 基础伤害 + 元素伤害
9. 吸血计算
```

## 霸体与韧性

### 第一阶段：Tag 霸体

原游戏的横版动作战斗中，霸体是重要的战斗机制。第一阶段只做简单的 Tag 霸体，不引入韧性系统：

```text
Status.SuperArmor    当前处于霸体状态，用 GameplayTag 表示
```

各单位配置：

```text
普通怪：没有霸体，受击即硬直
精英怪：部分技能期间加 Status.SuperArmor
Boss：阶段或技能期间加 Status.SuperArmor
玩家：某些技能释放期间获得短霸体（如战士的战神之躯）
```

受击逻辑中检查 `Status.SuperArmor`：有霸体的单位不进入硬直状态，但仍然承受伤害。

### 第二阶段（后续单独设计）：韧性系统

韧性（Poise）系统涉及更多设计细节，后续单独设计，不放在本文档中：

- 韧性减少规则：每次受击减固定值？还是按伤害比例？
- 韧性恢复规则：脱战后多久恢复？受击后多久开始恢复？
- 韧性归零后的效果：进入硬直？硬直时长？
- 霸体与韧性的关系：霸体期间韧性不减？还是霸体期间韧性归零才被打断？

## 玩家属性来源

原游戏中玩家最终属性来自多层叠加，这是复刻的重点：

```text
职业基础属性
+ 等级成长（每次升级自动提升一级属性）
+ 觉醒加成（觉醒后额外属性提升）
+ 装备属性（武器、防具、饰品）
+ 装备强化（+1 到 +15，每级提升装备属性）
+ 宠物守护神（攻击、防御、生命、魔法、致命五项守护）
+ 称号属性加成
+ 圣衣/时装属性加成
+ 技能被动加成
+ 临时 Buff / Debuff
= 最终 AttributeSet 数值
```

### 职业基础属性

每个职业有不同的基础属性和成长率：

| 职业 | 生命成长 | 防御成长 | 攻击成长 | 速度 |
|---|---|---|---|---|
| 骑士 | 最高 | 最高 | 最低 | 最慢 |
| 战士 | 高 | 高 | 低 | 较快 |
| 枪炮师 | 中 | 中 | 中 | 较慢 |
| 魔剑士 | 中 | 中 | 高 | 最快 |
| 弓箭手 | 低 | 低 | 中 | 中 |
| 法师 | 最低 | 最低 | 最高 | 中 |

### 装备系统

装备品质：

```text
白色（普通）< 蓝色（优秀）< 紫色（稀有）< 橙色（传说）
```

装备强化：

```text
1-15 级装备：最高强化到 +6
16 级以上装备：最高强化到 +15
```

强化材料：

```text
1 级强化石：所有强化等级
2 级强化石：+6 以上
3 级强化石：+10 以上或紫色品质以上
```

每级强化提升装备基础属性的一定百分比。

### 宠物守护神

原游戏的核心成长系统。宠物可以设置为守护神，为角色提供额外属性。

五项守护属性对应面板五项：

| 守护类型 | 说明 | 典型宠物 |
|---|---|---|
| 攻击守护 | 增加攻击力 | 夜兔王、毁灭天兽、赤焰斗神 |
| 防御守护 | 增加防御力 | 海魔王、铁甲威龙、泰坦 |
| 生命守护 | 增加最大生命 | 创世神罗、暴风战神、米兰英雄 |
| 魔法守护 | 增加最大魔法 | 海洋女神、海神之子、蓝海元素 |
| 致命守护 | 增加致命值 | 超龙王雷恩、古拉兽、七彩精灵 |

守护值取决于宠物自身的对应属性数值和星级。守护石可以进一步提升守护效果。

GAS 实现方案（使用 MMC 动态计算）：

普通 GE 的 Modifier Magnitude 不支持运行时动态计算，使用自定义 **ModifierMagnitudeCalculation（MMC）** 实现宠物守护的动态属性加成：

```text
每个宠物守护类型对应一个 MMC：
MMC_PetAttackGuard    读取宠物组件的攻击守护值，输出到 AttackPower Modifier
MMC_PetDefenseGuard   读取宠物组件的防御守护值，输出到 DefensePower Modifier
MMC_PetHealthGuard    读取宠物组件的生命守护值，输出到 MaxHealth Modifier
MMC_PetManaGuard      读取宠物组件的魔法守护值，输出到 MaxMana Modifier
MMC_PetCriticalGuard  读取宠物组件的致命守护值，输出到 CriticalRating Modifier
```

MMC 优势：
- 可以在 GE 配置中复用
- 支持 CurveTable 做星级映射
- 运行时读取宠物组件属性，宠物升级/换装自动生效

### 称号系统

称号按品质提供不同额度的属性加成：

```text
白色 < 红色 < 蓝色 < 金色 < 紫色
```

金色和紫色称号提供最大属性加成。同一时间只能激活一个称号。

### 圣衣/时装

商城购买的圣衣除了外观变化，还提供额外属性：

```text
例：星皇圣衣 -> +1000 攻击力、+6000 生命、+3000 防御
```

## 怪物属性来源

怪物直接配置最终战斗属性，不走一级属性转换。

怪物数据表字段：

```text
MonsterId
DisplayName
Level
Rank                Normal / Heavy / Elite / Boss
MaxHealth
AttackPower
DefensePower
CriticalRating
HitRating
EvasionRating
AttackSpeed
MoveSpeed
ElementResistance
HealthRegen
Tags
```

怪物减伤建议范围：

| 类型 | 减伤 | 设计目的 |
|---|---:|---|
| 普通小怪 | 5% - 12% | 打起来爽，不拖时间 |
| 厚血小怪 | 10% - 18% | 主要靠血量 |
| 盾兵/重甲怪 | 18% - 28% | 明确表现"硬"，但要有破盾或背击解法 |
| 精英怪 | 15% - 25% | 比普通怪稳 |
| Boss | 20% - 35% | 主要靠血量和机制 |
| 免伤阶段 | 40% - 60% | 短时间机制，非常驻 |

## GAS 落地

### AttributeSet 规划

第二阶段已完成拆分，`DOPlaySet` 已解散，拆为 `DOResourceSet` + `DOCombatSet`：

```text
DOHealthSet       Health / MaxHealth / Damage / Healing / HealthRegen（新增）
DOResourceSet     Mana / MaxMana / Stamina / MaxStamina / ManaRegen（新增，承接自 DOPlaySet）
DOCombatSet       AttackPower / DefensePower / MoveSpeed（迁移自 DOPlaySet）
                  CriticalRating / CritDamageRate（新增）
                  HitRating / EvasionRating（新增）
                  AttackSpeed / LifeStealRate（新增）
```

后续阶段结构（尚未实现）：

```text
DOElementSet      FireAttack / LightningAttack / IceAttack / LightAttack / DarkAttack
                  ElementResistance
DOPrimarySet      Strength / Agility / Intelligence（仅玩家，Phase 3）
```

注册方式：属性集作为 `CreateDefaultSubobject` 挂在 ASC 拥有者上，ASC 在 `InitAbilityActorInfo` 时自动发现并注册。

- 玩家：属性集挂在 `ADOPlayerState`（玩家 ASC 的拥有者）上。
- 怪物 / NPC：属性集挂在 `ADOCharacter` 自身（怪物 ASC 的拥有者）上，因此 `ADOCharacter` 构造函数同样创建这三个属性集；玩家 Pawn 继承该构造函数会产生冗余实例，但不会参与玩家 ASC 注册，无副作用。

迁移引用清单（第二阶段已统一更新）：

```text
DOPlayerState.h/.cpp                 PlaySet -> ResourceSet + CombatSet，新增 GetResourceSet()/GetCombatSet()
DOCharacter.cpp                      UDOPlaySet::GetMoveSpeedAttribute() -> UDOCombatSet::GetMoveSpeedAttribute()
DOExecutionCalculation_Damage.cpp    AttackPower/DefensePower 捕获改为 UDOCombatSet
ADOCharacter.cpp                     构造函数创建 HealthSet/ResourceSet/CombatSet 供怪物使用
DOPlaySet.h/.cpp                     已删除
```

### 回复属性实现

HealthRegen 和 ManaRegen 使用 **Periodic GE** 实现每秒回复：

```text
回复 GE 配置：
Duration = Infinite（或按 Buff 时长）
Period = 1.0s
Modifier = +HealthRegen（或 +ManaRegen）的值，施加到 Healing（或直接 Health）

实现方式：
GE 每 1 秒触发一次，ExecutionCalculation 或 Modifier 读取 HealthRegen 属性值，
作为治疗量施加。这样 HealthRegen 属性值变化时，回复量自动更新。
```

### 攻击速度实现

AttackSpeed 影响攻击动画速度和技能前摇，通过 GA 基类辅助函数实现：

```cpp
// UDOGameplayAbility
float GetAttackSpeed() const
{
    if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (const UDOCombatSet* CombatSet = ASC->GetSet<UDOCombatSet>())
        {
            return CombatSet->GetAttackSpeed();
        }
    }
    return 1.0f; // 保底
}
```

子类在 PlayMontage 时设置 Rate：

```cpp
UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    this, NAME_None, AttackMontage, GetAttackSpeed());
```

Phase 1 中 AttackSpeed 还没加入 AttributeSet，GetAttackSpeed 返回 1.0f，不影响现有技能。

### GameplayEffect 分层

```text
职业基础 GE      -> 一级属性（力量/敏捷/智力）
等级成长 GE      -> 每级提升一级属性
觉醒 GE          -> 觉醒后额外属性
装备 GE          -> 攻击/防御等二级属性
强化 GE          -> 装备属性百分比提升
宠物守护 GE      -> 攻击/防御/生命/魔法/致命（MMC 动态计算）
称号 GE          -> 全属性加成
圣衣 GE          -> 固定属性加成
技能被动 GE      -> 各类属性加成
Buff/Debuff GE   -> 临时属性变化
```

### 移动速度落地

MoveSpeed 是 GAS 属性，但 `CharacterMovementComponent::MaxWalkSpeed` 不是。需要桥接。

推荐做法：在角色类中用 `GetGameplayAttributeValueChangeDelegate` 监听 MoveSpeed 属性变化，直接写入 `MaxWalkSpeed`。这比在 AttributeSet 的 `PostGameplayEffectExecute` 中处理更清晰，因为属性变化的响应属于角色行为，不属于属性集职责。

```cpp
// DOCharacter.h
protected:
    void BindAttributeChangeDelegates();
    void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
```

```cpp
// DOCharacter.cpp
void ADOCharacter::BindAttributeChangeDelegates()
{
    if (UDOAbilitySystemComponent* DOASC = GetDOAbilitySystemComponent())
    {
        DOASC->GetGameplayAttributeValueChangeDelegate(
            UDOPlaySet::GetMoveSpeedAttribute()
        ).AddUObject(this, &ADOCharacter::OnMoveSpeedChanged);
    }
}

void ADOCharacter::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
    // MoveSpeed 属性值就是最终移动速度，直接写入
    GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}
```

绑定时机：在 ASC 初始化（`InitAbilityActorInfo`）之后调用 `BindAttributeChangeDelegates`。玩家在 `PossessedBy` / `OnRep_PlayerState` 中调用，怪物在 `ServerSideInit` 中调用。

设计说明：

```text
MoveSpeed 属性 = 角色最终移动速度
职业基础速度、装备加成、Buff 减速全部通过 GE 叠加修改 MoveSpeed
MaxWalkSpeed = MoveSpeed 当前值
```

不设单独的 BaseMoveSpeed，职业速度差异、装备加成、Buff 效果全部走 GE 叠加，统一通过 MoveSpeed 属性驱动 `MaxWalkSpeed`。这样所有速度修改方式一致，不需要区分"基础值"和"加成值"。

冲刺技能中 `GetMaxSpeed()` 读取的就是 `CharacterMovementComponent->GetMaxSpeed()`，即 MoveSpeed 的当前值，用于 RootMotion 结束时的速度限制。

怪物同样适用：怪物数据表中的 `MoveSpeed` 字段通过 GE 施加后，通过同一个 delegate 自动同步到 `MaxWalkSpeed`。

### ExecutionCalculation

伤害公式放在 ExecutionCalculation 中，不写在 AttributeSet 里：

```text
读取攻击者：AttackPower / CriticalRating / HitRating / ElementAttack
读取目标：DefensePower / EvasionRating / ElementResistance
读取 SetByCaller：SkillBaseDamage / SkillDamageMultiplier
计算：命中判定 -> 防御减免 -> 暴击 -> 元素伤害 -> 吸血
输出：Damage Meta Attribute
```

### 网络复制

玩家 ASC 使用 `Mixed` 复制模式。Mixed 模式下，GameplayEffect 只复制给 Owner 客户端，非 Owner 客户端看不到 GE。但 AttributeSet 中的属性值通过 `ReplicatedUsing` 复制给所有客户端。

属性可见性规划：

```text
所有客户端可见（ReplicatedUsing）：
  Health / MaxHealth          血条需要所有人看到
  Mana / MaxMana              法力条需要所有人看到
  AttackPower / DefensePower  PVP 面板需要看到
  MoveSpeed                   客户端预测移动需要
  Stamina / MaxStamina        动作资源需要本地预测
  CriticalRating / 等         PVP 面板需要看到
  元素攻击/抗性               PVP 面板需要看到

仅 Owner 可见（通过 GE 不复制实现）：
  Strength / Agility / Intelligence  一级属性只影响 Owner 的 GE 计算
  HealthRegen / ManaRegen            回复属性只影响 Owner 的 Periodic GE
```

伤害 Meta Attribute（Damage / Healing）不同步，只在服务端 PostGameplayEffectExecute 中处理。

## 实施阶段

### 第一阶段：核心战斗闭环

当前 DOPlaySet 已有 AttackPower / DefensePower，不需要改造。在 DOPlaySet 中新增 MoveSpeed 属性（不拆分，Phase 2 再拆）：

```text
Health / MaxHealth / Damage / Healing
Mana / MaxMana
Stamina / MaxStamina
AttackPower / DefensePower
MoveSpeed（新增）
```

实施内容：

```text
1. DOPlaySet 新增 MoveSpeed 属性
2. DOCharacter 实现 MoveSpeed delegate 绑定
3. DOGameplayTag 新增 Data.Damage / Data.DamageMultiplier
4. 创建 UDOExecutionCalculation_Damage 基础伤害公式
5. UDOAbilitySystemComponent 新增 GetCharacterLevel() 辅助函数
6. 创建测试用伤害 GE 蓝图
```

### 第二阶段：进阶属性 + AttributeSet 拆分（已完成）

目标：解散 `DOPlaySet`，拆为 `DOResourceSet` + `DOCombatSet`，补齐进阶战斗属性（暴击、命中、闪避、攻速、吸血）与回复属性，并扩展伤害结算。

拆分后结构：

```text
DOHealthSet       Health / MaxHealth / Damage / Healing / HealthRegen（新增）
DOResourceSet     Mana / MaxMana / Stamina / MaxStamina / ManaRegen（新增）
DOCombatSet       AttackPower / DefensePower / MoveSpeed（迁移）
                  CriticalRating / CritDamageRate（新增）
                  HitRating / EvasionRating（新增）
                  AttackSpeed / LifeStealRate（新增）
```

#### 属性集职责

```cpp
// DOResourceSet：资源与回复
UCLASS()
class UDOResourceSet : public UDOAttributeSet
{
    // Mana / MaxMana / Stamina / MaxStamina：复制、Clamp 同原 DOPlaySet
    // ManaRegen：COND_OwnerOnly 复制，>= 0，仅被回复 Periodic GE 读取
};

// DOCombatSet：战斗输入与进阶战斗属性
UCLASS()
class UDOCombatSet : public UDOAttributeSet
{
    // AttackPower / DefensePower / CriticalRating / HitRating /
    // EvasionRating / AttackSpeed / LifeStealRate：>= 0
    // MoveSpeed：>= 1，通过 delegate 桥接 CharacterMovementComponent::MaxWalkSpeed
    // CritDamageRate：>= 1，默认 1.5
};
```

初始化默认值（仅作保底，最终值由 GE 叠加决定）：

```text
AttackPower = 10, DefensePower = 0, MoveSpeed = 500
CriticalRating = 0, CritDamageRate = 1.5
HitRating = 0, EvasionRating = 0
AttackSpeed = 1.0, LifeStealRate = 0
Mana = 100, MaxMana = 100, Stamina = 100, MaxStamina = 100, ManaRegen = 0
```

#### 迁移清单

- `DOPlayerState`：`PlaySet` 拆为 `ResourceSet` + `CombatSet`，新增 `GetResourceSet()` / `GetCombatSet()`。
- `DOCharacter`：`MoveSpeed` 属性迁移到 `DOCombatSet`，委托绑定改为 `UDOCombatSet::GetMoveSpeedAttribute()`。
- `ADOCharacter` 构造函数同时创建 `HealthSet / ResourceSet / CombatSet` 三个属性集，使怪物（ASC 挂在自身）自动拥有完整属性；玩家 Pawn 的冗余实例不参与玩家 ASC 注册。
- `DOExecutionCalculation_Damage`：AttackPower / DefensePower 改为从 `UDOCombatSet` 捕获。
- 删除 `DOPlaySet.h / .cpp`。

#### 新增 GameplayTag

```text
Damage.Type.Player    玩家伤害，跳过命中判定（必中）
Damage.Type.Monster   怪物伤害，走命中/闪避判定
Damage.Type.Pet       召唤物/宠物伤害，走命中/闪避判定
Damage.CanLifeSteal   该次伤害触发吸血
```

这些 Tag 配置在伤害 GE 的 Source Tags 上，ExecutionCalculation 通过 `Spec.CapturedSourceTags` 读取。

#### ExecutionCalculation 扩展（暴击 / 命中 / 闪避）

```text
读取：AttackPower/CriticalRating/CritDamageRate/HitRating（Source, UDOCombatSet）
      DefensePower/EvasionRating（Target, UDOCombatSet）
      AttackerLevel / DefenderLevel（AvatarActor -> ADOCharacter::GetCharacterLevel）
      Damage.Type.*（Spec.CapturedSourceTags）

1. 基础伤害：max(1, (SkillBaseDamage + AttackPower) - DefensePower) * SkillDamageMultiplier
2. 命中判定（仅 Damage.Type.Monster / Damage.Type.Pet）：
   FinalHitChance = Clamp(0.90 + HitRating/(200+Lv*10) - EvasionRating/(200+Lv*10), 0.05, 0.98)
   未命中则 FinalDamage = 0
3. 暴击判定：CritChance = CriticalRating / (CriticalRating + 200 + Lv*10)
   命中阈值则 FinalDamage *= CritDamageRate
4. 命中且 FinalDamage>0：输出 Damage Meta Attribute
```

> 吸血（LifeSteal）不放在 ExecutionCalculation 里：伤害 GE 施加在目标（受害者）上，
> ExecutionCalculation 只能输出到目标属性，无法为来源回血（否则会错误地治疗受害者）。
> 吸血改由 `UDOAbilitySystemComponent::ApplyDamageToTarget` 统一入口处理：伤害 GE 施加到目标后，
> 若 SourceTags 含 `Damage.CanLifeSteal`，在服务端为来源按
> `LifeStealRate * (SkillBaseDamage + AttackPower)` 估算回血（`UDOHealthSet::Healing` Meta 已就绪，可作精确回血 GE 的落点）。

> 预测说明：暴击与命中的随机判定当前使用 `FMath::FRand()`。在 LocalPredicted 下，客户端预测值与服务端权威值可能短暂不一致，由 GAS 预测系统回滚修正。后续如需完全一致，可改为基于 SpecHandle + 帧号的确定性随机。

#### 攻速辅助

`UDOGameplayAbility::GetAttackSpeed()` 从 `UDOCombatSet` 读取 `AttackSpeed`，缺省返回 1.0f。子类的 `PlayMontageAndWait` 使用该值作为播放速率（Phase 1 中该函数已存在但恒返回 1.0f）。

#### 回复属性（Periodic GE）

- `HealthRegen`（DOHealthSet）/ `ManaRegen`（DOResourceSet）为普通复制属性。
- 回复由 `GE_Regen_Health` / `GE_Regen_Mana` 实现：Duration = Infinite，Period = 1.0s，Modifier 读取对应 Regen 属性值，周期性施加到 Health / Mana。
- 这两张 GE 为蓝图资产，C++ 侧只需保证属性与 `UDOHealthSet::PostGameplayEffectExecute` 的伤害/治疗转换就绪（Healing Meta 已在此处转换为生命回复）。

### 第三阶段：一级属性和成长

新增 DOPrimarySet（仅玩家）：

```text
DOPrimarySet      Strength / Agility / Intelligence
```

实施内容：

```text
1. 实现一级属性到二级属性的转换 GE（MMC 或 ExecutionCalculation）
2. 实现职业基础属性和等级成长 GE
3. PlayerState 新增等级字段
```

### 第四阶段：元素系统

新增 DOElementSet：

```text
DOElementSet      FireAttack / LightningAttack / IceAttack / LightAttack / DarkAttack
                  ElementResistance
```

实施内容：

```text
1. 新建 DOElementSet
2. 扩展 ExecutionCalculation 加入元素伤害计算
3. 元素攻击属性上限 Clamp
```

### 第五阶段：成长系统

```text
装备系统 GE（装备属性 + 强化等级）
宠物守护神 GE（五项守护属性，MMC 动态计算）
称号系统 GE
圣衣系统 GE
觉醒系统 GE
```

## 职业技能消耗

原游戏中技能消耗魔法值。不同职业的魔法值上限差异很大：

```text
法师：魔法值最高，技能消耗大
骑士：魔法值最低，技能消耗小或不需要
```

技能 GE 设置 Mana 消耗：

```text
Cost GE -> Mana -= SkillManaCost
```

如果 Mana 不足，CanActivateAbility 返回 false。

部分技能可能消耗生命值代替魔法值（如战士的某些特殊技能）。

## 总结

本方案忠实复刻龙斗士原游戏的属性体系，核心改动点：

```text
统一攻击力和防御力，不区分物理/魔法
致命从简单百分比改为数值型 Rating 系统（UI 显示百分比）
补全宠物守护神作为核心成长系统（MMC 动态计算）
装备强化 +1 到 +15 作为主要属性来源
伤害公式回归简单减法，加保底机制
一级属性为力量/敏捷/智力三项，体力改为独立动作资源
新增攻击速度属性
技能参数通过 SetByCaller 传递给 ExecutionCalculation
玩家攻击必中，怪物/召唤物攻击走命中判定
第一阶段只做 Tag 霸体，韧性系统后续单独设计
```
