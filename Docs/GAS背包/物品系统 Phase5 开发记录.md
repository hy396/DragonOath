# 物品系统 Phase 5 开发记录

# 1. 系统功能说明

Phase 5 保留现有 CommonUI、Slate 和 GameplayMessageRouter，将背包 UI 的领域逻辑从 Widget 中抽离：

- Inventory、Equipment、QuickBar 使用稳定的槽位 ViewModel 缓存。
- `UDOItemTooltipViewModel` 集中生成物品详情、装备属性和当前装备比较文本。
- Slate 装备槽读取 `FDOEquipmentSlotViewModel`，不再直接查询 EquipmentComponent。
- DragDrop Payload 增加 `SourceDomain`，只传递操作意图和实例身份。
- 异步 Icon 加载使用 `RequestedInstanceId + SoftObjectPath` 校验，避免旧回调覆盖复用后的格子。
- OperationResult、Changed 和超时共同驱动 Pending 状态，成功/失败/NoOp/Cancelled 都能结束请求。

# 2. C++ 架构说明

```text
Inventory/Equipment/QuickBar Component
              |
              v
      GameplayMessageRouter
              |
              v
      UDOInventoryViewModel
       |       |        |
       v       v        v
  Slot VM  Equipment VM Tooltip VM
       |
       v
  Slate / CommonUI Widget
```

| 类型 | 职责 |
|---|---|
| `FDOInventorySlotViewModel` | 单个背包格的只读展示快照。 |
| `FDOEquipmentSlotViewModel` | 单个装备槽的只读展示快照。 |
| `UDOItemTooltipViewModel` | 统一 Tooltip、装备属性、词缀和比较文本。 |
| `UDOInventoryViewModel` | 过滤、分页、选择、Pending 和子 ViewModel 缓存。 |
| `UDOItemQuickBarViewModel` | 固定快捷栏槽位缓存和使用 Pending。 |
| `SDOInventorySlotWidget` | 输入、拖放和资源显示，不负责领域查询或修改数据。 |

# 3. 蓝图编辑器配置教程

### 3.1 DataAsset 与资源配置

1. ItemDefinition 的 `DisplayName`、`Description`、`Icon`、ItemType、Rarity 和 Fragment 必须完整。
2. Icon 使用 `TSoftObjectPtr<UTexture2D>`，Widget 通过 Asset Manager 异步加载。
3. 装备 Tooltip 的属性以 `AttributeModifiers` 为主；Legacy Map 只在迁移期回退。
4. Appearance 资源由 Phase 4 的 `AppearanceId` 注册表解析，不能把资源指针塞进网络实例。

### 3.2 GameplayTag / GameplayAbility 配置

- UI 只订阅 `Message.UI.Inventory.Changed`、`Message.UI.Equipment.Changed`、`Message.UI.ItemQuickBar.Changed` 和各域 OperationResult。
- 装备公开表现订阅 `Message.UI.Equipment.PresentationChanged`。
- Ability/GE 的实际执行仍由服务器 GAS；Widget 不直接激活技能、不施加 GE、不扣除数量。
- DragDrop 使用 `EDOItemOperationDomain` 标明 Inventory 或 Equipment 意图。

### 3.3 Actor / Component 挂载

- `UDOInventoryViewModel` 与 `UDOItemQuickBarViewModel` 由现有 UI 页面生命周期创建和销毁。
- ViewModel 通过 PlayerState 获取组件弱引用；不拥有组件，不缓存权威数组。
- `UDOInventoryScreen` 负责页面激活、ViewModel 初始化和关闭时 Shutdown。
- Slate 槽位控件只持有对应的共享 ViewModel 快照。

### 3.4 UI 数据流

```text
服务器事务提交 / FastArray 接收
        -> Component Changed / OperationResult
        -> GameplayMessageRouter
        -> ViewModel Refresh 或清理对应 OperationId
        -> 稳定 SlotViewModel 更新
        -> Slate/UMG 局部重绘
```

普通数量变化只更新对应实例格；Sort、Restore、容量改变等显式全量刷新。异步图标回调在写回前验证 InstanceId 和路径。

### 3.5 Tooltip 和 DragDrop 配置教程

