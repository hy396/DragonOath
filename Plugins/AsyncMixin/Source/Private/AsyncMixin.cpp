// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：异步加载生命周期辅助，用来串联和取消异步加载请求，避免对象销毁后回调失效。

#include "AsyncMixin.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogAsyncMixin, Log, All);

// 全局加载状态映射——FAsyncMixin* → FLoadingState
TMap<FAsyncMixin*, TSharedRef<FAsyncMixin::FLoadingState>> FAsyncMixin::Loading;

FAsyncMixin::FAsyncMixin()
{
}

FAsyncMixin::~FAsyncMixin()
{
	check(IsInGameThread());

	// 析构时自动移除加载状态——这会取消所有未完成的加载，并阻止后续回调
	Loading.Remove(this);
}

const FAsyncMixin::FLoadingState& FAsyncMixin::GetLoadingStateConst() const
{
	check(IsInGameThread());
	return Loading.FindChecked(this).Get();
}

FAsyncMixin::FLoadingState& FAsyncMixin::GetLoadingState()
{
	check(IsInGameThread());

	// 已存在则返回，不存在则创建
	if (TSharedRef<FLoadingState>* LoadingState = Loading.Find(this))
	{
		return (*LoadingState).Get();
	}

	return Loading.Add(this, MakeShared<FLoadingState>(*this)).Get();
}

bool FAsyncMixin::HasLoadingState() const
{
	check(IsInGameThread());

	return Loading.Contains(this);
}

// 取消所有未完成的异步加载
void FAsyncMixin::CancelAsyncLoading()
{
	// 如果没有加载状态（从未发起过加载），就不创建
	if (HasLoadingState())
	{
		GetLoadingState().CancelAndDestroy();
	}
}

bool FAsyncMixin::IsAsyncLoadingInProgress() const
{
	if (HasLoadingState())
	{
		return GetLoadingStateConst().IsLoadingInProgress();
	}

	return false;
}

bool FAsyncMixin::IsLoadingInProgressOrPending() const
{
	if (HasLoadingState())
	{
		return GetLoadingStateConst().IsLoadingInProgressOrPending();
	}

	return false;
}

// 所有公开的 Async* 方法都转发到 FLoadingState
void FAsyncMixin::AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncLoad(SoftObjectPath, DelegateToCall);
}

void FAsyncMixin::AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncLoad(SoftObjectPaths, DelegateToCall);
}

void FAsyncMixin::AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, DelegateToCall);
}

void FAsyncMixin::AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback)
{
	GetLoadingState().AsyncCondition(Condition, Callback);
}

void FAsyncMixin::AsyncEvent(const FSimpleDelegate& Callback)
{
	GetLoadingState().AsyncEvent(Callback);
}

// 启动异步加载
void FAsyncMixin::StartAsyncLoading()
{
	// 如果没有任何待加载项，直接触发开始/完成回调
	// 避免无意义地分配 FLoadingState
	if (IsLoadingInProgressOrPending())
	{
		GetLoadingState().Start();
	}
	else
	{
		OnStartedLoading();
		OnFinishedLoading();
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FAsyncMixin::FLoadingState::FLoadingState(FAsyncMixin& InOwner)
	: OwnerRef(InOwner)
{
}

FAsyncMixin::FLoadingState::~FLoadingState()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncMixin_FLoadingState_DestroyThisMemoryDelegate);
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Destroy LoadingState (Done)"), this);

	// 析构时取消所有进行中的步骤和待销毁的定时器
	CancelOnly(/*bDestroying*/true);
	CancelDestroyThisMemory(/*bDestroying*/true);
}

// 仅取消所有步骤，不销毁自身
void FAsyncMixin::FLoadingState::CancelOnly(bool bDestroying)
{
	if (!bDestroying)
	{
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Cancel"), this);
	}

	CancelStartTimer();

	// 逐个取消所有步骤
	for (TUniquePtr<FAsyncStep>& Step : AsyncSteps)
	{
		Step->Cancel();
	}

	// 将步骤移到待销毁列表——避免在回调栈上 Reset 导致崩溃
	AsyncStepsPendingDestruction = MoveTemp(AsyncSteps);

	bPreloadedBundles = false;
	bHasStarted = false;
	CurrentAsyncStep = 0;
}

