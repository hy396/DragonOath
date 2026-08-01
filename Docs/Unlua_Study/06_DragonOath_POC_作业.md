# 06 DragonOath POC：本地交互提示编排

## 目标

做一个场景中的本地教学提示：Actor 出现在关卡后，等待 1 秒，再调用蓝图可访问的本地提示接口显示一句话。它不影响角色状态，不发送 RPC，不改 GAS 标签。

## 为什么选它

它覆盖了真正会用到的路径：Lua 模块加载、生命周期、计时状态、调用 UE 暴露接口、日志和重载；但没有把风险带到战斗与联机核心。

## 实施清单

1. 在 `/Game/UnLuaTutorial/` 创建 `BP_UnLuaPromptActor`，父类为 `Actor`。
2. 给蓝图提供一个仅本地表现的可调用函数，例如 `ShowLocalPrompt(Message)`。初学阶段可以只用 `Print String` 显示文本。
3. 创建 `Plugins/UnLua/Content/Script/Tutorial/PromptActor.lua`。
4. 在蓝图的 UnLua 模块绑定处填写 `Tutorial.PromptActor`。
5. 将 Actor 放进测试地图，在 PIE 中验证日志与提示。
6. 在 Listen Server + 1 Client PIE 中分别观察，确认它只是各自本地表现，未改变任何同步状态。

## Lua 骨架

```lua
local PromptActor = UnLua.Class()

function PromptActor:ReceiveBeginPlay()
    self.wait_seconds = 1.0
    self.has_shown = false
    print("[UnLua][Prompt] Actor started")
end

function PromptActor:ReceiveTick(delta_seconds)
    if self.has_shown then
        return
    end

    self.wait_seconds = self.wait_seconds - delta_seconds
    if self.wait_seconds <= 0.0 then
        self.has_shown = true
        self:ShowLocalPrompt("Approach the marker to interact")
        print("[UnLua][Prompt] Prompt shown")
    end
end

return PromptActor
```

`ShowLocalPrompt` 的具体名字应与蓝图中实际暴露的函数一致。若不存在该函数，先在蓝图实现它；不要在 Lua 中伪造同名字段。

## 验收标准

- PIE 启动后，日志只输出一次 `Actor started` 和一次 `Prompt shown`。
- 画面出现预期的本地提示。
- 删除或禁用 Lua 模块绑定后，提示不再执行，证明不是蓝图的其他图表在触发。
- Listen Server + 1 Client 测试中，未引入角色位置、属性、GameplayTag 或技能状态变化。

## 复盘问题

完成后，用自己的话回答：

1. 为什么 `self.has_shown` 要在 `ReceiveBeginPlay` 初始化？
2. 为什么不能把 `print` 放在每帧无条件执行？
3. 若提示内容需要由服务器决定，服务器应同步什么，Lua 负责什么？
4. 为什么这个 POC 不能改成“Lua 直接扣除闪避体力”？

建议答案：服务器同步或 RPC 下发**已裁决的结果/数据**，Lua 只读取并呈现；闪避体力属于 GAS 资源消耗与服务器权威，需要走 Ability/GameplayEffect 流程。
