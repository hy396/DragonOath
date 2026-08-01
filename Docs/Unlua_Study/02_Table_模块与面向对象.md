# 02 Table、模块与 Lua 的“对象”

Lua 最重要的概念是 `table`。它同时承担 C++ 中 `TArray`、`TMap`、普通对象和命名空间的角色。

## 1. Table 不是 C++ struct

```lua
local skill = {
    name = "Dodge",
    stamina_cost = 20,
    cooldown = 1.0,
}

print(skill.name)
print(skill["stamina_cost"])

skill.cooldown = 0.8
skill.input_tag = "InputTag.Ability.Dodge"
```

键没有预先声明；拼错字段名不会报编译错误，而是创建一个新字段。因此游戏 Lua 建议：字段名使用 `snake_case`，核心数据建立统一构造函数或配置校验。

## 2. 数组与遍历

```lua
local rewards = { "Gold", "Potion", "Crystal" }

print(rewards[1]) -- Gold；Lua 数组从 1 开始。

for index, reward in ipairs(rewards) do
    print(index, reward)
end

local attributes = { attack = 100, defense = 50 }
for key, value in pairs(attributes) do
    print(key, value)
end
```

`ipairs` 适合从 `1` 开始连续的数组；`pairs` 适合字典，遍历顺序不保证稳定。**不要用 `pairs` 的顺序驱动确定性玩法结果。**

## 3. 赋值传的是引用

```lua
local first = { hp = 100 }
local second = first
second.hp = 20

print(first.hp) -- 20：两者指向同一个 table。
```

这与 `TSharedPtr` 的共享对象更接近，而不是 C++ 值类型 `FMyStruct` 的复制。需要复制时，应写明确的复制函数；不要假设 `copy = source` 会克隆数据。

## 4. 方法、冒号与 self

```lua
local character = { name = "Lina", hp = 100 }

function character:take_damage(amount)
    self.hp = self.hp - amount
    return self.hp
end

character:take_damage(15)
print(character.hp) -- 85
```

`object:method(x)` 等价于 `object.method(object, x)`。冒号会自动把对象作为第一个参数 `self` 传入；定义和调用要保持一致。

## 5. 模块与 require

假设文件为 `Plugins/UnLua/Content/Script/Tutorial/CombatMath.lua`：

```lua
local CombatMath = {}

function CombatMath.clamp(value, minimum, maximum)
    if value < minimum then
        return minimum
    end
    if value > maximum then
        return maximum
    end
    return value
end

return CombatMath
```

另一份脚本中加载它：

```lua
local CombatMath = require("Tutorial.CombatMath")
local hp = CombatMath.clamp(120, 0, 100)
print(hp)
```

`require` 路径相对脚本根目录，使用点号，不写 `.lua`。一个模块必须 `return` 对外暴露的 table。模块加载后会被缓存，因此修改文件后是否重新加载要看 UnLua 的热重载策略，不能假定再次 `require` 就会读取新文件。

## 6. 练习：写一个纯 Lua 技能数据模块

创建 `Tutorial/DodgeConfig.lua`，导出：

```lua
{
    stamina_cost = 20,
    cooldown = 1.0,
    invincible_duration = 0.25,
}
```

再为它添加 `get_cooldown_after_haste(haste_percent)`，例如 `20` 代表 20% 冷却缩减。返回值最低限制为 `0.1` 秒。

参考答案：

```lua
local DodgeConfig = {
    stamina_cost = 20,
    cooldown = 1.0,
    invincible_duration = 0.25,
}

function DodgeConfig.get_cooldown_after_haste(haste_percent)
    local scale = 1.0 - haste_percent / 100.0
    local cooldown = DodgeConfig.cooldown * scale
    if cooldown < 0.1 then
        cooldown = 0.1
    end
    return cooldown
end

return DodgeConfig
```

这段只是学习数据计算。它不能直接决定联机中的闪避冷却；正式冷却仍应由 GAS 的 GameplayEffect 和服务器流程负责。
