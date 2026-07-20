# Messages 模块说明（本地消息总线）

> 路径：`Source/DragonOath/Messages/`
> 定位：DragonOath 的**本地事件消息总线**基础设施，基于 Epic `GameplayMessageRouter`（`UGameplayMessageSubsystem`）与 Lyra 模式移植而来。
> 关联文档：`Docs/04_Local_Message_Bus.md`

---

## 1. 概述：核心功能与整体作用

`Messages` 文件夹提供一套**解耦的事件发布/订阅（Pub/Sub）机制**，让战斗、UI、成就等系统之间不必互相持有引用，只需通过 `GameplayTag` 频道收发消息。

整体作用可概括为三层：

1. **消息载体（Payload）**
   - `FDOVerbMessage`：通用「事件记录型」消息（谁 对 谁 做了 什么事 + 数值 + 标签）。用于伤害、击杀、助攻、重置等同构战斗事件。
   - `FDONotificationMessage`：UI 专用「一次性提示」消息（带本地化文案 + 目标玩家）。用于成就、拾取、击杀流等短暂 UI 提示。

2. **跨网投递（Replication）**
   - `FDOVerbMessageReplication`：用 `FFastArraySerializer` 把 verb 消息从服务端增量复制到所有客户端，客户端收到后再广播到本地总线，保证远端客户端也能触发 HUD / 处理器。

3. **订阅与加工（Processor / Helpers）**
   - `UGameplayMessageProcessor`：消息处理器基类，封装订阅生命周期（BeginPlay 注册、EndPlay 反注册），用于组合/转换/再广播消息（如连击、连杀检测）。
   - `UDOVerbMessageHelpers`：辅助库，提供「任意 UObject → PlayerState/PlayerController」反查，以及 `FDOVerbMessage` ↔ `FGameplayCueParameters` 双向桥接。
   - `UDOVerbMessageProcessor_DamageHUD`：**参考实现**（默认不编译），演示如何订阅伤害事件做飘字/VFX。

> **关键认知**：`UGameplayMessageSubsystem` 是**本地（per-World）**总线。服务端权威广播只在服务端本地生效；要让所有客户端收到「事件记录型」消息，靠挂在 `PlayerState` 上的 `FDOVerbMessageReplication` 跨网补充。两条路径最终落到**同一个本地总线**，订阅方无需关心消息来自哪条路径。

---

## 2. 目录文件清单

| 文件 | 类型 | 职责 |
|---|---|---|
| `DOVerbMessage.h/.cpp` | USTRUCT + 实现 | 通用事件消息载体 `FDOVerbMessage`，含 `ToString()` |
| `DONotificationMessage.h/.cpp` | USTRUCT + Tag 定义 | UI 通知消息 `FDONotificationMessage` + `TAG_DO_AddNotification_Message` |
| `DOVerbMessageReplication.h/.cpp` | FastArray USTRUCT | 事件记录型消息的网络复制容器 |
| `GameplayMessageProcessor.h/.cpp` | UActorComponent | 消息处理器基类（订阅生命周期管理） |
| `DOVerbMessageHelpers.h/.cpp` | UBlueprintFunctionLibrary | 对象反查 + GameplayCue 桥接 |
| `DOVerbMessageProcessor_DamageHUD.h/.cpp` | UGameplayMessageProcessor 子类 | 伤害 HUD 订阅方**参考实现**（默认不编译） |

---

## 3. 关键类与接口

### 3.1 消息载体

#### `FDOVerbMessage`（USTRUCT, BlueprintType）
通用事件消息。字段含义：

| 字段 | 含义 |
|---|---|
| `FGameplayTag Verb` | 事件类型标签，同时充当**订阅频道**（如 `Message.Combat.Damage.Applied`） |
| `TObjectPtr<UObject> Instigator` | 事件发起者（APawn/APlayerState/AActor），网络回调中常为裸 Actor，需经 Helpers 反查 PlayerState |
| `TObjectPtr<UObject> Target` | 事件作用对象 |
| `FGameplayTagContainer InstigatorTags` | 发起者标签，通常取自 `GameplayEffectContext::CapturedSourceTags` |
| `FGameplayTagContainer TargetTags` | 目标标签，取自 `CapturedTargetTags` |
| `FGameplayTagContainer ContextTags` | 扩展上下文（是否爆头/团队击杀/武器类型/阵营等） |
| `double Magnitude` | 事件数值，含义随事件而定（伤害值 / 助攻贡献值） |
| `FString ToString() const` | 调试用，底层 `UScriptStruct::ExportText` 文本化 |

