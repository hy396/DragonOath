# 04. Local Message Bus

## 当前决定

DragonOath 纳入 `GameplayMessageRouter` 作为项目本地消息总线。

它来源于 Epic Lyra 风格的 GameplayTag 广播订阅模型，适合让 UI、音效、特效、提示、调试面板等系统在本机内解耦通信。

## 定位

`GameplayMessageRouter` 负责：

- 本地跨系统事件分发
- 降低模块之间的 include 和对象引用依赖
- 使用 GameplayTag 作为消息频道
- 使用 USTRUCT 作为消息 Payload
- 支持 C++ 和 Blueprint 监听

`GameplayMessageRouter` 不负责：

- 服务器权威逻辑
- 客户端到服务器通信
- 服务器到客户端同步
- 属性、背包、掉落、任务等真实状态复制
- 高频 Tick 级数据流

## 联机边界

这个插件本身不是网络消息系统。

源码特征：

```text
UGameplayMessageSubsystem : UGameInstanceSubsystem
BroadcastMessageInternal -> 遍历本地 ListenerMap
无 Server / Client / NetMulticast RPC
无 Replicated / NetSerialize / GetLifetimeReplicatedProps
```

因此：

```text
服务器 BroadcastMessage
  -> 只通知服务器本地监听者

客户端 BroadcastMessage
  -> 只通知该客户端本地监听者
```

正确联机流程：

```text
服务器权威逻辑
  -> GAS / RPC / Replicated Property / GameplayCue
  -> 客户端收到复制或通知
  -> 客户端本地 Broadcast GameplayMessage
  -> UI / Audio / VFX / Debug 监听并表现
```

## 推荐使用场景

适合：

- Attribute 变化后通知 HUD 刷新
- 技能冷却变化后通知技能栏
- 背包数据变化后通知背包界面
- 任务状态变化后通知 UI / 音乐 / 提示
- 本地收到 GameplayCue 后通知 Niagara / 摄像机反馈
- 调试系统监听战斗事件

不适合：

- 造成伤害
- 决定死亡
- 生成掉落
- 修改背包真实数据
- 提交技能释放请求
- 同步任务进度

这些必须走 GAS、服务器 RPC、复制属性、PlayerState、GameState 或后端服务。

## 命名规范

消息频道使用 GameplayTag，建议三段式：

```text
Message.UI.Inventory.Changed
Message.UI.Skill.CooldownChanged
Message.Combat.Damage.Applied
Message.Combat.Target.Killed
Message.Audio.Music.Changed
Message.VFX.Hit.Fired
Message.Debug.Network.StateChanged
```

规则：

- 统一以 `Message.` 开头。
- 第二段表示领域，例如 `UI`、`Combat`、`Audio`、`VFX`、`Debug`。
- 第三段开始表示对象和动作。
- 默认使用精确匹配。
- 只有日志、调试、聚合面板才使用 Partial Match。

## Payload 规范

Payload 必须是明确的 `USTRUCT(BlueprintType)`。

示例：

```cpp
USTRUCT(BlueprintType)
struct FDOCombatDamageMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> SourceActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    float Damage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bCritical = false;
};
```

规则：

- Payload 只描述“已经发生的事实”，不承载“请求服务器做某事”。
- 不把大对象、复杂容器、临时指针长期保存进 Payload。
- 蓝图收到 Payload 后可以拷贝字段，但不要跨帧保存内部指针。
- 高频连续数值不要每帧广播，使用属性绑定、复制属性或专门组件。

## 项目推荐流程

伤害数字：

```text
服务器计算伤害
  -> Health 复制 / GameplayCue 到客户端
  -> 客户端生成 FDOCombatDamageMessage
  -> Broadcast Message.Combat.Damage.Applied
  -> Niagara 伤害数字系统监听并表现
```

技能栏：

```text
ASC 冷却 Tag 或 Cooldown GE 变化
  -> 本地 Ability UI Adapter 整理显示数据
  -> Broadcast Message.UI.Skill.CooldownChanged
  -> 技能栏 Widget 监听并刷新
```

背包：

```text
服务器确认背包变化
  -> replicated inventory data / OnRep
  -> 客户端 Broadcast Message.UI.Inventory.Changed
  -> 背包 Common UI 界面监听并刷新
```

## 生命周期

监听者必须保存 `FGameplayMessageListenerHandle`。

销毁时必须调用：

```cpp
ListenerHandle.Unregister();
```

UI Widget 建议在 `NativeConstruct` 注册，在 `NativeDestruct` 反注册。

Actor / Component 建议在 `BeginPlay` 注册，在 `EndPlay` 反注册。

## 与其他系统的关系

```text
GAS
  -> 决定技能、属性、Buff、伤害
  -> 可以在本地收到结果后广播 Message

Common UI / UMG
  -> 监听 Message 刷新显示
  -> 不通过 Message 修改权威数据

Niagara
  -> 可以监听本地战斗 Message 表现伤害数字、命中特效

GameplayMessageRouter
  -> 解耦表现层和系统层
  -> 不替代网络复制
```
