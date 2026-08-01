# 03 UnLua：它在 UE 中做什么

UnLua 将 UE 反射系统暴露给 Lua。你可以在 Lua 中访问标记为可访问的 `UCLASS`、`UPROPERTY`、`UFUNCTION`、`USTRUCT` 和 `UENUM`，并让 Lua 模块覆写某个蓝图类的事件实现。

## 1. 运行关系

```text
C++：定义稳定能力、网络权威、GAS、可暴露接口
  -> 蓝图：组合组件、资产引用、指定 Lua 模块、可视化配置
    -> Lua：本地表现、界面流程、剧情/活动/交互编排
```

Lua 不是独立的 Actor 系统。Lua 模块附着在一个 UE 对象或蓝图实例的生命周期上；UE 仍然创建 Actor、调度事件和管理网络复制。

## 2. DragonOath 的职责边界

| 可用于 Lua POC | 不能交给 Lua 决定 |
| --- | --- |
| 本地交互提示、NPC 台词分支、UI 展示、镜头提示、低风险活动编排 | 伤害数值、命中确认、死亡、掉落、经验、存档写入、反作弊、服务器权威状态 |
| 读取已复制的结果并显示 | 修改 GAS 核心激活/结算规则 |

原因很直接：Lua 客户端脚本不可信，而且 UnLua 不会替你解决 GAS 的预测、复制和服务端裁决。

## 3. 当前项目的脚本位置

项目已经把下面路径配置为打包的 UFS 文件：

```text
Plugins/UnLua/Content/Script/
```

本教程使用：

```text
Plugins/UnLua/Content/Script/Tutorial/
  HelloUnLua.lua
```

模块路径就是去掉根路径和扩展名后的点号路径：`Tutorial.HelloUnLua`。

## 4. 编辑器前置检查

在开始第一个练习前，打开 `DragonOath.uproject`，确认插件管理器中 **UnLua** 已启用。若修改了 `.uplugin`、项目模块依赖或 C++ 头文件，按项目约定重新生成工程文件并编译；仅修改 `.lua` 时通常不需要 C++ 编译。

在内容浏览器创建一个临时蓝图：

```text
/Game/UnLuaTutorial/BP_UnLuaHello
```

父类选 `Actor`。这个资产只用于学习，完成后可保留在教程目录，不能挂到角色、PlayerState 或 ASC 上。

## 5. 对 C++ 程序员最容易误解的地方

- `self` 是当前绑定的 UE 对象实例，不是 C++ 静态成员也不是 `this` 的简单文本替代。
- 能否从 Lua 调用由 UE 反射和访问规则决定，不是任意 C++ 函数都天然可调。
- 蓝图/C++ 的父实现、Lua 覆写和生命周期事件会共同参与调用链。先只覆写一个事件，确认顺序后再扩展。
- Lua 中保存 UE 对象引用时，仍须理解 UE 生命周期。对象销毁后不要继续假定引用有效。

下一章会完成一个只写日志的 Actor 覆写，它没有游戏状态副作用，因此最适合作为第一个验证。
