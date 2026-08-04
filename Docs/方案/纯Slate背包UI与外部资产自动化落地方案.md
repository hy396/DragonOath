# 1. 方案结论

可以由 Codex 完整负责这套背包与装备 UI 的实现。推荐继续采用“CommonUI 页面壳 + 原生 Slate 主体”的方案：页面生命周期、输入层级和菜单栈交给 CommonUI，背包格子、装备槽、Tooltip、拖拽、分页、快捷栏和高密度布局全部由 C++ Slate 构建。

本方案只描述执行计划。本阶段不生成图片、不导入或配置 Content 资产，也不修改 C++ 代码。真正开始执行时，再按本文档调用图像生成工具，生成图片并完成 Unreal 资产导入、配置和验证。

装备图标只负责 UI 展示，不驱动角色 Mesh、武器 Actor 或服饰外观。装备系统与未来的服饰/外观系统保持独立。

# 2. 目标与非目标

## 2.1 目标

- 建立可扩展的纯 Slate 背包、装备、快捷栏和 Tooltip UI。
- 支持外部制作的图片、材质、字体、音效和 RenderTarget。
- 让物品 DataAsset 只保存资源引用与物品数据，Slate 负责显示和交互。
- 资源采用软引用和异步加载，避免打开背包时同步加载大量图片。
- 保持服务器权威：Slate 只发起请求，不直接修改背包数量或装备结果。
- 为键鼠、手柄、不同分辨率和联机 PIE 预留验证路径。

## 2.2 非目标

- 不使用 UMG Designer 搭建背包主体。
- 不让 Lua 维护原生 `SWidget` 树，也不让 Lua 执行背包权威逻辑。
- 不在第一版实现装备改变角色外观。
- 不使用运行时从任意磁盘路径读取图片的方案作为正式资源管线。
- 不为每一个物品创建独立的 UI 蓝图。

# 3. 纯 Slate 使用外部资产是否可行

可行，但需要区分“外部制作文件”和“运行时资源”。Slate 不能直接把一个任意磁盘上的 PNG 路径当作稳定的游戏 UI 资源使用；正式运行时应将 PNG、材质、字体等导入 Unreal，成为 `UTexture2D`、`UMaterialInterface`、`UFont` 或其他 UObject 资产，再由 Slate 通过 `FSlateBrush` 使用。

运行时链路如下：

```text
外部 PNG / 材质 / 字体
        ↓ 导入 Unreal
UTexture2D / UMaterialInterface / UFont
        ↓ DataAsset 软引用
TSoftObjectPtr
        ↓ AssetManager / FStreamableManager 异步加载
FSlateBrush / FSlateFontInfo / FSlateSound
        ↓
SImage / SBorder / STextBlock / Slate 面板
```

因此，纯 Slate 并不意味着 UI 只能使用 C++ 中硬编码的颜色和几何图形。图标、背景、边框、遮罩、字体、音效和角色预览都可以来自 Content 资产。

# 4. 推荐目录与资源边界

外部原始文件和 Unreal 最终资产分开保存，便于重新生成、替换和审查。

```text
ArtSource/
  DragonOath/
    Inventory/
      Icons/
      Frames/
      References/

Content/DragonOath/
  Items/
    Definitions/
    Icons/
      Generated/
      Placeholder/
  UI/
    Inventory/
      Frames/
      Materials/
      Fonts/
      Audio/
      Preview/
```

`ArtSource` 保存生成工具或美术软件导出的源文件，不直接参与打包。`Content/DragonOath` 保存已经导入并可被 AssetManager、DataAsset 和 Slate 使用的 Unreal 资产。

建议资源命名如下：

```text
T_DO_ItemIcon_HealthPotion_Small
T_DO_ItemIcon_ManaPotion_Small
T_DO_ItemIcon_IronSword_01
T_DO_Inventory_Frame
T_DO_Inventory_Slot_Normal
T_DO_Inventory_Slot_Selected
T_DO_Inventory_Slot_Disabled
M_DO_Inventory_CooldownMask
M_DO_Inventory_RarityGlow
RT_DO_Inventory_CharacterPreview
```

