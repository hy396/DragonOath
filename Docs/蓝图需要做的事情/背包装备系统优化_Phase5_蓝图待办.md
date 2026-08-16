# 背包装备系统优化： Phase 5 蓝图待办

## C++ 已完成

- Inventory、Equipment、QuickBar 使用稳定槽位 ViewModel 缓存。
- `UDOItemTooltipViewModel` 集中 Tooltip、属性和装备比较文本。
- Slate 装备槽改为读取 `FDOEquipmentSlotViewModel`。
- DragDrop Payload 增加 `SourceDomain`，只传递操作意图。
- Icon 异步回调校验 `RequestedInstanceId` 和路径。
- OperationResult/Changed/超时共同维护 Pending。

## 0. 从零创建背包页面蓝图

当前项目没有预先创建的背包 WBP 时，按下面顺序在 Unreal Editor 中创建。页面蓝图只作为 CommonUI 页面壳，背包主体仍由 `UDOInventoryScreen::RebuildWidget()` 创建的原生 Slate 负责。

当前资产状态：

- [x] `/Game/DragonOath/BP/UI/WBP_InventoryScreen` 已创建，父类为 `DOInventoryScreen`。
- [x] `/Game/BP/Player/BP_DOPlayerController` 已创建或复用，父类为 `DOPlayerController`。
- [x] `/Game/DragonOath/UI/Inventory/{Frames,Materials,Fonts,Audio,Preview}` 资源目录已由脚本准备。
- [ ] 在编辑器 Class Defaults 中把“背包界面类”绑定为 `WBP_InventoryScreen`。

### 0.1 创建目录和 WBP

1. 在内容浏览器创建目录 `/Game/DragonOath/BP/UI`。
2. 在该目录中点击 **Add > User Interface > Widget Blueprint**。
3. 将父类改为 `DOInventoryScreen`（C++ 类 `UDOInventoryScreen`）。
4. 将资产命名为 `WBP_InventoryScreen`，保存后编译。
5. 不要在 Designer 中创建背包格子、装备槽、分页或操作栏；这些控件由 Slate 自动生成。

如果使用项目内脚本 `Tools/CreateInventoryUIAssets.py`，脚本会创建或复用：

- `/Game/DragonOath/BP/UI/WBP_InventoryScreen`
- `/Game/BP/Player/BP_DOPlayerController`

脚本不会强行写入受保护的 C++ 默认属性，避免不同 UE 版本的 Python 反射失败；控制器绑定按下一节手动完成。

### 0.2 创建或确认 PlayerController 蓝图

1. 打开 `/Game/BP/Player/BP_DOPlayerController`；如果项目已有同名蓝图，直接使用现有资产，不要重复创建。
2. 确认父类为 `DOPlayerController`（C++ 类 `ADOPlayerController`）。
3. 在 **Class Defaults > DO|UI > 背包界面类** 中选择 `WBP_InventoryScreen`。
4. 编译并保存 `BP_DOPlayerController`。
5. 确认测试地图使用的是该 PlayerController（World Settings > GameMode Override，或项目默认 GameMode 的 Player Controller Class）。

### 0.3 编辑器内验证页面链路

1. PIE 启动后按项目约定的 `I` 或 `B` 键，确认 `ToggleInventoryScreen()` 能打开页面。
2. 确认页面打开后由 `UDOInventoryScreen` 初始化 `UDOInventoryViewModel`，而不是蓝图创建第二个库存对象。
3. 确认鼠标、键盘和手柄焦点落在 Slate TileView；按 `Esc` 能关闭 CommonUI 页面。
4. 确认页面关闭时 ViewModel 执行 `Shutdown()`，再次打开后数据和 Pending 状态没有重复订阅。

### 0.4 当前视觉资源边界

- 当前第一轮视觉使用 `FDOInventoryStyle` 的 Slate Brush 作为稳定回退：金色外框、暖橙面板、紫色分类按钮和槽位占位图已经由 C++ 提供。
- 外部 PNG、字体、材质和角色 RenderTarget 后续按 `Docs/方案/纯Slate背包UI与外部资产自动化落地方案.md` 导入到 Content，再替换 StyleSet 或 ItemDefinition 的软引用。
- 不要为了替换边框或图标在 `/Game/DragonOath/BP/UI` 新建物品格 WBP；`WBP_InventoryScreen` 仍只承担 CommonUI 页面壳。
- 参考图中红框标注的右侧人物/宠物/社交信息区域暂不实现，避免引入与背包权威数据无关的第二套 UI 数据源。

## 编辑器/蓝图需要完成

1. 页面生命周期由 `UDOInventoryScreen` 管理：激活时初始化 `UDOInventoryViewModel`，关闭时调用 `Shutdown()`。
2. 物品格读取 ViewModel 的可见槽位，不复制或修改 Inventory FastArray。
3. 装备槽读取 ViewModel 的装备槽快照，不直接查询 EquipmentComponent。
4. Tooltip 统一调用 `UDOItemTooltipViewModel`；不要在 Widget 中重复解析 Definition 或属性 Map。
5. 拖放请求调用 ViewModel 的移动、装备、拆分和丢弃接口，保留 OperationId。
6. 操作结果订阅 Inventory/Equipment/QuickBar 的 OperationResult，Success、Failure、NoOp、Cancelled 均结束 Pending。
7. Icon、模型、材质使用软引用和异步加载；失败时显示占位资源。
8. 页面重新打开、切换过滤器或分页时执行 `Refresh()`。
9. 装备槽容器从 ViewModel 动态生成，不要在 WBP Designer 中重新硬编码九个槽位。

## UI 回归

- [ ] StackCount 变化只更新对应物品格。
- [ ] Filter、分页、选择、排序和装备比较正常。
- [ ] Tooltip 正确显示属性、强化、耐久、词缀和差值。
- [ ] 异步 Icon 晚到不会覆盖复用后的物品格。
- [ ] 拖放失败后 Selection、Pending 和显示快照恢复正确。
- [ ] 零库存 QuickBar 绑定显示为已绑定但不可使用。
- [ ] StackCount/Pending 等内容变化只触发格子重绘；排序、分页和增删才重建 TileView 列表。

## 不要做

- 不要在 UI Blueprint 里扣数量、改槽位或施加 GE。
- 不要因 Changed 消息清空整个域的 Pending。
- 不要在 DragDrop Payload 中保存 UObject、组件指针或最终属性结果。