#### `FDONotificationMessage`（USTRUCT, BlueprintType）
UI 通知消息。字段：

| 字段 | 含义 |
|---|---|
| `FGameplayTag TargetChannel` | 目标频道，决定由哪种 UI 组件消费（成就/击杀流/拾取流…各自订阅自己的 channel） |
| `TObjectPtr<APlayerState> TargetPlayer` | 目标玩家，留空 = 本机所有本地玩家 |
| `FText PayloadMessage` | 给玩家看的本地化文案 |
| `FGameplayTag PayloadTag` | 频道相关额外负载（风格 / 资产 id） |
| `TObjectPtr<UObject> PayloadObject` | 频道相关额外负载（图标/音效/DataTable 行等） |

相关 Tag：`TAG_DO_AddNotification_Message`（`"Message.UI.Notification.Added"`）。

### 3.2 网络复制

#### `FDOVerbMessageReplication` / `FDOVerbMessageReplicationEntry`（USTRUCT, FastArray）
- `Entry` 封装单条 `FDOVerbMessage`，是 FastArray 元素项。
- `AddMessage(const FDOVerbMessage&)`：服务端调用，塞入 `CurrentMessages` 并 `MarkItemDirty` 触发增量复制。
- `PostReplicatedAdd / PostReplicatedChange`：客户端收到后，逐条 `RebroadcastMessage()` 重新广播到本地 `UGameplayMessageSubsystem`。
- `SetOwner(UObject*)`：**必须**在复制前设置宿主（通常是 PlayerState），用于路由到 World 取本地总线。
- `NetDeltaSerialize`：交给 `FFastArraySerializer::FastArrayDeltaSerialize` 做增量 delta 序列化。
- 宿主位置：挂在 `ADOPlayerState` 上（见 `DOHealthComponent.h` 注释）。

### 3.3 订阅基类

#### `UGameplayMessageProcessor`（UActorComponent, Blueprintable）
- `BeginPlay()` → 自动调 `StartListening()`；`EndPlay()` → 先 `StopListening()`，再遍历 `ListenerHandles` 统一 `UnregisterListener`（防野回调）。
- `StartListening() / StopListening()`：子类重写，在里面 `RegisterListener` 并 `AddListenerHandle(...)` 存句柄。
- `AddListenerHandle(FGameplayMessageListenerHandle&&)`：存订阅句柄，零拷贝。
- `GetServerTime() const`：取 `GameState` 权威服务器时间，便于做时间窗判定（如「3 秒内 2 杀算连击」）。
- 惯例：**每个 World 在服务器上 spawn 一次**（不是每个玩家一份），事件只针对部分玩家时由处理器内部按 PlayerState 过滤。

### 3.4 辅助库

#### `UDOVerbMessageHelpers`（UBlueprintFunctionLibrary）
- `GetPlayerStateFromObject(UObject*)`：APlayerController/APlayerState/APawn → 对应 `APlayerState`（找不到返回 nullptr）。
- `GetPlayerControllerFromObject(UObject*)`：同上反查 `APlayerController`。
- `VerbMessageToCueParameters(const FDOVerbMessage&)`：把消息桥接到 GAS `FGameplayCueParameters`（`Verb→OriginalTag`、`Instigator→Instigator`、`Target→EffectCauser`、`*Tags→Aggregated*Tags`、`Magnitude→RawMagnitude`；`ContextTags` 暂未桥接）。
- `CueParametersToVerbMessage(const FGameplayCueParameters&)`：反向桥接。

### 3.5 参考实现

#### `UDOVerbMessageProcessor_DamageHUD`（`UGameplayMessageProcessor` 子类）
演示订阅 `Message.Combat.Damage.Applied` 并 `OnDamage` 回调（目前仅打日志，预留 HUD 飘字/Niagara/音效/震屏接入点）。**默认不编译**（启用方式见其 `.h` 顶部说明）。

---

## 4. 数据流（文字架构图）

