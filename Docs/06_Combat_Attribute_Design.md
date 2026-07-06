# 06. Combat Attribute Design

状态：设计草案，先评审，不直接实施代码。

参考资料：

```text
E:\浏览器下载\龙斗士游戏属性大全.xlsx
```

当前 Excel 中的属性包括：

```text
力量、敏捷、智慧
攻击、防御、生命、魔法、致命
命中、闪避、移动速度
吸血、霸体
炎火攻击、雷电攻击、冰冻攻击、光明攻击、黑暗攻击
元素抗性
回血、回魔
```

## 设计目标

这套属性系统要同时服务玩家、普通怪、精英怪、Boss、装备、技能和 Buff。

核心目标：

- 玩家和怪物都能使用同一套战斗公式。
- 玩家可以有一级属性、装备成长、技能加成。
- 怪物不必模拟完整玩家成长，可以直接配置最终战斗属性。
- 防御、抗性、命中、闪避不能让战斗变成“打不动”或“总是空刀”。
- GAS 中的 AttributeSet 只保存运行时数值，不把复杂战斗公式塞进 AttributeSet。

最重要的原则：

```text
血量决定主要击杀时间。
攻击决定主要威胁强度。
防御和抗性只做手感微调，不作为主要堆数值手段。
```

## 属性分层

### 一级属性

一级属性主要给玩家、装备、成长系统使用。

```text
Strength   力量
Agility    敏捷
Wisdom     智慧
```

推荐用途：

```text
力量 -> 攻击、少量生命
敏捷 -> 命中、闪避、攻击速度或移动相关属性
智慧 -> 魔法、回魔、部分元素伤害
```

普通怪不建议强行配置力量、敏捷、智慧。怪物可以直接配置最终属性，例如攻击、防御、生命、命中、闪避。

原因：

```text
玩家需要成长感，所以一级属性有价值。
怪物需要好调表，所以直接配置最终属性更省心。
```

### 核心战斗属性

这些属性玩家和怪物都可以拥有。

```text
MaxHealth      最大生命
MaxMana        最大魔法
AttackPower    攻击力
DefensePower   防御力
```

当前代码里：

```text
DOHealthSet    负责 Health / MaxHealth / Damage / Healing
DOPlaySet      已有 Mana / MaxMana / Stamina / MaxStamina / AttackPower / DefensePower
```

第一阶段可以继续沿用当前结构。等属性变多以后，再考虑拆出：

```text
DOResourceSet  Mana / Stamina / Regen
DOCombatSet    Attack / Defense / Crit / Hit / Evasion / MoveSpeed
DOElementSet   ElementAttack / ElementResistance
```

### 进阶战斗属性

这些属性建议做成“评分”或“百分比”，不要混着用。

推荐第一阶段先用百分比，便于调试：

```text
CritChance      暴击率，0.15 表示 15%
CritDamageRate  暴击伤害倍率，默认 1.5
HitChance       命中率，0.90 表示 90%
EvasionChance   闪避率，0.05 表示 5%
MoveSpeed       移动速度
```

后期如果装备数值膨胀，再改成 Rating：

```text
CritRating -> 公式换算 CritChance
HitRating -> 公式换算 HitChance
EvasionRating -> 公式换算 EvasionChance
```

### 战斗特效属性

```text
LifeStealRate   吸血比例
SuperArmor      霸体相关属性
HealthRegen     回血
ManaRegen       回魔
```

吸血推荐按最终伤害计算：

```text
Heal = FinalDamage * LifeStealRate
```

注意：

```text
只有带 Damage.CanLifeSteal 标签的伤害才触发吸血。
持续伤害、反伤、环境伤害默认不触发吸血。
```

霸体不建议一开始做成纯随机概率。横版动作游戏里，随机“不被打断”会让玩家感觉不公平。

推荐拆成两层：

```text
Status.SuperArmor       当前处于霸体状态，用 GameplayTag 表示
Poise / MaxPoise        韧性值，受到攻击时减少，归零后可被打断
```

第一阶段简单做法：

```text
普通怪：没有霸体，少量韧性
精英怪：部分技能期间加 Status.SuperArmor
Boss：阶段或技能期间加 Status.SuperArmor
玩家：某些技能释放期间获得短霸体
```

## 防御设计

你纠结“小怪也有防御怎么办”，核心答案是：

```text
小怪当然可以有防御，但防御不要用线性减法。
```

不推荐：

```text
FinalDamage = Attack - Defense
```

问题：

```text
低攻击打高防御会变成 0 伤害。
数值稍微调高，手感就突然崩。
小怪一旦有防御，新手玩家会觉得打不动。
```

推荐：

```text
DamageReduction = DefensePower / (DefensePower + DefenseScale)
FinalPhysicalDamage = RawPhysicalDamage * (1 - DamageReduction)
```

