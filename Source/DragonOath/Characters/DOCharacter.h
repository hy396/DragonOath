#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "DOCharacter.generated.h"

class UDOAbilitySystemComponent;

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

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;

	// 基类默认只初始化角色自身挂载的 ASC，适合怪物或简单 NPC。
	// 玩家角色会在 ADOPlayerCharacter 中重写，从 PlayerState 取得 ASC。
	virtual void InitializeAbilitySystem();

	// 真正执行 InitAbilityActorInfo 的通用工具函数。
	// OwnerActor 表示 ASC 的拥有者，AvatarActor 固定为当前 Character。
	void InitializeAbilitySystemComponent(UDOAbilitySystemComponent* InAbilitySystemComponent, AActor* InOwnerActor);

private:
	// 缓存当前角色正在使用的 ASC，方便角色和蓝图快速访问。
	UPROPERTY(Transient)
	TObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;
};