```
[发布方：服务端权威]
  DOHealthComponent::BroadcastDamageApplied / BroadcastEliminationFired
        │  构造 FDOVerbMessage
        ▼
  UGameplayMessageSubsystem::BroadcastMessage(Verb, Msg)   ← 本地（服务端）总线
        │
        ├─(本地订阅方)─► UGameplayMessageProcessor 子类（HUD/统计/连击检测）
        │
        └─(跨网)─► PlayerState 上的 FDOVerbMessageReplication::AddMessage
                       │  FastArray 增量复制到客户端
                       ▼
                  客户端 PostReplicatedAdd/Change
                       │  RebroadcastMessage()
                       ▼
                  客户端本地 UGameplayMessageSubsystem
                       │
                       └─► 客户端 UGameplayMessageProcessor 子类（HUD 飘字等）
```

`FDONotificationMessage` 走独立频道 `Message.UI.Notification.Added`，由 UI 组件按 `TargetChannel` 各自订阅，不经过 FastArray 复制（通常靠 Client RPC / 本地触发）。

---

## 5. 使用方法与调用示例

### 5.1 发布消息（C++，服务端权威）
来自 `DOHealthComponent::BroadcastDamageApplied` 的真实写法：

```cpp
FDOVerbMessage DamageMsg;
DamageMsg.Verb       = DragonOathGameplayTags::Message::Combat::DamageApplied;
DamageMsg.Instigator = EffectCauser ? EffectCauser : EffectInstigator;
DamageMsg.Target     = GetOwner();
DamageMsg.Magnitude  = EffectMagnitude;

if (EffectSpec)
{
    if (const FGameplayTagContainer* SrcTags = EffectSpec->CapturedSourceTags.GetAggregatedTags())
        DamageMsg.InstigatorTags = *SrcTags;
    if (const FGameplayTagContainer* TgtTags = EffectSpec->CapturedTargetTags.GetAggregatedTags())
        DamageMsg.TargetTags = *TgtTags;
}

UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
MessageSubsystem.BroadcastMessage(DamageMsg.Verb, DamageMsg);
```

> 发布方只应在**服务端权威**处调用（伤害/击杀由服务器决定），客户端靠 `FDOVerbMessageReplication` 的跨网路径收到，不要双向重复广播。

### 5.2 订阅消息（C++，继承处理器基类）
来自 `UDOVerbMessageProcessor_DamageHUD::StartListening`：

```cpp
void UDOVerbMessageProcessor_DamageHUD::StartListening()
{
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);

    AddListenerHandle(
        MessageSubsystem.RegisterListener<FDOVerbMessage>(
            DragonOathGameplayTags::Message::Combat::DamageApplied,  // 订阅频道（默认 ExactMatch）
            this,                                                    // 成员函数监听（弱引用保护，避免野回调）
            &UDOVerbMessageProcessor_DamageHUD::OnDamage
        )
    );
}

void UDOVerbMessageProcessor_DamageHUD::OnDamage(FGameplayTag Channel, const FDOVerbMessage& Message)
{
    // Message.Instigator / Message.Target 在网络回调中常为裸 Actor，按需反查：
    if (APlayerState* PS = UDOVerbMessageHelpers::GetPlayerStateFromObject(Message.Instigator))
    {
        // ... 接 HUD 飘字 / Niagara / 音效 / 统计组件
    }
}
```

要点：
- 模板参数 `<FDOVerbMessage>` 必须与发布方 `BroadcastMessage` 的 payload 类型**严格一致**。
- 用**成员函数监听**而非裸 lambda 捕获 `this`，由子系统弱引用保护，Actor 销毁后不触发。
- `AddListenerHandle` 把句柄交给基类，EndPlay 统一反注册。
- 想监听「整个 Combat 子频道」可用 `Message.Combat` + `PartialMatch`（若 API 版本支持）。

### 5.3 蓝图侧订阅
- verb 消息：蓝图用 **"Listen for Gameplay Messages"** 节点，Channel 填 `Message.Combat.Damage.Applied`，Payload 选 `FDOVerbMessage`。
- 通知消息：订阅 `Message.UI.Notification.Added`，按 `TargetChannel` 过滤后读 `PayloadMessage` / `PayloadObject`。
- 辅助：`UDOVerbMessageHelpers` 的 `GetPlayerStateFromObject` / `GetPlayerControllerFromObject` 已标 `BlueprintCallable`，可在蓝图中反查对象。

### 5.4 跨网复制（挂载 Replication）
`FDOVerbMessageReplication` 挂在 `ADOPlayerState` 上，服务端在权威广播事件的同时调用：

