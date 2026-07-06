# GameplayMessageRouter · 使用文档

一个基于 **GameplayTag** 的**广播-订阅消息总线**。来源于 Epic 的 Lyra 示例工程，已集成到本项目。

让"发送方"和"监听方"不互相 `#include`、不直接持有对方指针，就能完成跨系统通信。

---

## 目录

1. [为什么用消息路由](#为什么用消息路由)
2. [核心概念](#核心概念)
3. [C++ 用法](#c-用法)
4. [蓝图用法](#蓝图用法)
5. [匹配规则（精确 vs 部分）](#匹配规则精确-vs-部分)
6. [命名约定](#命名约定)
7. [生命周期与清理](#生命周期与清理)
8. [日志与调试](#日志与调试)
9. [踩坑与注意事项](#踩坑与注意事项)
10. [和项目其他委托系统对比](#和项目其他委托系统对比)

---

## 为什么用消息路由

典型痛点——A 系统要在 B 系统的某事件发生时做点事，直接写就是：

```cpp
// MusicSystem.cpp
#include "QuestSystem.h"   // 依赖上来了
...
QuestSubsystem->OnQuestStarted.AddUObject(this, &UMusicSystem::HandleQuestStarted);
```

问题：

- QuestSystem 也可能想反过来听 MusicSystem 的事件 → 循环依赖
- 换一个"成就系统"想加入，每个都要互相 include
- 模块拆分时头疼

**消息路由的思路**：

```
发送方 ──[Broadcast]──> [消息路由 Subsystem] ──[Notify]──> 监听方
           ↑                                            ↑
           │                                            │
         只认 Tag + Payload 类型，不认对方              同上
```

两边都只依赖"Tag + 消息 USTRUCT"——**头文件解耦完成**。

---

## 核心概念

| 概念 | 说明 |
|------|------|
| **频道（Channel）** | 一个 `FGameplayTag`，例如 `UI.Inventory.ItemAdded` |
| **消息结构（Payload）** | 一个 `USTRUCT`，发送方和监听方必须约定一致 |
| **监听句柄（Handle）** | `FGameplayMessageListenerHandle`，注册时返回，销毁时用来反注册 |
| **匹配规则** | `ExactMatch`（只收完全相同频道）/ `PartialMatch`（也收子频道） |

---

## C++ 用法

### 定义消息结构

```cpp
// MyMessages.h
USTRUCT(BlueprintType)
struct FMyItemAddedMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName ItemID = NAME_None;

    UPROPERTY(BlueprintReadOnly)
    int32 Count = 0;
};
```

### 定义频道 Tag

在 `NativeGameplayTags.h` 同级的 Tag 定义文件里：

```cpp
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Inventory_ItemAdded);
```

```cpp
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Inventory_ItemAdded, "UI.Inventory.ItemAdded");
```

### 发送（广播）

```cpp
#include "GameFramework/GameplayMessageSubsystem.h"
#include "MyMessages.h"

void UMyInventory::AddItem(FName ItemID)
{
    // ... 加道具的业务 ...

    FMyItemAddedMessage Msg;
    Msg.ItemID = ItemID;
    Msg.Count = 1;

    UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
    Router.BroadcastMessage(TAG_UI_Inventory_ItemAdded, Msg);
}
```

### 接收（监听）

**方式一：Lambda**

```cpp
void UMyHUD::BeginPlay()
{
    Super::BeginPlay();

    UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);

    ListenerHandle = Router.RegisterListener<FMyItemAddedMessage>(
        TAG_UI_Inventory_ItemAdded,
        [this](FGameplayTag Channel, const FMyItemAddedMessage& Payload)
        {
            UE_LOG(LogTemp, Log, TEXT("拿到道具 %s × %d"), *Payload.ItemID.ToString(), Payload.Count);
        });
}

void UMyHUD::EndPlay(const EEndPlayReason::Type Reason)
{
    ListenerHandle.Unregister();
    Super::EndPlay(Reason);
}
```

**方式二：成员函数（带弱引用保护）**

```cpp
class UMyHUD : public UUserWidget
{
    GENERATED_BODY()
public:
    void HandleItemAdded(FGameplayTag Channel, const FMyItemAddedMessage& Payload);
private:
    FGameplayMessageListenerHandle ListenerHandle;
};

// cpp
void UMyHUD::NativeConstruct()
{
    UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
    ListenerHandle = Router.RegisterListener(TAG_UI_Inventory_ItemAdded, this, &UMyHUD::HandleItemAdded);
}
```

成员函数版本内部以 `TWeakObjectPtr` 持有 `this`——对象被 GC 后回调直接跳过，**不会崩**。但仍建议 EndPlay / Destruct 时主动反注册。

---

## 蓝图用法

### 广播消息

任何蓝图都能调 **Broadcast Message** 节点：

1. 添加 "Broadcast Message" 节点（类别：Messaging）
2. **Channel** 引脚：填 GameplayTag
3. **Message** 引脚：连一个 Make 对应 USTRUCT 的节点

### 监听消息

使用 **Listen For Gameplay Messages** 节点（异步节点）：

1. 放在 BeginPlay / 需要开始监听的地方
2. **Channel**：填 Tag
3. **Payload Type**：选消息 USTRUCT（从下拉菜单）
4. **Match Type**：默认 Exact
5. 输出引脚：
   - **On Message Received**：消息到达时触发的事件
   - **Actual Channel**：实际频道（PartialMatch 才有意义）
   - **Payload**：自动拆成 USTRUCT 字段，可以直接读

节点会持续监听直到蓝图销毁或手动 Cancel。

---

## 匹配规则（精确 vs 部分）

假设你监听 `UI.Inventory`：

| 广播的频道 | ExactMatch 监听 | PartialMatch 监听 |
|------------|----------------|------------------|
| `UI.Inventory` | ✅ 收到 | ✅ 收到 |
| `UI.Inventory.ItemAdded` | ❌ 不收到 | ✅ 收到 |
| `UI.Inventory.ItemRemoved` | ❌ 不收到 | ✅ 收到 |
| `UI.Settings` | ❌ 不收到 | ❌ 不收到 |

**使用建议**：

- 默认用 **ExactMatch**——意图明确，不会意外收到兄弟频道的消息
- 想写一个"监听所有 UI.Inventory.* 事件的日志器"这种聚合需求，才用 **PartialMatch**

---

## 命名约定

推荐三层结构 `领域.对象.动作`：

```
UI.Inventory.ItemAdded
UI.Inventory.ItemRemoved
UI.Settings.LanguageChanged

Quest.Objective.Completed
Quest.State.Started

Audio.Music.TrackChanged
Audio.SFX.Triggered
```

好处：

- PartialMatch 聚合容易（监听 `UI` 就能抓所有 UI 事件）
- 人类读得懂，自动补全友好
- 和 Tag Config 的层级管理自然匹配

---

## 生命周期与清理

**务必**在对象销毁前反注册监听器，否则消息到达时会调用已死对象的回调：

```cpp
FGameplayMessageListenerHandle Handle;

// 注册
Handle = Router.RegisterListener(...);

// 销毁时
Handle.Unregister();
// 或
Router.UnregisterListener(Handle);
```

两种等价。

**成员函数版本（`RegisterListener(Channel, this, &...)`）**自带弱引用保护——对象被 GC 后不会崩，但**监听器条目不会自动清理**（仍占内存）。严谨写法还是要主动 Unregister。

**Subsystem 自身 Deinitialize** 时会清空所有监听器（`ListenerMap.Reset()`），程序退出时安全。

---

## 日志与调试

打开控制台：

```
GameplayMessageSubsystem.LogMessages 1
```

之后每条 BroadcastMessage 都会打印到 Log（含 Channel + Payload 内容），便于排查"消息发了没""监听者有没有收到"。

关闭：

```
GameplayMessageSubsystem.LogMessages 0
```

---

## 踩坑与注意事项

### 1. 类型必须严格匹配

发送方广播 `FMyMessage`，监听方注册 `FOtherMessage` → 不会收到，日志里会报 **Struct type mismatch**。

注意 `IsChildOf` 语义：允许"发送方是监听方期望类型的子类"，但反向不行。

### 2. 调用顺序不保证

多个监听器注册到同一频道时，**触发顺序不稳定**。不要写"音乐必须比 UI 先收到"这种依赖。如需顺序，拆成两个频道（`X.Prepare` / `X.Done`）。

### 3. Payload 生命周期

蓝图 `GetPayload` 只在 `On Message Received` 触发时的那一帧**暂存有效**。拷贝出来存到变量没问题，但不要拿 Payload 的指针跨帧用（C++ 层面也一样——回调返回后 Payload 可能已失效）。

### 4. 初始化时序

Subsystem 之间的 Initialize 顺序**不保证**。如果你在 `Initialize` 里就想订阅某频道的首次广播，而那条广播也在另一个 Subsystem 的 `Initialize` 里发出——可能错过。

解决方案：

- 把"全局状态"持有在 Subsystem，监听方订阅后主动查一次当前状态
- 或者用 `FWorldDelegates::OnPostWorldInitialization` 等晚一些的时机订阅

### 5. 不要频繁广播

`DYNAMIC_MULTICAST_DELEGATE` 走反射，高频事件（如每帧 Tick 级别）会有性能开销。只广播"有意义的离散事件"，状态同步走 TickComponent 或 AttributeSet 更合适。

---

## 和项目其他委托系统对比

| 通信方式 | 什么时候用 | 耦合度 |
|---------|-----------|-------|
| **GameplayMessage 路由**（本插件） | 跨系统事件，两边都不想互相认识 | 最低 |
| **Subsystem 上的 `DYNAMIC_MULTICAST_DELEGATE`** | 某个 Subsystem 作为"权威来源"对外发事件 | 中等（订阅方要 include Subsystem） |
| **World Delegate / Engine Delegate** | 全局引擎级事件（PIE 开始、关卡切换等） | 低（都通过引擎） |
| **Component 上的 UPROPERTY Delegate** | 父-子 Actor 之间的紧密通信 | 高（直接绑定） |

项目里具体使用场景举例：

- **任务状态变化 → 音乐切换**：这种弱耦合跨系统用本路由最合适
- **UI 按钮 → 对应系统**：直接 UPROPERTY Delegate 更直观
- **存档系统的 OnPreSave / OnPostLoad**（若启用）：Subsystem 级 Delegate

---

## 参考资料

- Lyra Starter Game（Epic 官方示例，本插件来源）
- `Source/GameplayMessageRuntime/Public/GameFramework/GameplayMessageSubsystem.h`——公开 API 注释
- `Source/GameplayMessageRuntime/Public/GameFramework/AsyncAction_ListenForGameplayMessage.h`——蓝图异步节点