// 取消并排队销毁
void FAsyncMixin::FLoadingState::CancelAndDestroy()
{
	CancelOnly(/*bDestroying*/false);
	RequestDestroyThisMemory();
}

// 取消延迟销毁定时器
void FAsyncMixin::FLoadingState::CancelDestroyThisMemory(bool bDestroying)
{
	if (IsPendingDestroy())
	{
		if (!bDestroying)
		{
			UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Destroy LoadingState (Canceled)"), this);
		}

		FTSTicker::GetCoreTicker().RemoveTicker(DestroyMemoryDelegate);
		DestroyMemoryDelegate.Reset();
	}
}

// 请求在下一帧销毁加载状态
// 延迟一帧是因为调用者可能还在栈上，直接 Remove 会导致悬垂引用
void FAsyncMixin::FLoadingState::RequestDestroyThisMemory()
{
	if (!IsPendingDestroy())
	{
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Destroy LoadingState (Requested)"), this);

		DestroyMemoryDelegate = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) {
			// 从全局 Map 中移除，释放所有内存
			FAsyncMixin::Loading.Remove(&OwnerRef);
			return false;
		}));
	}
}

// 取消延迟启动定时器
void FAsyncMixin::FLoadingState::CancelStartTimer()
{
	if (StartTimerDelegate.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(StartTimerDelegate);
		StartTimerDelegate.Reset();
	}
}

// 启动异步加载序列
void FAsyncMixin::FLoadingState::Start()
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Start (Current Progress %d/%d)"), this, CurrentAsyncStep + 1, AsyncSteps.Num());

	// 取消可能存在的延迟启动（用户先忘了 Start，Ticker 已安排；用户又手动调了 Start）
	CancelStartTimer();

	if (!bHasStarted)
	{
		bHasStarted = true;
		OwnerRef.OnStartedLoading();
	}

	TryCompleteAsyncLoading();
}

// 异步加载单个软对象路径
void FAsyncMixin::FLoadingState::AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& DelegateToCall)
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] AsyncLoad '%s'"), this, *SoftObjectPath.ToString());

	// 创建异步步骤：包含用户回调 + StreamableHandle
	AsyncSteps.Add(
		MakeUnique<FAsyncStep>(
			DelegateToCall,
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftObjectPath, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT("AsyncMixin"))
			)
	);

	TryScheduleStart();
}

// 异步加载多个软对象路径
void FAsyncMixin::FLoadingState::AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall)
{
	{
		const FString& Paths = FString::JoinBy(SoftObjectPaths, TEXT(", "), [](const FSoftObjectPath& SoftObjectPath) { return FString::Printf(TEXT("'%s'"), *SoftObjectPath.ToString()); });
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] AsyncLoad [%s]"), this, *Paths);
	}

	AsyncSteps.Add(
		MakeUnique<FAsyncStep>(
			DelegateToCall,
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftObjectPaths, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT("AsyncMixin"))
			)
	);

	TryScheduleStart();
}

// 预加载主资产及其 Bundle
void FAsyncMixin::FLoadingState::AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall)
{
	{
		const FString& Assets = FString::JoinBy(AssetIds, TEXT(", "), [](const FPrimaryAssetId& AssetId) { return AssetId.ToString(); });
		const FString& Bundles = FString::JoinBy(LoadBundles, TEXT(", "), [](const FName& LoadBundle) { return LoadBundle.ToString(); });
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X]  AsyncPreload Assets [%s], Bundles[%s]"), this, *Assets, *Bundles);
	}

	TSharedPtr<FStreamableHandle> StreamingHandle;

	if (AssetIds.Num() > 0)
	{
		bPreloadedBundles = true;	// 标记需要保留 StreamableHandle

		const bool bLoadRecursive = true;
		StreamingHandle = UAssetManager::Get().PreloadPrimaryAssets(AssetIds, LoadBundles, bLoadRecursive);
	}

	AsyncSteps.Add(MakeUnique<FAsyncStep>(DelegateToCall, StreamingHandle));

	TryScheduleStart();
}

// 添加异步条件步骤
void FAsyncMixin::FLoadingState::AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& DelegateToCall)
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] AsyncCondition '0x%X'"), this, &Condition.Get());

	AsyncSteps.Add(MakeUnique<FAsyncStep>(DelegateToCall, Condition));

	TryScheduleStart();
}