```cpp
// 在 PlayerState 上持有 FDOVerbMessageReplication 成员并 SetOwner(this)
Replication.SetOwner(this);
// 服务端：把需要跨网的事件塞进复制容器
Replication.AddMessage(DamageMsg);
```

客户端 `PostReplicatedAdd/Change` 自动把消息重新广播到本地总线，订阅方代码与本地发布完全一致，无需区分来源。

### 5.5 桥接到 GameplayCue
```cpp
// 把消息总线事件转成 GameplayCue 参数，触发轻量特效/音效
FGameplayCueParameters CueParams = UDOVerbMessageHelpers::VerbMessageToCueParameters(DamageMsg);
// ... 喂给 ASC->ExecuteGameplayCue(...) / AddGameplayCue ...
```

---

## 6. 业务场景与触发时机

| 业务场景 | 消息 / 频道 | 发布时机 | 典型订阅方 |
|---|---|---|---|
| 受到伤害飘字 / 受击反馈 | `FDOVerbMessage` @ `Message.Combat.Damage.Applied` | GAS `GameplayEffect` 结算伤害时（服务端权威） | `DamageHUD` 处理器、Niagara 飘字、受击音效、震屏 |
| 击杀 / 击杀横幅 | `FDOVerbMessage` @ `Message.Combat.Elimination.Fired` | 目标血量归零、死亡状态机进入 DeathStarted | 击杀流横幅、连杀计数器、统计组件 |
| 助攻结算 | `FDOVerbMessage` @ `Message.Combat.Assist.Contributed` | 击杀者外、近期造成伤害的玩家结算时 | 助攻提示、经验/奖励分发 |
| 服务器权威重置 | `FDOVerbMessage` @ `Message.Combat.GameplayReset` | 回合/关卡重置时 | UI 清场、状态机复位 |
| 成就 / 拾取 / 击杀流提示 | `FDONotificationMessage` @ `Message.UI.Notification.Added` | 成就达成、道具入包等一次性 UI 事件 | 各类 UI Widget 按 `TargetChannel` 订阅 |
| 红点 / 新手引导 | 其他 Message.UI.* 频道（见 `DOGameplayTag.cpp`） | 状态变化时 | 对应 UI 模块 |
| 连击 / 连杀检测 | 组合多个 verb 消息 | 处理器内部基于 `GetServerTime()` 时间窗判定 | `UGameplayMessageProcessor` 子类 |

**触发时机要点：**
- 战斗结果（伤害/击杀）由**服务器权威**决定并通过 verb 消息广播；客户端只消费，不生产。
- 纯本地 UI 提示（如成就、红点）可在客户端直接广播 `FDONotificationMessage`，无需跨网。
- 处理器通常**每 World 一份**（非每玩家），需要按玩家区分时在回调内用 `GetPlayerStateFromObject` 过滤。
- 联机测试要求：**PIE Listen Server + 1 Client** 起，验证服务端事件能在客户端 HUD 正确触发。

---

## 7. 注意事项与常见坑

1. **payload 类型必须匹配**：`RegisterListener<T>` 的 `T` 与发布方 `BroadcastMessage` 的 payload 类型不一致时，订阅方收不到消息。
2. **不要裸 lambda 捕获 `this`**：用成员函数监听，或确保 lambda 对 `this` 做弱引用，否则 Actor 销毁后触发野回调。
3. **`FDOVerbMessageReplication` 必须 `SetOwner`**：宿主为 null 时 `RebroadcastMessage` 会 `check` 失败（见 `.cpp:66`）。
4. **`ContextTags` 未桥接 GameplayCue**：`VerbMessageToCueParameters` / 反向目前都不处理 `ContextTags`，扩展时需注意。
5. **避免双向广播**：服务端权威事件不要同时在客户端 `BroadcastMessage`，否则与复制路径重复触发。
6. **参考实现默认不编译**：`UDOVerbMessageProcessor_DamageHUD` 启用前需按其 `.h` 顶部说明处理（改文件名 / 复制 / 改 Build.cs 加 UMG·Niagara）。
7. **Tag 引用用 C++ 变量**：发布/订阅频道统一用 `DragonOathGameplayTags::Message::*`，不要手写 `FGameplayTag::RequestGameplayTag` 字符串（见 `AGENTS.md` 规范）。