其中：

```text
DefenseScale = 100 + DefenderLevel * 10
```

这是第一阶段可用的简单曲线。它的好处是防御收益递减，不会轻易把伤害压到 0。

示例：

```text
等级 10 时 DefenseScale = 200

DefensePower = 22  -> 减伤约 10%
DefensePower = 50  -> 减伤约 20%
DefensePower = 86  -> 减伤约 30%
DefensePower = 200 -> 减伤约 50%
```

反推公式：

```text
DefensePower = DefenseScale * TargetReduction / (1 - TargetReduction)
```

这意味着策划表里可以先填“目标减伤比例”，再自动算 DefensePower。

## 怪物防御分配

怪物的生命决定它能活多久，防御只决定它“硬不硬”的手感。

建议范围：

| 类型 | 目标减伤 | 设计目的 |
| --- | ---: | --- |
| 普通小怪 | 5% - 12% | 让玩家打起来爽，不要拖时间 |
| 厚血小怪 | 10% - 18% | 感觉更耐打，但主要靠血量 |
| 盾兵/重甲怪 | 18% - 28% | 明确表现“硬”，但要有破盾或背击解法 |
| 精英怪 | 15% - 25% | 比普通怪稳，不应过度刮痧 |
| Boss | 20% - 35% | 稳定耐打，主要靠血量和阶段机制 |
| 特殊免伤阶段 | 40% - 60% | 只用于短时间机制，不做常驻 |

第一阶段建议：

```text
普通怪 DefensePower 不要超过同级 15% 减伤。
Boss 常驻减伤不要超过 35%。
超过 35% 的减伤要通过 Buff、护盾、阶段机制表现出来。
```

这样做的结果：

```text
小怪可以有防御，但不会让玩家觉得打不动。
Boss 可以有防御，但主要挑战来自机制、血量和攻击节奏。
```

## 元素攻击和元素抗性

Excel 中的元素攻击：

```text
炎火攻击
雷电攻击
冰冻攻击
光明攻击
黑暗攻击
```

Excel 说明是“无视敌人普通防御力”。这里建议理解为：

```text
元素伤害不吃 DefensePower。
元素伤害吃 ElementResistance。
```

第一阶段简单设计：

```text
FireAttack
LightningAttack
IceAttack
LightAttack
DarkAttack
ElementResistance
```

统一元素抗性公式：

```text
ElementReduction = ElementResistance / (ElementResistance + ResistanceScale)
FinalElementDamage = RawElementDamage * (1 - ElementReduction)
```

后期如果要做怪物弱点，再拆成：

```text
FireResistance
LightningResistance
IceResistance
LightResistance
DarkResistance
```

弱点可以用负抗性表达，但要限制范围：

```text
最低 -50% 承伤提升
最高 75% 减伤
```

## 命中和闪避

横版动作 RPG 里，玩家已经通过走位、跳跃、技能范围来决定是否命中。命中/闪避如果做得太随机，会削弱操作反馈。

第一阶段建议保守使用：

```text
玩家主动技能默认不随机 Miss。
命中/闪避主要用于自动追踪、召唤物、怪物普通攻击、远程弹道修正。
```

简单公式：

```text
FinalHitChance = Clamp(BaseHitChance + AttackerHit - DefenderEvasion, MinHitChance, MaxHitChance)
```

初始参数：

```text
BaseHitChance = 0.90
MinHitChance  = 0.20
MaxHitChance  = 0.98
```

普通小怪：

```text
HitChance 0.80 - 0.90
EvasionChance 0.00 - 0.05
```

高敏捷怪：

```text
EvasionChance 0.08 - 0.15
```

Boss：

```text
默认不高闪避，避免玩家技能经常落空。
Boss 的难度主要来自机制，不来自随机闪避。
```

## 暴击设计

第一阶段用简单百分比：

```text
CritChance = 0.05 - 0.30
CritDamageRate = 1.5
```

推荐规则：

```text
普通怪低暴击或无暴击。
精英怪可以有 5% - 10% 暴击。
Boss 不建议高随机暴击，除非技能有明显前摇和提示。
玩家暴击成长主要来自装备、敏捷、Buff。
```

暴击计算顺序：

```text
RawDamage
-> 防御或抗性
-> 暴击倍率
-> 最终伤害
-> 吸血和伤害数字
```

如果希望暴击更爽，也可以先暴击再防御；但第一阶段推荐防御后暴击，数值更稳定。

## 玩家属性来源

玩家最终属性来自多层叠加：

```text
职业基础属性
+ 等级成长
+ 装备属性
+ 宝石 / 守护者 / 称号
+ 技能被动
+ 临时 Buff / Debuff
= 最终 AttributeSet 数值
```

玩家一级属性转换建议：