1. 在背包 Widget 中使用 `UDOInventoryViewModel::GetVisibleSlots()` 创建物品格。
2. Tooltip 请求转发给 `UDOItemTooltipViewModel::BuildForInventorySlot` 或 `BuildForEquipmentSlot`。
3. 装备槽点击/拖放通过 ViewModel 的 `RequestEquipInstance`、`RequestUnequip` 发起请求。
4. DragDrop 不携带 UObject、组件指针或最终数量，只携带 InstanceId、源域、源槽位和意图数量。
5. 图标加载失败显示占位纹理，不阻塞组件操作结果。

### 3.6 编辑器操作步骤

1. 打开背包页面 Widget，确认按钮调用 ViewModel 而不是直接修改组件数组。
2. 确认所有 OperationResult 通道已绑定，NoOp/Cancelled 也会关闭 Pending。
3. 检查装备槽 Widget 不再通过 `GetEquipmentComponent()->Find...` 读取数据。
4. 为 Icon 和表现注册表配置可异步加载的软引用。
5. PIE 中关闭/重新打开背包页面，确认 ViewModel 从权威快照恢复选择和槽位显示。

`UDOInventoryScreen` 已是 `UCommonActivatableWidget`，因此项目只需创建其 Blueprint 子类并在 PlayerController 的 `InventoryScreenClass` 指定，不需要在 Blueprint 中重复创建 Inventory/Equipment Component。页面 Widget 的视觉布局仍由现有 C++ Slate 面板生成；Blueprint 负责页面类、输入层和样式资源绑定。

# 4. 测试流程

### 单机

- 修改一个 StackCount，确认只有对应格刷新。
- 排序、分页、过滤、选择和拖放后检查缓存对象没有重复创建。
- Tooltip 验证属性、强化、耐久、词缀和当前装备差值。
- Icon 延迟加载后替换物品，确认旧回调不会覆盖新格子。

### 网络

Listen Server + 1 Client 下验证 Result/Changed 任意到达顺序、Pending 超时和页面重新激活；远端客户端只使用公开表现数据。

### GAS

装备属性比较只读取定义值和实例强化用于展示，最终战斗属性仍从 ASC AttributeSet 快照读取；UI 不作为属性权威来源。

# 5. 常见错误

| 现象 | 原因 | 处理 |
|---|---|---|
| Tooltip 与角色属性不一致 | Widget 自己累加旧 Map | 使用 Tooltip ViewModel 和 typed 属性；最终战力从 ASC 读取。 |
| 格子显示上一件物品图标 | 异步回调没有校验请求实例 | 比较 `RequestedInstanceId` 和路径。 |
| 拖放后重复请求 | Widget 绕过 ViewModel 或 Pending 未锁定 | 所有操作统一由 ViewModel 生成 OperationId。 |
| 页面重开显示旧数据 | 只依赖旧 Changed 消息 | 页面激活时主动 `Refresh()` 全量读取快照。 |
| 装备槽与背包数据不一致 | Slate 直接查组件并保存局部副本 | 只读取稳定 EquipmentSlotViewModel。 |

## 验证状态

Phase 5 C++ ViewModel/Slate 重构主体已完成，Slate Tooltip 的旧重复解析分支已经删除，并通过静态验收：Inventory/Equipment/QuickBar 使用稳定 ViewModel 缓存，Tooltip 只从 ViewModel 构建，DragDrop 携带 `SourceDomain`，异步 Icon 回调校验 `InstanceId + SoftObjectPath`。本轮还修复了 Slate 面板对 `UDOInventoryComponent` 的完整类型依赖，DragonOath 模块源码编译和链接通过。仍需要在编辑器中完成 Widget 绑定、软引用资源、Appearance 注册表和网络 PIE 回归。

## 6. 本轮补充

- 装备槽 UI 不再硬编码九个槽位，改为读取 `UDOEquipmentComponent::GetSupportedSlotTags` 和 `UDOEquipmentLayout` 配置；未配置 Layout 时保持现有默认槽位。
- Inventory ViewModel 记录可见列表的结构变化：仅 StackCount、Pending、装备状态或 Tooltip 内容变化时只做 Slate Paint Invalidate；排序、分页和条目增删才请求 TileView 重建。
- 仍需在编辑器执行 Phase 5 UI 回归，确认实际 Widget/Slate 容器在资产切换和网络延迟下正确重绘。