// 插入纯回调事件
void FAsyncMixin::FLoadingState::AsyncEvent(const FSimpleDelegate& DelegateToCall)
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] AsyncEvent"), this);

	AsyncSteps.Add(MakeUnique<FAsyncStep>(DelegateToCall));

	TryScheduleStart();
}

// 安排下一帧启动（如果用户忘记调用 StartAsyncLoading）
void FAsyncMixin::FLoadingState::TryScheduleStart()
{
	CancelDestroyThisMemory(/*bDestroying*/false);

	// 如果还没有安排启动定时器，安排一个下一帧的
	if (!StartTimerDelegate.IsValid())
	{
		StartTimerDelegate = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) {
			QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncMixin_FLoadingState_TryScheduleStartDelegate);
			Start();
			return false;
		}));
	}
}

bool FAsyncMixin::FLoadingState::IsLoadingInProgress() const
{
	if (AsyncSteps.Num() > 0)
	{
		if (CurrentAsyncStep < AsyncSteps.Num())
		{
			if (CurrentAsyncStep == (AsyncSteps.Num() - 1))
			{
				// 最后一步——直接看它是否还在加载
				return AsyncSteps[CurrentAsyncStep]->IsLoadingInProgress();
			}

			// 不是最后一步，说明中间步骤还有未完成的，仍在加载中
			return true;
		}
	}

	return false;
}

bool FAsyncMixin::FLoadingState::IsLoadingInProgressOrPending() const
{
	return StartTimerDelegate.IsValid() || IsLoadingInProgress();
}

bool FAsyncMixin::FLoadingState::IsPendingDestroy() const
{
	return DestroyMemoryDelegate.IsValid();
}

// 核心推进逻辑：逐个完成异步步骤，执行用户回调
void FAsyncMixin::FLoadingState::TryCompleteAsyncLoading()
{
	// 如果还没 Start 就被回调了，说明已经被取消/完成，直接跳过
	if (!bHasStarted)
	{
		return;
	}

	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] TryCompleteAsyncLoading - (Current Progress %d/%d)"), this, CurrentAsyncStep + 1, AsyncSteps.Num());

	// 逐步推进：当前步骤完成就执行回调并前进一步
	while (CurrentAsyncStep < AsyncSteps.Num())
	{
		FAsyncStep* Step = AsyncSteps[CurrentAsyncStep].Get();
		if (Step->IsLoadingInProgress())
		{
			// 还在加载中——绑定完成回调，等它完成后再推进
			if (!Step->IsCompleteDelegateBound())
			{
				UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Step %d - Still Loading (Listening)"), this, CurrentAsyncStep + 1);
				const bool bBound = Step->BindCompleteDelegate(FSimpleDelegate::CreateSP(this, &FLoadingState::TryCompleteAsyncLoading));
				ensureMsgf(bBound, TEXT("This is not intended to return false.  We're checking if it's loaded above, this should definitely return true."));
			}
			else
			{
				UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Step %d - Still Loading (Waiting)"), this, CurrentAsyncStep + 1);
			}

			break;
		}
		else
		{
			UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] Step %d - Completed (Calling User)"), this, CurrentAsyncStep + 1);

			// 先推进索引再执行回调——回调中可能追加新步骤并再次 Start
			CurrentAsyncStep++;

			Step->ExecuteUserCallback();
		}
	}

	// 所有步骤完成——触发 OnFinishedLoading
	// 注意：bHasStarted 检查防止递归调用时多次触发
	if (IsLoadingComplete() && bHasStarted)
	{
		CompleteAsyncLoading();
	}
}

// 所有加载完成后的收尾
void FAsyncMixin::FLoadingState::CompleteAsyncLoading()
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] CompleteAsyncLoading"), this);

	if (bHasStarted)
	{
		bHasStarted = false;
		OwnerRef.OnFinishedLoading();
	}

	// OnFinishedLoading 回调中可能又追加了新的加载请求，再检查一次
	if (IsLoadingComplete())
	{
		if (!bPreloadedBundles && !IsLoadingInProgressOrPending())
		{
			// 没有预加载 Bundle，也没有其他待处理的——安全释放加载状态
			RequestDestroyThisMemory();
			return;
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FAsyncMixin::FLoadingState::FAsyncStep::FAsyncStep(const FSimpleDelegate& InUserCallback)
	: UserCallback(InUserCallback)
{
}

FAsyncMixin::FLoadingState::FAsyncStep::FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle)
	: UserCallback(InUserCallback)
	, StreamingHandle(InStreamingHandle)
{
}

FAsyncMixin::FLoadingState::FAsyncStep::FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FAsyncCondition>& InCondition)
	: UserCallback(InUserCallback)
	, Condition(InCondition)
{
}

