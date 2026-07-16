#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "DOCharacter.generated.h"

class UDOAbilitySystemComponent;
class UDOHealthSet;
class UDOResourceSet;
class UDOCombatSet;
class UDOHealthComponent;
struct FOnAttributeChangeData;

/**
 * DragonOath 角色基类。
 *
 * 本类只处理 Character 和 ASC 的通用连接，不写玩家专属逻辑。
 * 玩家从 PlayerState 拿 ASC 的规则放在 ADOPlayerCharacter；怪物可以在自己的类或蓝图上挂 ASC。
 */
UCLASS()
class DRAGONOATH_API ADOCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADOCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "DO|Character")
	UDOAbilitySystemComponent* GetDOAbilitySystemComponent() const;

	// 角色等级，玩家和怪物共用。玩家后续可在 PlayerState 中覆盖。
	UFUNCTION(BlueprintPure, Category = "DO|Character")
	int32 GetCharacterLevel() const { return CharacterLevel; }

	UFUNCTION(BlueprintCallable, Category = "DO|Character")
	void SetCharacterLevel(int32 NewLevel) { CharacterLevel = FMath::Max(1, NewLevel); }

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;

	// 基类默认只初始化角色自身挂载的 ASC，适合怪物或简单 NPC。
	// 玩家角色会在 ADOPlayerCharacter 中重写，从 PlayerState 取得 ASC。
	virtual void InitializeAbilitySystem();

	// 真正执行 InitAbilityActorInfo 的通用工具函数。
	// OwnerActor 表示 ASC 的拥有者，AvatarActor 固定为当前 Character。
	void InitializeAbilitySystemComponent(UDOAbilitySystemComponent* InAbilitySystemComponent, AActor* InOwnerActor);

	// 绑定 MoveSpeed 属性变化委托，同步到 CharacterMovementComponent::MaxWalkSpeed。
	// 在 ASC 初始化后调用。
	void BindAttributeChangeDelegates();

	// MoveSpeed 属性变化回调，直接写入 MaxWalkSpeed。
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

private:
	// 缓存当前角色正在使用的 ASC，方便角色和蓝图快速访问。
	UPROPERTY(Transient)
	TObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;

	// 属性集：玩家 Pawn 也继承此构造函数，会产生冗余实例，但不会参与玩家 ASC 注册（玩家 ASC 挂在 PlayerState）。
	// 怪物 / NPC 的 ASC 挂在自身，这里的属性集会被自动注册，使其拥有完整战斗/资源属性。
	UPROPERTY()
	TObjectPtr<UDOHealthSet> HealthSet;

	UPROPERTY()
	TObjectPtr<UDOResourceSet> ResourceSet;

	UPROPERTY()
	TObjectPtr<UDOCombatSet> CombatSet;

	// 死亡行为组件：状态机 / Tag 应用 / FDOVerbMessage 广播。
	// 怪物：与 ASC 一起挂自身（与 HealthSet 同生命周期）。
	// 玩家：玩家 Character 上也创建一份，但实际玩家 ASC 在 PlayerState，
	//       所以玩家 Character 上的这个组件是冗余实例（不影响功能，Character 销毁时一起释放）。
	UPROPERTY()
	TObjectPtr<UDOHealthComponent> HealthComponent;

	// 角色等级。玩家默认 1，怪物由数据表配置。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DO|Character", Meta = (AllowPrivateAccess = "true"))
	int32 CharacterLevel = 1;
};
