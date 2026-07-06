#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "DOCharacter.generated.h"

class ADOPlayerState;
class UDOAbilitySystemComponent;

/**
 * DragonOath 角色基类。
 *
 * 本类不直接创建 ASC，而是从 ADOPlayerState 取得 ASC 并把自己注册为 Avatar。
 * OwnerActor = PlayerState，AvatarActor = 当前 Pawn，这是多人 GAS 项目里常见的玩家角色结构。
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

	UFUNCTION(BlueprintCallable, Category = "DO|Character")
	ADOPlayerState* GetDOPlayerState() const;

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;

	// 服务端 PossessedBy、客户端 OnRep_PlayerState、BeginPlay 都可能成为 ASC 初始化入口。
	// 函数内部允许重复调用，最终只会把当前 Pawn 设置成 Avatar。
	void InitializeAbilitySystem();

private:
	// 缓存 PlayerState 上的 ASC，方便角色和蓝图快速访问。
	UPROPERTY(Transient)
	TObjectPtr<UDOAbilitySystemComponent> AbilitySystemComponent;
};