物品定义继续放在 `Content/DragonOath/Items/Definitions`。当前 `UDOItemDefinition::Icon` 使用 `TSoftObjectPtr<UTexture2D>`，因此图标可以独立替换，不需要改动物品实例和背包存档数据。

# 5. Slate 与各类外部资产的使用方式

## 5.1 Texture2D

Texture2D 用于物品图标、面板背景、槽位边框、品质框、状态图标和占位图。加载完成后创建或更新 `FSlateBrush`，由 `SImage`、`SBorder` 或自定义 Slate 控件绘制。

图标默认使用正方形 PNG，并预留透明边距。物品图标、品质框和槽位背景分层显示，避免把所有效果烘焙成一张图，从而支持后续换品质、选中、禁用和冷却状态。

## 5.2 Material

Material 用于冷却遮罩、稀有度发光、选中高亮、禁用灰度和进度效果。需要动态参数时，创建动态材质实例并将其绑定到 Slate Brush；冷却剩余比例由 C++ 设置材质参数，不由图片本身承担。

材质只负责视觉表现，冷却时间和冷却缩减仍由 Gameplay Ability System 或共享冷却组件计算。Slate 读取只读状态并刷新显示。

## 5.3 Font

字体通过 `FSlateFontInfo` 和统一的 `FDOInventoryStyle` 使用。物品数量、装备属性、分类标题和 Tooltip 文本不在各个控件中散落配置字体，而是从 StyleSet 读取，便于后续切换主题和适配中日韩字符集。

正式字体需要确认授权、中文字符覆盖范围和打包规则。若字体来自外部文件，先导入 Unreal，再在 Slate 样式中引用，不在运行时直接访问操作系统字体路径。

## 5.4 Sound

点击、悬停、拖拽开始、装备成功、装备失败和物品使用音效通过 `FSlateSound` 或 Slate 应用层播放。音效资源由统一样式或 UI 音效服务管理，控件只发出语义事件，避免每个格子持有重复的音效对象。

图像生成工具只负责图片，不负责生成音效。音效执行阶段使用项目已有资源或单独提供的音频资源。

## 5.5 RenderTarget

RenderTarget 可用于装备页的角色 3D 预览：由独立的 SceneCapture 和预览 Actor 绘制，再以 Slate Brush 显示 `UTextureRenderTarget2D`。

第一版可以先使用角色占位预览或静态背景。预览只表现角色当前展示状态，不把“穿戴装备自动改变外观”写入装备系统；未来服饰系统建立独立的外观数据和预览接口后，再接入服饰预览。

# 6. 图片生成与导入执行流程

以下步骤只在真正开始制作资源时执行，当前阶段不执行。

## 6.1 生成前建立资源清单

先从物品定义和 UI 需求整理资源清单，每一行至少包含：

```text
资源逻辑名 | 类型 | 用途 | 尺寸 | 是否透明 | 对应 DataAsset | 状态
```

优先制作第一批可验证资源：两类回复药水、一个武器、一个头部装备、一个普通槽位、一个选中槽位、一个品质框和一个占位图。先验证完整链路，再批量生成全部图标。

## 6.2 调用图像生成工具

执行阶段由 Codex 根据资源清单批量调用图像生成工具。每批图片使用固定的风格说明、视角、光照、背景处理、描边规则和色彩范围，保证同一套物品图标具有一致性。

生成提示词中应明确：

- 单个物品居中，适合正方形 UI 图标；
- 保留透明安全边距，不切断武器尖端或瓶口；
- 不添加文字、数字、水印和不可控的 UI 边框；
- 同类物品只改变颜色、形状或材质特征；
- 需要透明通道时，要求透明背景；若工具输出实色背景，则进入后处理步骤。

生成结果先保存到 `ArtSource/DragonOath/Inventory/Icons`，经过人工或自动质量检查后，才复制到 Unreal 导入目录。当前文档阶段不会生成这些文件。

## 6.3 透明通道与质量检查

如果生成结果没有有效 Alpha：

1. 检查背景颜色是否统一。
2. 使用色键或图像后处理去除背景。
3. 检查物品边缘是否出现背景色溢出。
4. 检查半透明边缘、阴影和发光是否被错误抠除。
5. 导出为带 Alpha 的 PNG，并保留源图以便重新处理。

