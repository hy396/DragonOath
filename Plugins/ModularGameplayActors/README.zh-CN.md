# ModularGameplayActors 中文说明

## 定位

`ModularGameplayActors` 是 Lyra 的模块化 Actor 基类插件。

它基于 UE 的 `ModularGameplay`，让 GameFeature 或其他系统可以向 Character、Pawn、PlayerController、PlayerState、GameMode、GameState 等 Actor 注入组件。

## DragonOath 使用建议

- 如果后续采用 Lyra/GameFeature 扩展路线，建议早期就评估是否继承这些基类。
- 如果暂时不做 GameFeature，可以先作为 `CommonGame` 依赖保留。
- 玩家、怪物、控制器和状态类开工前，应决定是否继承 Modular 版本。

## 注意

- 它不是 AI 或 GAS 系统。
- 它的价值在“模块化扩展入口”。
- 一旦核心类继承路线确定，后期再改成本会较高。