```text
1 力量  -> 1.5 AttackPower + 2 MaxHealth
1 敏捷  -> 0.2% HitChance + 0.1% EvasionChance
1 智慧  -> 3 MaxMana + 0.5 ManaRegen + 少量元素攻击
```

这些转换比例只是起步值，后面要根据实际手感改。

## 怪物属性来源

怪物推荐直接配置最终属性，不走一级属性转换。

怪物数据表建议字段：

```text
MonsterId
DisplayName
Level
Rank                Normal / Heavy / Elite / Boss
MaxHealth
AttackPower
DefenseReduction    目标物理减伤，策划可读字段
DefensePower        可由 DefenseReduction 自动计算
HitChance
EvasionChance
CritChance
MoveSpeed
ElementResistance
HealthRegen
Tags
```

普通怪示例：

```text
Level = 10
Rank = Normal
TargetTimeToKill = 4 秒
DefenseReduction = 0.10
CritChance = 0
EvasionChance = 0.03
```

精英怪示例：

```text
Level = 10
Rank = Elite
TargetTimeToKill = 20 秒
DefenseReduction = 0.20
CritChance = 0.05
EvasionChance = 0.05
```

Boss 示例：

```text
Level = 10
Rank = Boss
TargetTimeToKill = 120 秒
DefenseReduction = 0.30
CritChance = 0
EvasionChance = 0
Status.SuperArmor 在指定技能和阶段期间开启
```

## TTK 平衡方法

TTK 是 Time To Kill，也就是击杀时间。

先定同级玩家的基准输出：

```text
PlayerDPS = 玩家同级平均每秒伤害
```

再定怪物目标生存时间：

```text
NormalMonsterTTK = 3 - 6 秒
EliteMonsterTTK  = 15 - 30 秒
BossTTK          = 90 - 180 秒
```

怪物血量估算：

```text
MonsterHealth = PlayerDPS * TargetTTK * ExpectedDamageMultiplier
```

如果怪物有 20% 减伤：

```text
ExpectedDamageMultiplier = 0.8
```

反过来：

```text
MonsterHealth = PlayerDPS * TargetTTK * 0.8
```

这个思路能避免一开始乱填生命和防御。

玩家承伤也类似：

```text
PlayerSurvivalTime = PlayerHealth / MonsterDPSAfterDefense
```

推荐目标：

```text
普通小怪单只：玩家能扛 20 秒以上
普通小怪群体：玩家需要走位，但不会瞬间暴毙
精英怪：玩家吃完整连招会危险
Boss：大招必须躲，小技能可以承受几次
```

## GAS 落地建议

AttributeSet 只保存最终运行时数值：

```text
Health / MaxHealth
Mana / MaxMana
Stamina / MaxStamina
AttackPower / DefensePower
CritChance / CritDamageRate
HitChance / EvasionChance
MoveSpeed
LifeStealRate
ElementAttack / ElementResistance
HealthRegen / ManaRegen
```

GameplayEffect 负责修改属性：

```text
装备 GE：长期加属性
Buff GE：临时加属性
技能消耗 GE：扣 Mana / Stamina
伤害 GE：写入 Damage Meta Attribute
治疗 GE：写入 Healing Meta Attribute
```

ExecutionCalculation 负责战斗公式：

```text
读取攻击者 AttackPower / ElementAttack / CritChance
读取目标 DefensePower / ElementResistance / EvasionChance
计算命中、暴击、防御、抗性
输出 Damage Meta Attribute
```

AttributeSet 负责最后应用：

```text
PostGameplayEffectExecute 中把 Damage 转成 Health 减少
把 Healing 转成 Health 增加
Clamp 到合法范围
广播本地 UI / 战斗消息
```

## 第一阶段实施顺序

建议不要一次把所有属性都写进代码。

第一阶段只做核心闭环：

```text
Health / MaxHealth
Mana / MaxMana
Stamina / MaxStamina
AttackPower / DefensePower
Damage / Healing
```

第二阶段加入：

```text
CritChance / CritDamageRate
MoveSpeed
HealthRegen / ManaRegen
```

第三阶段加入：

```text
HitChance / EvasionChance
LifeStealRate
SuperArmor / Poise
```

第四阶段加入：

```text
五系元素攻击
元素抗性
怪物弱点
守护者系统属性
```

## 当前推荐结论

现在不要纠结“小怪到底该不该有防御”。

推荐结论：

```text
所有可受伤单位都可以有 DefensePower。
普通怪 DefensePower 很低，只提供 5% - 12% 减伤。
精英和 Boss 防御稍高，但主要靠血量和机制撑时长。
元素伤害无视 DefensePower，但受到 ElementResistance 影响。
复杂公式放 ExecutionCalculation，AttributeSet 只管保存和 Clamp。
```

这套方案适合先做出可玩的战斗，再逐步加复杂成长系统。
