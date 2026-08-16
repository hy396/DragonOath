# 背包装备系统优化：Phase 3 蓝图待办

## C++ 已完成

- `UDOEquipmentInstance` 保存单件装备的属性 GE Handle、AbilitySpec/GE/Tag 授予句柄。
- `UDOAbilitySet` 支持 `GrantedAbilities`、`GrantedGameplayEffects` 和 `GrantedTags`。
- 装备 GAS 通过 `EquipmentAbilitySet` 配置，SourceObject 指向具体 EquipmentInstance。
- 卸下装备只撤销当前实例来源，不清理职业 Ability。
- 装备属性使用 `AttributeModifiers` + 原生 `UDOEquipmentAttributeEffect` + SetByCaller Builder。

## 编辑器/蓝图需要完成

1. 为每类装备创建 `DA_AbilitySet_Equipment_*`。
2. `GrantedAbilities` 的 `AbilityClass` 必须选择具体、非 Abstract 的 `DOGameplayAbility` 蓝图子类。
3. `AbilityId` 选择 `Ability.Id.*`，输入/事件 Tag 使用集中声明的 `InputTag.*` / `Event.*`。
4. 被动 GE 配置有效 DurationPolicy；使用 SetByCaller 的 Modifier 必须由 C++ Spec Builder 写值。
5. 在装备 `DOItemDefinition` 的 `DOItemFragment_Equipment` 中填写 `EquipmentAbilitySet`。
6. 旧资产从 `BaseAttributeMagnitudes` 迁移到 `AttributeModifiers`，迁移后执行 Validate。
7. 确认 PlayerState 只挂载一个 EquipmentComponent 和 ASC；不要在 Character Blueprint 重复添加。
8. 如需自定义装备部位，创建 `UDOEquipmentLayout` DataAsset 并在 EquipmentComponent 的 Layout 属性中指定；未指定时使用默认九槽位。
9. 对可跨多个部位使用的装备，在 Equipment Fragment 配置 `CompatibleSlotQuery`，并用 Data Validation 检查查询结果。

## 编辑器测试

- [ ] 穿戴后 GA、GE、GrantedTags 出现在 PlayerState ASC。
- [ ] 卸下后只移除该装备来源，职业技能仍可激活。
- [ ] 两件装备提供同类效果时可分别卸下。
- [ ] 替换失败、背包已满时旧装备和旧 GAS 状态保留。
- [ ] 读档、重生和重新 Possess 后装备 GAS 状态一致。
- [ ] 耐久归零时属性和装备 Ability 被撤销，修复耐久后重新生效。

## 不要做

- 不要把 `UDOGameplayAbility_Dash` 等 Abstract C++ 类直接填入 AbilitySet。
- 不要在 Widget 或 Blueprint 中直接调用 `ClearDOAbilities()`。
- 不要保存 EquipmentInstance、ActiveGameplayEffectHandle 或 AbilitySpecHandle 到 SaveGame。
