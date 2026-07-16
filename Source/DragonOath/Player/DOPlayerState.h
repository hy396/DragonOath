#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"

#include "DOPlayerState.generated.h"

class UDOAbilitySystemComponent;
class UDOHealthSet;
class UDOResourceSet;
class UDOCombatSet;
class UDOHealthComponent;
class UDOProfessionAbilityConfig;

/**
 * 玩家状态。
 *
 * PlayerState 生命周期比 Pawn 更稳定，适合拥有 ASC 和持久属性集。
 * 玩家死亡、换 Pawn、重生时，能力和属性不会因为 Pawn 被销毁而丢失。
 *
 * 职业技能也在这里初始化：服务端根据职业 Tag 找到 AbilitySet，一次性授予全部技能。
 */
UCLASS()
class DRAGONOATH_API ADOPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADOPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "DO|PlayerState")
	UDOAbilitySystemComponent* GetDOAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "DO|PlayerState")
	UDOHealthSet* GetHealthSet() const;

	UFUNCTION(BlueprintPure, Category = "DO|PlayerState")
	UDOResourceSet* GetResourceSet() const;

	UFUNCTION(BlueprintPure, Category = "DO|PlayerState")
	UDOCombatSet* GetCombatSet() const;

	// 当前职业标识
	UFUNCTION(BlueprintPure, Category = "DO|Profession")
	FGameplayTag GetProfessionTag() const { return ProfessionTag; }

	// 服务端切换职业：清除旧技能 -> 设置新职业 Tag -> 授予新技能
	UFUNCTION(BlueprintCallable, Category = "DO|Profession")
	void SetProfession(FGameplayTag NewProfession);

	// 服务端授予职业技能。需要在 ASC 初始化后调用（由 ADOPlayerCharacter::InitializeAbilitySystem 触发）。
	void GrantProfessionAbilities();

	// 确保职业已就绪：未设定则按兜底来源设定并授予，已设定则保证技能已授予。
	// 由 ADOPlayerCharacter::InitializeAbilitySystem 在服务器侧调用。
	void EnsureProfessionSet();

protected:
	// 职业变更时的客户端回调（更新 UI 等）
	UFUNCTION()
	void OnRep_ProfessionTag();

private:
	// 玩家真正的 ASC。Mixed 复制模式下，Owner 能拿到完整 GE 信息，其它客户端只需要标签和 Cue。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DO|Ability", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;

	// 生命相关属性集：Health、MaxHealth、Damage、HealthRegen 等。
	UPROPERTY()
	TObjectPtr<UDOHealthSet> HealthSet;

	// 资源属性集：Mana、Stamina 及其上限、ManaRegen。第二阶段从 DOPlaySet 拆分。
	UPROPERTY()
	TObjectPtr<UDOResourceSet> ResourceSet;

	// 战斗属性集：AttackPower、DefensePower、MoveSpeed、暴击/命中/闪避/攻速/吸血。第二阶段从 DOPlaySet 拆分。
	UPROPERTY()
	TObjectPtr<UDOCombatSet> CombatSet;

	// 死亡行为组件：状态机 / Tag 应用 / FDOVerbMessage 广播。
	// 玩家走 PlayerState 路径，所以这个组件挂在 PlayerState 而不是 Character 上 —— 这样玩家死亡/换 Pawn/重生时，
	// 死亡状态机与死亡 Tag 不会因为 Character 销毁而丢失。
	UPROPERTY()
	TObjectPtr<UDOHealthComponent> HealthComponent;

	// 当前职业标识，复制到客户端
	UPROPERTY(ReplicatedUsing = OnRep_ProfessionTag, EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Profession", Meta = (AllowPrivateAccess = "true"))
	FGameplayTag ProfessionTag;

	// 职业技能总配置（DA_ProfessionAbilityConfig 资产）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Profession", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDOProfessionAbilityConfig> ProfessionAbilityConfig;

	// 兜底职业来源。原型期默认值；后续接存档/Loadout/选人流程时可替换来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Profession", Meta = (AllowPrivateAccess = "true", Categories = "Profession"))
	FGameplayTag DefaultProfessionTag;

	// 服务端标记，防止重复授予
	bool bProfessionAbilitiesGranted = false;
};