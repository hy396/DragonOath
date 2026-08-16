#include "ItemSystem/Core/DOItemDefinitionSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "ItemSystem/Core/DOItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DOItemDefinitionSubsystem)

UDOItemDefinitionSubsystem* UDOItemDefinitionSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UDOItemDefinitionSubsystem>();
		}
	}

	return nullptr;
}

const UDOItemDefinition* UDOItemDefinitionSubsystem::ResolveItemDefinition(const UObject* WorldContextObject, const FPrimaryAssetId& DefinitionId)
{
	if (UDOItemDefinitionSubsystem* Subsystem = Get(WorldContextObject))
	{
		return Subsystem->ResolveItemDefinitionSync(DefinitionId);
	}

	// Transient 测试对象通常没有 GameInstance，仍通过 AssetManager 保持可解析性。
	return ResolveWithAssetManager(DefinitionId);
}

const UDOItemDefinition* UDOItemDefinitionSubsystem::ResolveItemDefinitionSync(const FPrimaryAssetId& DefinitionId)
{
	if (!DefinitionId.IsValid())
	{
		return nullptr;
	}

	if (const UDOItemDefinition* CachedDefinition = FindLoadedDefinition(DefinitionId))
	{
		return CachedDefinition;
	}

	if (const UDOItemDefinition* Definition = ResolveWithAssetManager(DefinitionId))
	{
		CacheDefinition(DefinitionId, const_cast<UDOItemDefinition*>(Definition));
		return Definition;
	}

	return nullptr;
}

TSharedPtr<FStreamableHandle> UDOItemDefinitionSubsystem::RequestItemDefinitionAsync(const FPrimaryAssetId& DefinitionId, FDOItemDefinitionLoaded Delegate)
{
	if (!DefinitionId.IsValid())
	{
		Delegate.ExecuteIfBound(DefinitionId, nullptr);
		return nullptr;
	}

	if (const UDOItemDefinition* LoadedDefinition = FindLoadedDefinition(DefinitionId))
	{
		Delegate.ExecuteIfBound(DefinitionId, LoadedDefinition);
		return nullptr;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(DefinitionId);
	if (!AssetPath.IsValid())
	{
		Delegate.ExecuteIfBound(DefinitionId, nullptr);
		return nullptr;
	}

	TWeakObjectPtr<UDOItemDefinitionSubsystem> WeakThis(this);
	return AssetManager.GetStreamableManager().RequestAsyncLoad(
		AssetPath,
		FStreamableDelegate::CreateLambda([WeakThis, DefinitionId, Delegate = MoveTemp(Delegate)]() mutable
		{
			if (UDOItemDefinitionSubsystem* Subsystem = WeakThis.Get())
			{
				const UDOItemDefinition* Definition = Subsystem->ResolveItemDefinitionSync(DefinitionId);
				Delegate.ExecuteIfBound(DefinitionId, Definition);
			}
			else
			{
				Delegate.ExecuteIfBound(DefinitionId, nullptr);
			}
		}));
}

void UDOItemDefinitionSubsystem::ClearCache()
{
	DefinitionCache.Reset();
}

void UDOItemDefinitionSubsystem::Deinitialize()
{
	ClearCache();
	Super::Deinitialize();
}

const UDOItemDefinition* UDOItemDefinitionSubsystem::FindLoadedDefinition(const FPrimaryAssetId& DefinitionId) const
{
	if (const TWeakObjectPtr<UDOItemDefinition>* CachedDefinition = DefinitionCache.Find(DefinitionId))
	{
		if (CachedDefinition->IsValid())
		{
			return CachedDefinition->Get();
		}
	}

	return UAssetManager::Get().GetPrimaryAssetObject<UDOItemDefinition>(DefinitionId);
}

void UDOItemDefinitionSubsystem::CacheDefinition(const FPrimaryAssetId& DefinitionId, UDOItemDefinition* Definition)
{
	if (DefinitionId.IsValid() && Definition)
	{
		DefinitionCache.Add(DefinitionId, Definition);
	}
}

const UDOItemDefinition* UDOItemDefinitionSubsystem::ResolveWithAssetManager(const FPrimaryAssetId& DefinitionId)
{
	if (!DefinitionId.IsValid())
	{
		return nullptr;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	if (UDOItemDefinition* LoadedDefinition = AssetManager.GetPrimaryAssetObject<UDOItemDefinition>(DefinitionId))
	{
		return LoadedDefinition;
	}

	const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(DefinitionId);
	return AssetPath.IsValid() ? Cast<UDOItemDefinition>(AssetPath.TryLoad()) : nullptr;
}