不建议直接把带纯色背景的图片当作正式图标导入，否则在不同槽位背景上会出现明显色块。

## 6.4 Unreal 导入设置

导入后按资源用途统一设置：

- `Texture Group`：`UI`；
- `Compression Settings`：`UserInterface2D`；
- 图标通常启用 `sRGB`；
- UI 小图按需要关闭 MipMap，避免缩小时出现不必要的模糊；
- Alpha 图标检查 `Has Alpha Channel` 和边缘过滤效果；
- 不把图标设置为 Virtual Texture，除非后续有明确的大规模 UI 资源需求；
- 资源命名、目录和引用必须与资源清单一致。

最终由物品 DataAsset 配置 `Icon` 软引用。Slate 不保存图片路径字符串，也不直接依赖外部文件路径。

# 7. Slate 资源加载与生命周期

## 7.1 异步加载

背包打开时先创建布局和占位图，再通过 AssetManager 或 `FStreamableManager` 异步加载当前可见物品的 Definition 与 Icon。加载完成后只刷新对应格子，不重建整棵 Slate 树。

推荐顺序：

```text
背包状态变化
  → ViewModel 得到 DefinitionId
  → 请求软引用资源
  → 显示占位图
  → 异步加载完成
  → 更新格子 Brush
  → 释放不可见资源引用
```

禁止在 Slate 的 Tick、拖拽回调或格子创建过程中对大量图标执行同步 `LoadSynchronous`。必要的同步加载只允许用于已经确认常驻的少量 UI 框架资源。

## 7.2 Brush 生命周期

`FSlateBrush` 必须由样式对象、资源缓存或对应 Slate 控件稳定持有，不能把局部变量的地址交给长期存在的控件。资源卸载前先清理 Brush 对资源的引用，防止 Slate 继续绘制已失效的 UObject。

品质框、槽位背景和通用按钮使用集中式 `FDOInventoryStyle`。物品图标使用按资源路径缓存的 Brush，避免同一张图在几十个格子中重复创建。

## 7.3 九宫格与缩放

面板背景和槽位边框使用九宫格资源及 `FSlateBoxBrush`，保证窗口缩放时四角不变形、边缘不拉伸。纯图标不使用九宫格，图标保持等比缩放并限制最大显示尺寸。

# 8. 背包与装备页面接入

## 8.1 页面结构

```text
CommonUI 页面壳
└── 原生 Slate 根控件
    ├── 左侧：分类、分页、筛选
    ├── 中部：可虚拟化物品格列表
    ├── 右侧：装备槽和选中物品详情
    ├── Tooltip：悬停或手柄聚焦信息
    └── Modal：丢弃、确认、错误提示
```

`UDOInventoryScreen` 负责页面生命周期和 Slate 根控件创建。`SDOInventoryEquipmentPanel` 及其子控件负责布局、绘制和交互。若当前 CommonUI 注册流程要求使用 Widget Blueprint，则 Blueprint 只作为页面壳，不在 Designer 中重新搭建背包主体。

## 8.2 物品格

每个格子接收只读的 ViewModel 数据：DefinitionId、Icon、数量、品质、是否选中、是否可用和冷却状态。格子不持有权威库存副本，不自行扣数量，也不直接发送服务器 RPC。

图标加载完成后显示 `SImage`；数量使用统一字体的 `STextBlock`；品质边框和选中效果由 StyleSet 或材质 Brush 叠加；冷却遮罩读取剩余比例并更新动态材质参数。

## 8.3 装备槽

装备槽显示当前装备的图标、槽位标签、品质和属性摘要。点击装备、卸下或交换时，由 Slate 调用 ViewModel 的请求接口，最终由服务器验证并修改 `UDOInventoryComponent` 与 `UDOEquipmentComponent`。

装备图标与角色外观完全解耦。未来增加服饰时，服饰系统单独提供外观预览数据，不复用装备事务作为外观变更入口。

## 8.4 快捷栏

快捷栏复用同一套图标加载和冷却显示组件，但采用更小的尺寸和更高的刷新频率。快捷栏只显示绑定结果，并通过 Item/Ability 请求接口使用物品；不可把快捷栏当作第二份库存数据。

