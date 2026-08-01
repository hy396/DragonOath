# 01 Lua 基础：从 C++ 迁移心智模型

Lua 是动态类型、垃圾回收的脚本语言。你不写头文件、不声明类型，也不需要手动释放普通对象。代价是许多错误会从“编译期”延后到“运行期”。

## 1. 最小程序

```lua
-- 两个连字符开始单行注释。
local player_name = "Lina"
local level = 1
local is_alive = true

print(player_name, level, is_alive)
```

`local` 很重要：它类似 C++ 函数或文件作用域内的局部变量。没有 `local` 的赋值会写入全局环境，容易与其他模块互相污染。

| C++ | Lua | 说明 |
| --- | --- | --- |
| `int32 Level = 1;` | `local level = 1` | 类型由值决定 |
| `bool bAlive = true;` | `local is_alive = true` | Lua 只有 `false` 和 `nil` 为假 |
| `FString Name = TEXT("Lina");` | `local name = "Lina"` | 字符串可用单/双引号 |
| `nullptr` | `nil` | 表示不存在，也可删除 table 键 |
| `// comment` | `-- comment` | 单行注释 |

### 真值规则

```lua
if 0 then
    print("会执行") -- 0 在 Lua 中是真；这点与 C++ 不同。
end

if "" then
    print("也会执行") -- 空字符串也是真。
end
```

只有 `false` 与 `nil` 为假。因此不要把数字 `0` 当成“无效”。

## 2. 条件与循环

```lua
local hp = 35

if hp <= 0 then
    print("死亡")
elseif hp < 30 then
    print("危险")
else
    print("安全")
end

for index = 1, 3 do
    print(index)
end

local count = 0
while count < 3 do
    count = count + 1
end
```

关键差别：Lua 用 `then`、`do`、`end` 表示代码块，没有花括号；范围 `1, 3` 包含两端。

## 3. 函数、多返回值与可选参数

```lua
local function calculate_damage(attack, multiplier)
    multiplier = multiplier or 1.0
    return attack * multiplier
end

local damage = calculate_damage(100, 1.5)
print(damage) -- 150

local function get_hp_range()
    return 80, 100
end

local current_hp, max_hp = get_hp_range()
print(current_hp, max_hp)
```

`a or b` 常用来提供默认值，但只适合“`false` 也应视为缺省”的参数。若 `false` 是有效值，要显式判断 `nil`：

```lua
local function set_visible(value)
    if value == nil then
        value = true
    end
    return value
end
```

## 4. 第一组练习

新建任意临时 `.lua` 文件，先不要看答案，完成：

1. 声明 `character_name`、`gold`、`is_in_combat` 三个局部变量并打印。
2. 写 `is_dead(current_hp)`：`current_hp <= 0` 时返回 `true`，否则返回 `false`。
3. 写 `heal(current_hp, max_hp, amount)`：返回治疗后的生命值，结果不得超过 `max_hp`。
4. 用 `for` 打印 1 到 5 中的偶数。

答案：

```lua
local function is_dead(current_hp)
    return current_hp <= 0
end

local function heal(current_hp, max_hp, amount)
    local new_hp = current_hp + amount
    if new_hp > max_hp then
        new_hp = max_hp
    end
    return new_hp
end

for number = 1, 5 do
    if number % 2 == 0 then
        print(number)
    end
end
```

完成本章后，继续 [02_Table_模块与面向对象.md](02_Table_模块与面向对象.md)。