FAsyncMixin::FLoadingState::FAsyncStep::~FAsyncStep()
{

}

// 执行用户回调并解绑（确保只调用一次）
void FAsyncMixin::FLoadingState::FAsyncStep::ExecuteUserCallback()
{
	UserCallback.ExecuteIfBound();
	UserCallback.Unbind();
}

// 步骤是否完成：StreamableHandle 加载完成 / 异步条件满足 / 纯事件（始终完成）
bool FAsyncMixin::FLoadingState::FAsyncStep::IsComplete() const
{
	if (StreamingHandle.IsValid())
	{
		return StreamingHandle->HasLoadCompleted();
	}
	else if (Condition.IsValid())
	{
		return Condition->IsComplete();
	}

	return true;
}

// 取消步骤：释放 Handle/Condition，解绑回调
void FAsyncMixin::FLoadingState::FAsyncStep::Cancel()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->BindCompleteDelegate(FSimpleDelegate());
		StreamingHandle.Reset();
	}
	else if (Condition.IsValid())
	{
		Condition.Reset();
	}

	bIsCompletionDelegateBound = false;
}

// 绑定完成回调——如果已经完成则返回 false（太晚了）
bool FAsyncMixin::FLoadingState::FAsyncStep::BindCompleteDelegate(const FSimpleDelegate& NewDelegate)
{
	if (IsComplete())
	{
		return false;
	}

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->BindCompleteDelegate(NewDelegate);
	}
	else if (Condition)
	{
		Condition->BindCompleteDelegate(NewDelegate);
	}

	bIsCompletionDelegateBound = true;

	return true;
}

bool FAsyncMixin::FLoadingState::FAsyncStep::IsCompleteDelegateBound() const
{
	return bIsCompletionDelegateBound;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FAsyncCondition::FAsyncCondition(const FAsyncConditionDelegate& Condition)
	: UserCondition(Condition)
{
}

FAsyncCondition::FAsyncCondition(TFunction<EAsyncConditionResult()>&& Condition)
	: UserCondition(FAsyncConditionDelegate::CreateLambda([UserFunction = MoveTemp(Condition)]() mutable { return UserFunction(); }))
{
}

FAsyncCondition::~FAsyncCondition()
{
	FTSTicker::GetCoreTicker().RemoveTicker(RepeatHandle);
}

// 检查条件是否满足
bool FAsyncCondition::IsComplete() const
{
	if (UserCondition.IsBound())
	{
		const EAsyncConditionResult Result = UserCondition.Execute();
		return Result == EAsyncConditionResult::Complete;
	}

	return true;
}

// 绑定条件满足后的回调——同时启动轮询定时器
bool FAsyncCondition::BindCompleteDelegate(const FSimpleDelegate& NewDelegate)
{
	if (IsComplete())
	{
		return false;
	}

	CompletionDelegate = NewDelegate;

	// 以 0.16 秒间隔轮询条件（~6Hz）
	if (!RepeatHandle.IsValid())
	{
		RepeatHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(this, &FAsyncCondition::TryToContinue), 0.16);
	}

	return true;
}

// Ticker 轮询回调——检查条件是否满足，满足则触发完成回调
bool FAsyncCondition::TryToContinue(float)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncCondition_TryToContinue);

	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%X] AsyncCondition::TryToContinue"), this);

	if (UserCondition.IsBound())
	{
		const EAsyncConditionResult Result = UserCondition.Execute();

		switch (Result)
		{
		case EAsyncConditionResult::TryAgain:
			return true;	// 继续轮询
		case EAsyncConditionResult::Complete:
			RepeatHandle.Reset();
			UserCondition.Unbind();

			CompletionDelegate.ExecuteIfBound();
			CompletionDelegate.Unbind();
			break;
		}
	}

	return false;
}