# 9. Blueprint、Lua 与 Slate 的职责

Blueprint 适合承担：

- 创建和配置 `UDOItemDefinition` DataAsset；
- 配置 Icon、名称、描述、分类、装备槽和数值；
- 注册 CommonUI 页面壳、HUD 容器和输入层；
- 配置复杂消耗品的 GameplayAbility、动画和特效表现；
- 调整少量主题参数和占位资源。

Blueprint 不承担：

- 背包数量扣除、堆叠、移动和装备事务；
- 服务器 RPC 的权威校验；
- 伤害、治疗、冷却和属性结算；
- Slate 主体的大量格子布局和资源生命周期。

Lua 可以用于低频的展示层辅助，例如调试面板、运营活动筛选、临时文本和非权威提示，但不建议让 Lua 直接维护原生 Slate 控件树，也不用于背包权威逻辑、装备事务、网络请求或 GAS 结算。第一版保持 C++ Slate + C++ ViewModel，减少 UnLua 与 Slate 引用计数、生命周期和网络状态之间的耦合。

# 10. 由 Codex 完整执行时的工作顺序

## 阶段 A：接口和资源清单

- 核对物品 Definition、Inventory ViewModel、Equipment ViewModel 和 QuickBar ViewModel 的字段。
- 扫描需要图标的 DataAsset，生成资源清单和缺失资源报告。
- 固定图标尺寸、风格、命名、目录和导入规则。

## 阶段 B：Slate 基础设施

- 实现统一 StyleSet、颜色、字体、Brush、材质和音效入口。
- 实现异步软引用加载、占位图、缓存和加载失败回退。
- 实现通用 ItemSlot、EquipmentSlot、Tooltip、CooldownOverlay 和分页控件。

## 阶段 C：页面和交互

- 接入 CommonUI 页面栈和输入层。
- 接入背包、装备、快捷栏 ViewModel。
- 完成点击、聚焦、拖拽、分页、筛选、装备、卸下和使用请求。
- 确认所有状态修改仍经过服务器权威流程。

## 阶段 D：图片生成与 Unreal 资产配置

- 根据资源清单调用图像生成工具。
- 进行透明通道、边缘、尺寸、命名和风格一致性检查。
- 导入 PNG，设置 UI 纹理参数并保存最终资产。
- 将图标配置到对应 ItemDefinition DataAsset。
- 创建或配置槽位边框、冷却遮罩、选中高亮和角色预览 RenderTarget。
- 对缺少音频的部分使用项目现有 UI 音效或明确的占位音效。

## 阶段 E：验证与交付

- 检查所有物品图标是否能从软引用异步加载。
- 检查资源缺失时是否显示占位图并输出可定位日志。
- 检查窗口缩放、Tooltip 裁剪、手柄聚焦和拖拽反馈。
- 使用 PIE Listen Server + 1 Client 验证装备和物品使用请求。
- 验证打包后 AssetManager 能找到 Definition 和 Icon，不能只在编辑器未保存状态下通过。
- 更新蓝图待办和资源清单，记录仍需人工确认的资产。

# 11. 验收标准

- 背包主体没有依赖 Widget Blueprint Designer，核心布局由 Slate 生成。
- 物品 Definition 只保存软引用，图标不写死在 Slate 控件中。
- 图标、边框、材质、字体、音效和 RenderTarget 都能按统一入口替换。
- 图标异步加载期间有占位显示，加载失败有回退显示。
- 打开背包不会因为批量同步加载图标造成明显卡顿。
- 装备变化不会自动修改角色外观；服饰系统可以在未来独立接入。
- 客户端不能通过 UI 直接决定库存数量、装备结果、属性效果或冷却结束时间。
- 资源目录、命名、导入设置和 DataAsset 引用均通过检查。

# 12. 当前阶段结论

本阶段只落地方案文档，不生成任何图片，不修改 C++，不修改 Blueprint 和 Content 资产。执行阶段优先完成“少量图标 + 一个完整背包格子链路”的垂直切片，确认外部图片生成、导入、软引用、异步加载和 Slate 显示全部正常后，再批量制作剩余资源。
