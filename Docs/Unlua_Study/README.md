# UnLua 学习路线

这套材料面向有 C++ 基础、但从未接触 Lua 的 DragonOath 开发者。目标不是把所有 Lua 特性背下来，而是能在 UE 5.8 + UnLua 中安全地编写和调试**非权威的内容编排逻辑**。

> 项目已包含 `Plugins/UnLua`，并在 `Config/DefaultGame.ini` 中配置了 `Plugins/UnLua/Content/Script` 的打包路径。暂时不要改项目代码或把 UnLua 用进战斗、伤害、掉落、经验、存档及任何服务器权威流程。

## 学完后能做什么

1. 读懂并独立写出 Lua 的变量、控制流、函数、table 和模块。
2. 理解 Lua 的 table 与 C++ 的对象/容器的差别，避开 `nil`、1 起始下标、引用共享等常见坑。
3. 在蓝图类上绑定 UnLua 模块，覆写 `ReceiveBeginPlay`、`ReceiveTick` 等 UE 事件。
4. 从 Lua 调用可访问的 `UFUNCTION` / `UPROPERTY`，使用日志定位脚本问题。
5. 为 DragonOath 完成一个低风险 POC：本地交互提示或剧情触发编排。

## 推荐顺序

| 阶段 | 文件 | 预计时间 | 完成标准 |
| --- | --- | --- | --- |
| 1 | [01_Lua_基础.md](01_Lua_基础.md) | 60-90 分钟 | 能自己写函数和循环 |
| 2 | [02_Table_模块与面向对象.md](02_Table_模块与面向对象.md) | 60-90 分钟 | 能写一个 `require` 模块 |
| 3 | [03_UnLua_运行模型与准备.md](03_UnLua_运行模型与准备.md) | 30-45 分钟 | 明白 UE/蓝图/Lua 的职责边界 |
| 4 | [04_第一个蓝图覆写练习.md](04_第一个蓝图覆写练习.md) | 45-60 分钟 | 在屏幕或日志看到 Lua 事件执行 |
| 5 | [05_调试热重载与项目规范.md](05_调试热重载与项目规范.md) | 30-45 分钟 | 能定位一次 Lua 脚本错误 |
| 6 | [06_DragonOath_POC_作业.md](06_DragonOath_POC_作业.md) | 1-2 小时 | 做完本地提示 POC |

每一章都按“先读 -> 手写 -> 运行 -> 对照答案”的顺序进行。不要只复制代码；Lua 的语法很小，亲自敲一遍比看十遍有效。

## 先记住的三件事

- Lua 的核心容器只有 `table`：数组、字典、对象和模块通常都是它。
- Lua 下标默认从 `1` 开始，访问不存在的键得到 `nil`，不是 C++ 的异常或默认构造值。
- UnLua 是 C++/蓝图与 Lua 之间的运行时桥梁，不会改变 UE 的网络模型。服务器权威仍必须由 C++ 或 GAS 执行。

## 学习时的目录约定

后续真正的游戏 Lua 统一放在 `Plugins/UnLua/Content/Script/`；本目录仅保存讲义、练习说明和学习笔记。建议先用 `Tutorial/` 子目录做实验，成熟后才迁移到正式功能目录。

## 参考资料

- 项目内插件源码：`Plugins/UnLua/`
- 项目已有技术选型说明：`Docs/05_Scripting_Plugin_Comparison.md`
- UnLua 官方仓库：https://github.com/Tencent/UnLua

插件的 API 会随版本略有差异。实际编写前，以项目当前 `Plugins/UnLua` 的示例和文档为准；本文示例采用 UnLua 常见的 `require`、`UnLua.Class()` 和蓝图覆写模式。
