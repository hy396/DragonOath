#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DOItemDefinitionSubsystem.generated.h"

struct FStreamableHandle;
class UDOItemDefinition;

DECLARE_DELEGATE_TwoParams(FDOItemDefinitionLoaded, FPrimaryAssetId /*DefinitionId*/, const UDOItemDefinition* /*Definition*/);

/**
 * ItemDefinition 的统一解析与缓存入口。
 * 该 Subsystem 只负责只读静态定义的加载，不拥有背包、装备或存档状态。
 */
UCLASS()
class DRAGONOATH_API UDOItemDefinitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 从 WorldContext 获取当前 GameInstance 的定义 Subsystem；无 GameInstance 时返回 nullptr。 */
	static UDOItemDefinitionSubsystem* Get(const UObject* WorldContextObject);

	/** 统一同步解析入口，兼容测试中的 transient World 或无 World 对象。 */
	static const UDOItemDefinition* ResolveItemDefinition(const UObject* WorldContextObject, const FPrimaryAssetId& DefinitionId);

	/** 服务器事务使用的同步解析。未加载时允许通过 AssetManager 进行同步加载。 */
	const UDOItemDefinition* ResolveItemDefinitionSync(const FPrimaryAssetId& DefinitionId);

	/** UI 使用的异步解析入口，避免在展示热路径中阻塞游戏线程。 */
	TSharedPtr<FStreamableHandle> RequestItemDefinitionAsync(const FPrimaryAssetId& DefinitionId, FDOItemDefinitionLoaded Delegate);

	void ClearCache();

protected:
	virtual void Deinitialize() override;

private:
	const UDOItemDefinition* FindLoadedDefinition(const FPrimaryAssetId& DefinitionId) const;
	void CacheDefinition(const FPrimaryAssetId& DefinitionId, UDOItemDefinition* Definition);
	static const UDOItemDefinition* ResolveWithAssetManager(const FPrimaryAssetId& DefinitionId);

	UPROPERTY(Transient)
	TMap<FPrimaryAssetId, TWeakObjectPtr<UDOItemDefinition>> DefinitionCache;
};
