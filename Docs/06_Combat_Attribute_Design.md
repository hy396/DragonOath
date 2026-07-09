# 06. Combat Attribute Design

状态：基于龙斗士原游戏复刻设计，先评审，不直接实施代码。

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
| 命中 | HitRating | 命中数值，换算为命中率 |
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

## 伤害公式

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

示例（等级 10 时 CritScale = 300）：

```text
CriticalRating = 30  -> 暴击率约 9%
CriticalRating = 75  -> 暴击率约 20%
CriticalRating = 150 -> 暴击率约 33%
CriticalRating = 300 -> 暴击率约 50%
```

### 命中和闪避

横版动作游戏里，玩家通过走位和技能范围决定是否命中。命中/闪避主要用于自动攻击、召唤物、怪物攻击。

```text
FinalHitChance = Clamp(BaseHitChance + AttackerHitRating / HitScale - DefenderEvasionRating / EvasionScale, 0.05, 0.98)
```

初始参数：

```text
BaseHitChance = 0.90
HitScale = 200 + AttackerLevel * 10
EvasionScale = 200 + DefenderLevel * 10
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

### 吸血

```text
Heal = FinalDamage * LifeStealRate
```

只有带 `Damage.CanLifeSteal` 标签的伤害才触发吸血。持续伤害、反伤、环境伤害默认不触发。

### 计算顺序

```text
1. 读取攻击者 AttackPower 和技能基础伤害/倍率
2. 计算基础伤害 = SkillBaseDamage + AttackPower
3. 防御减免 = max(1, 基础伤害 - DefensePower)
4. 命中判定（如果适用）
5. 暴击判定，如果暴击乘以 CritDamageRate
6. 元素伤害单独计算
7. 最终伤害 = 基础伤害 + 元素伤害
8. 吸血计算
```

## 霸体与韧性

原游戏的横版动作战斗中，霸体是重要的战斗机制。

第一阶段做法：

```text
Status.SuperArmor       当前处于霸体状态，用 GameplayTag 表示
Poise / MaxPoise        韧性值，受到攻击时减少，归零后可被打断
```

各单位配置：

```text
普通怪：没有霸体，少量韧性
精英怪：部分技能期间加 Status.SuperArmor
Boss：阶段或技能期间加 Status.SuperArmor
玩家：某些技能释放期间获得短霸体（如战士的战神之躯）
```

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

GAS 实现方案：

```text
每个宠物守护 GE 根据宠物属性动态计算 Modifier magnitude
攻击守护 GE -> AttackPower
防御守护 GE -> DefensePower
生命守护 GE -> MaxHealth
魔法守护 GE -> MaxMana
致命守护 GE -> CriticalRating
```

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

当前代码：

```text
DOHealthSet    Health / MaxHealth / Damage / Healing
DOPlaySet      Mana / MaxMana / Stamina / MaxStamina / AttackPower / DefensePower
```

当前 DOPlaySet 已有 AttackPower / DefensePower，不需要改造。后续属性增多后拆为独立 AttributeSet：

```text
DOHealthSet       Health / MaxHealth / Damage / Healing / HealthRegen
DOResourceSet     Mana / MaxMana / Stamina / MaxStamina / ManaRegen
DOCombatSet       AttackPower / DefensePower
                  CriticalRating / CritDamageRate / HitRating / EvasionRating
                  AttackSpeed / MoveSpeed / LifeStealRate
DOElementSet      FireAttack / LightningAttack / IceAttack / LightAttack / DarkAttack
                  ElementResistance
DOPrimarySet      Strength / Agility / Intelligence（仅玩家）
```

### GameplayEffect 分层

```text
职业基础 GE      -> 一级属性（力量/敏捷/智力）
等级成长 GE      -> 每级提升一级属性
觉醒 GE          -> 觉醒后额外属性
装备 GE          -> 攻击/防御等二级属性
强化 GE          -> 装备属性百分比提升
宠物守护 GE      -> 攻击/防御/生命/魔法/致命
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
计算：命中 -> 防御减免 -> 暴击 -> 元素伤害 -> 吸血
输出：Damage Meta Attribute
```

### 网络复制

所有战斗属性使用 `ReplicatedUsing=OnRep_XXX` 同步。

一级属性（力量/敏捷/智力）仅服务端修改，客户端通过 OnRep 刷新 UI。

伤害 Meta Attribute（Damage / Healing）不同步，只在服务端 PostGameplayEffectExecute 中处理。

## 实施阶段

### 第一阶段：核心战斗闭环

当前 DOPlaySet 已有 AttackPower / DefensePower，不需要改造：

```text
Health / MaxHealth / Damage / Healing
Mana / MaxMana
AttackPower / DefensePower
```

完成基础伤害公式 ExecutionCalculation。

### 第二阶段：进阶属性

新增 DOCombatSet：

```text
CriticalRating / CritDamageRate
HitRating / EvasionRating
AttackSpeed / MoveSpeed
LifeStealRate
HealthRegen / ManaRegen
```

### 第三阶段：一级属性和成长

新增 DOPrimarySet（仅玩家）：

```text
Strength / Agility / Intelligence
```

实现一级属性到二级属性的转换 GE。

实现职业基础属性和等级成长。

### 第四阶段：元素系统

新增 DOElementSet：

```text
FireAttack / LightningAttack / IceAttack / LightAttack / DarkAttack
ElementResistance
```

### 第五阶段：成长系统

```text
装备系统 GE（装备属性 + 强化等级）
宠物守护神 GE（五项守护属性）
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
致命从简单百分比改为数值型 Rating 系统
补全宠物守护神作为核心成长系统
装备强化 +1 到 +15 作为主要属性来源
伤害公式回归简单减法，加保底机制
一级属性为力量/敏捷/智力三项，体力改为独立动作资源
新增攻击速度属性
```
