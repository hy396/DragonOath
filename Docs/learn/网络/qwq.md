
这是 **UE 引擎 C++ 网络同步** 的核心代码，我用通俗的方式逐层拆解，帮你彻底看懂。

---

### 一、先看懂外层函数：`GetLifetimeReplicatedProps`

这是 `AActor` 自带的虚函数，**所有需要网络同步的 Actor 都必须重写它**。

- 作用：向引擎注册「我这个 Actor 里，哪些成员变量需要从服务器同步到客户端」。
- 第一行 `Super::GetLifetimeReplicatedProps(...)` 是调用父类实现，把父类需要同步的属性也一并加上，不能省略。
- 参数 `OutLifetimeProps` 是输出数组，你往里面添加的属性，引擎就会自动在网络间同步。

---

### 二、第一段代码：`DOREPLIFETIME_CONDITION` 宏

这是「带条件限制的属性复制」，是基础同步宏的加强版。

#### 1. 基础版：`DOREPLIFETIME`

最原始的写法是：

```cpp
DOREPLIFETIME(ADerivedActor, Health);
```

含义：把 `ADerivedActor` 类的 `Health`（血量）属性，从服务器**同步给所有能看到这个 Actor 的客户端**，没有任何限制。

#### 2. 加了 `_CONDITION` 之后

```cpp
DOREPLIFETIME_CONDITION(ADerivedActor, Health, COND_OwnerOnly);
```

多了第三个参数——**复制条件**，用来控制「这个属性发给哪些客户端」，用来节省网络流量。
例子里的 `COND_OwnerOnly` 意思是：

> `Health` 这个属性，**只同步给这个 Actor 的拥有者客户端**，其他玩家的客户端收不到这个值。

比如玩家自己的血量、背包道具，只需要玩家本人知道，不需要发给全房间玩家，就用这个条件。

#### 常见的 `COND_` 开头条件

- `COND_None`：无条件，所有客户端都同步（默认行为）
- `COND_OwnerOnly`：仅同步给 Actor 所有者
- `COND_SkipOwner`：跳过所有者，只同步给其他客户端
- `COND_InitialOnly`：只在 Actor 首次创建时同步一次，之后不再更新

---

### 三、第二段代码：`DOREPLIFETIME_CONDITION_NOTIFY` 宏

比上面的宏多了一个核心能力：**属性同步到客户端后，自动触发一个回调函数**（也就是 UE 里的 RepNotify 机制）。

#### 1. 什么是 RepNotify

给属性开启 RepNotify 后，你需要在类里写一个对应函数 `OnRep_Health()`。当客户端收到服务器发来的新血量时，引擎会**自动调用这个函数**，你可以在里面写更新血条UI、播放受击特效等逻辑。

#### 2. 两种通知模式的区别

宏的第三个参数控制「什么时候触发 OnRep 回调」：

- `REPNOTIFY_Always`
  每次服务器同步这个属性，**哪怕数值完全没变**，客户端也会触发 `OnRep_Health`。
  适用于每次同步都要执行逻辑的场景，比如强制刷新UI、重置状态。
- `REPNOTIFY_OnChanged`
  **只有属性的值发生变化时**，客户端才会触发 `OnRep_Health`。
  这是最常用的模式，比如血量从100掉到80才播放掉血特效，数值不变就不执行，节省性能。

> 补充：这个宏名字里带 `_CONDITION`，说明它也支持第四个参数写复制条件（比如 `COND_OwnerOnly`），图里的例子省略了，默认就是 `COND_None`。

---

### 四、一句话总结三者区别

| 宏                                 | 核心能力              | 典型使用场景                             |
| :--------------------------------- | :-------------------- | :--------------------------------------- |
| `DOREPLIFETIME`                  | 基础全量同步          | 角色位置、模型状态等所有人都要看到的属性 |
| `DOREPLIFETIME_CONDITION`        | 按条件控制同步范围    | 玩家血量、背包等私有属性，只发给自己     |
| `DOREPLIFETIME_CONDITION_NOTIFY` | 条件同步 + 客户端回调 | 同步后需要执行逻辑，比如更新UI、播放特效 |

日常开发里，角色血量这种属性，一般就用 `DOREPLIFETIME_CONDITION_NOTIFY` + `COND_OwnerOnly` + `REPNOTIFY_OnChanged` 的组合。
