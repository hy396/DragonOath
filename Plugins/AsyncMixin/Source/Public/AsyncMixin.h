// Copyright Epic Games, Inc. All Rights Reserved.
// DragonOath 中文注释：异步加载生命周期辅助，用来串联和取消异步加载请求，避免对象销毁后回调失效。

#pragma once

#include "Containers/Ticker.h"
#include "UObject/SoftObjectPtr.h"

class FAsyncCondition;
class FName;
class UPrimaryDataAsset;
struct FPrimaryAssetId;
struct FStreamableHandle;
template <class TClass> class TSubclassOf;

/** 流式加载完成的回调委托，参数为 StreamableHandle */
DECLARE_DELEGATE_OneParam(FStreamableHandleDelegate, TSharedPtr<FStreamableHandle>)

//TODO 考虑引入"保留策略"（Retention Policy）：预加载资源默认驻留内存直到取消，
//     但如果只是用 AsyncLoad 加载单个资源呢？不希望每次调用都带策略参数，
//     也不希望搞一套 preload vs asyncload 的分裂 API，所以更倾向于
//     统一的保留策略。它应该是 FAsyncMixin 的成员（真的分配内存），
//     还是模板参数？
//enum class EAsyncMixinRetentionPolicy : uint8
//{
//	Default,
//	KeepResidentUntilComplete,
//	KeepResidentUntilCancel
//};

/**
 * 异步加载混入类（Async Mix-in）
 *
 * 简化异步加载请求的管理，保证线性回调顺序，让异步代码写起来像同步一样简单。
 *
 * ## 用法
 *
 * 1. 让你的类继承 FAsyncMixin（UObject 也可以多继承它）
 *
 * 2. 按如下模式发起异步加载：
 * ```cpp
 * CancelAsyncLoading();              // 列表控件等可复用对象，先取消上一次未完成的请求
 * AsyncLoad(ItemOne, CallbackOne);
 * AsyncLoad(ItemTwo, CallbackTwo);
 * StartAsyncLoading();
 * ```
 *
 * 3. 回调中可以安全地捕获 `this`——FAsyncMixin 保证了：
 *    - 宿主对象销毁时自动解绑所有回调
 *    - 调用 CancelAsyncLoading() 后也不会收到已取消的回调
 *
 * ## 回调顺序保证
 *
 * 即使 ItemOne 已经在内存中，CallbackOne 也**一定**在 CallbackTwo 之前执行——
 * 回调严格按 AsyncLoad 调用顺序触发。
 *
 * ## StartAsyncLoading()
 *
 * 务必在所有 AsyncLoad 之后调用。如果忘记调用，下一帧会自动触发，
 * 但这会导致一帧的加载指示器闪烁。如果所有资源已在内存中，
 * 立即调用可避免闪烁。
 *
 * ## 内存零开销
 *
 * FAsyncMixin 本身不占用额外内存。内部使用静态 TMap 管理所有加载状态，
 * 只在需要时分配，完成后立即释放。
 *
 * ## 调试
 *
 * 命令行添加 `-LogCmds="LogAsyncMixin Verbose"` 可查看详细日志。
 */
class ASYNCMIXIN_API FAsyncMixin : public FNoncopyable
{
protected:
	FAsyncMixin();

public:
	virtual ~FAsyncMixin();

protected:
	/** 加载开始时的回调，子类可重写 */
	virtual void OnStartedLoading() { }
	/** 所有加载完成后的回调，子类可重写 */
	virtual void OnFinishedLoading() { }

protected:
	/** 异步加载软类引用，完成后调用 Callback */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载软类引用，完成后将解析出的子类传给 Callback */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void(TSubclassOf<T>)>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(),
			FSimpleDelegate::CreateLambda([SoftClass, UserCallback = MoveTemp(Callback)]() mutable {
				UserCallback(SoftClass.Get());
			})
		);
	}

	/** 异步加载软类引用，完成后调用 Callback（无额外参数） */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), Callback);
	}

	/** 异步加载软对象引用，完成后调用 Callback */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载软对象引用，完成后将解析出的对象指针传给 Callback */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void(T*)>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(),
			FSimpleDelegate::CreateLambda([SoftObject, UserCallback = MoveTemp(Callback)]() mutable {
				UserCallback(SoftObject.Get());
			})
		);
	}

	/** 异步加载软对象引用，完成后调用 Callback（无额外参数） */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), Callback);
	}

	/** 异步加载单个软对象路径，完成后调用 Callback */
	void AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 异步加载多个软对象路径，完成后调用 Callback */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObjectPaths, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载多个软对象路径，完成后调用 Callback */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& Callback = FSimpleDelegate());

	/**
	 * 预加载主资产（Primary Asset）及其引用的 Bundle
	 * @param Assets     主资产指针数组
	 * @param LoadBundles 要加载的 Bundle 名称列表
	 * @param Callback    全部加载完成后的回调
	 */
	template<typename T = UPrimaryDataAsset>
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<T*>& Assets, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		TArray<FPrimaryAssetId> PrimaryAssetIds;
		for (const T* Item : Assets)
		{
			PrimaryAssetIds.Add(Item);
		}

		AsyncPreloadPrimaryAssetsAndBundles(PrimaryAssetIds, LoadBundles, Callback);
	}

	/** 预加载主资产及其 Bundle（TFunction 回调版本） */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, TFunction<void()>&& Callback)
	{
		AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/**
	 * 预加载主资产及其引用的 Bundle
	 * @param AssetIds    主资产 ID 数组
	 * @param LoadBundles 要加载的 Bundle 名称列表
	 * @param Callback    全部加载完成后的回调
	 */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate());

	/**
	 * 添加一个异步条件——条件满足前不会推进后续步骤
	 * @param Condition 异步条件对象
	 * @param Callback  条件满足后的回调
	 */
	void AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback = FSimpleDelegate());

	/**
	 * 在异步加载序列中插入一个纯回调事件（不加载任何资源）
	 *
	 * 适合不需要关联特定资源、但需要在序列中某个位置执行逻辑的场景，
	 * 例如某些资源是可选的，不需要绑定到 StreamableHandle。
	 */
	void AsyncEvent(TFunction<void()>&& Callback)
	{
		AsyncEvent(FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/**
	 * 在异步加载序列中插入一个纯回调事件（FSimpleDelegate 版本）
	 *
	 * 同上，只是使用 FSimpleDelegate 而非 TFunction。
	 */
	void AsyncEvent(const FSimpleDelegate& Callback);

	/** 启动异步加载——在所有 AsyncLoad 注册完毕后调用 */
	void StartAsyncLoading();

	/** 取消所有未完成的异步加载请求 */
	void CancelAsyncLoading();

	/** 当前是否正在异步加载中？ */
	bool IsAsyncLoadingInProgress() const;

private:
	/**
	 * 加载状态（FLoadingState）
	 *
	 * FAsyncMixin 本身零内存——真正的加载状态都存在一个静态 TMap 里，
	 * 只在需要时分配，完成后释放。
	 */
	class FLoadingState : public TSharedFromThis<FLoadingState>
	{
	public:
		FLoadingState(FAsyncMixin& InOwner);
		virtual ~FLoadingState();

		/** 启动异步加载序列 */
		void Start();

		/** 取消异步加载并销毁加载状态 */
		void CancelAndDestroy();

		/** 异步加载单个软对象路径 */
		void AsyncLoad(FSoftObjectPath SoftObject, const FSimpleDelegate& DelegateToCall);
		/** 异步加载多个软对象路径 */
		void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall);
		/** 预加载主资产及其 Bundle */
		void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& PrimaryAssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall);
		/** 添加异步条件 */
		void AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback);
		/** 插入纯回调事件 */
		void AsyncEvent(const FSimpleDelegate& Callback);

		/** 所有步骤是否已加载完成 */
		bool IsLoadingComplete() const { return !IsLoadingInProgress(); }
		/** 当前是否正在加载中 */
		bool IsLoadingInProgress() const;
		/** 是否正在加载或等待启动 */
		bool IsLoadingInProgressOrPending() const;
		/** 是否已排队等待销毁 */
		bool IsPendingDestroy() const;

	private:
		/** 仅取消不销毁（内部使用） */
		void CancelOnly(bool bDestroying);
		/** 取消延迟启动定时器 */
		void CancelStartTimer();
		/** 尝试安排下一次启动 */
		void TryScheduleStart();
		/** 尝试推进并完成异步加载 */
		void TryCompleteAsyncLoading();
		/** 所有加载完成后的收尾 */
		void CompleteAsyncLoading();

	private:
		/** 请求在下一帧销毁加载状态（延迟销毁，避免栈上回调崩溃） */
		void RequestDestroyThisMemory();
		/** 取消延迟销毁请求 */
		void CancelDestroyThisMemory(bool bDestroying);

		/** 拥有此加载状态的 FAsyncMixin 宿主对象引用 */
		FAsyncMixin& OwnerRef;

		/**
		 * 是否预加载了 Bundle——预加载 Bundle 需要持有 StreamableHandle，
		 * 否则资源会被卸载；如果没预加载 Bundle，加载完成后可安全释放内存
		 */
		bool bPreloadedBundles = false;

		/**
		 * 异步步骤（FAsyncStep）
		 *
		 * 每次调用 AsyncLoad / AsyncCondition / AsyncEvent 都会创建一个步骤，
		 * 按顺序执行：当前步骤完成后才推进到下一步。
		 */
		class FAsyncStep
		{
		public:
			FAsyncStep(const FSimpleDelegate& InUserCallback);
			FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle);
			FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FAsyncCondition>& InCondition);

			~FAsyncStep();

			/** 执行用户回调 */
			void ExecuteUserCallback();

			/** 该步骤是否仍在加载中 */
			bool IsLoadingInProgress() const
			{
				return !IsComplete();
			}

			/** 该步骤是否已完成（加载完成 或 条件满足 或 纯事件） */
			bool IsComplete() const;
			/** 取消该步骤 */
			void Cancel();

			/** 绑定完成回调，返回是否绑定成功（已完成的步骤返回 false） */
			bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);
			/** 完成回调是否已绑定 */
			bool IsCompleteDelegateBound() const;

		private:
			FSimpleDelegate UserCallback;				// 用户注册的回调
			bool bIsCompletionDelegateBound = false;		// 完成回调是否已绑定

			// 异步"东西"——三选一：流式加载句柄 / 异步条件 / 纯事件（两者都无效）
			TSharedPtr<FStreamableHandle> StreamingHandle;	// 流式加载句柄
			TSharedPtr<FAsyncCondition> Condition;			// 异步条件
		};

		bool bHasStarted = false;					// 是否已开始加载序列
		int32 CurrentAsyncStep = 0;					// 当前正在执行的步骤索引
		TArray<TUniquePtr<FAsyncStep>> AsyncSteps;			// 异步步骤列表
		TArray<TUniquePtr<FAsyncStep>> AsyncStepsPendingDestruction;	// 等待销毁的步骤（避免回调中 Reset 崩溃）

		FTSTicker::FDelegateHandle StartTimerDelegate;		// 延迟启动定时器句柄
		FTSTicker::FDelegateHandle DestroyMemoryDelegate;	// 延迟销毁定时器句柄
	};

	/** 获取只读的加载状态（必须已存在） */
	const FLoadingState& GetLoadingStateConst() const;

	/** 获取可写的加载状态（不存在则自动创建） */
	FLoadingState& GetLoadingState();

	/** 是否已有加载状态 */
	bool HasLoadingState() const;

	/** 是否正在加载或等待启动 */
	bool IsLoadingInProgressOrPending() const;

private:
	/** 全局加载状态映射——FAsyncMixin* → FLoadingState，实现零成员变量开销 */
	static TMap<FAsyncMixin*, TSharedRef<FLoadingState>> Loading;
};

/**
 * 异步作用域（Async Scope）
 *
 * 有时候"混入"方式不合适——比如一个对象需要管理多个独立的异步任务链，
 * 每条链有自己的依赖和生命周期。这时可以用 FAsyncScope：
 *
 * 它是一个独立的异步依赖处理器，可以像 FAsyncMixin 一样发起加载、
 * 保证回调顺序，但不需要继承，直接作为成员变量使用。
 */
class ASYNCMIXIN_API FAsyncScope : public FAsyncMixin
{
public:
	using FAsyncMixin::AsyncLoad;

	using FAsyncMixin::AsyncPreloadPrimaryAssetsAndBundles;

	using FAsyncMixin::AsyncCondition;

	using FAsyncMixin::AsyncEvent;

	using FAsyncMixin::CancelAsyncLoading;

	using FAsyncMixin::StartAsyncLoading;

	using FAsyncMixin::IsAsyncLoadingInProgress;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/**
 * 异步条件轮询结果
 */
enum class EAsyncConditionResult : uint8
{
	TryAgain,	// 条件尚未满足，继续轮询
	Complete,	// 条件已满足，推进下一步
};

/** 异步条件委托——返回当前条件是否满足 */
DECLARE_DELEGATE_RetVal(EAsyncConditionResult, FAsyncConditionDelegate);

/**
 * 异步条件（Async Condition）
 *
 * 允许在异步加载序列中插入自定义的阻塞条件，
 * 条件满足前不会推进后续步骤。内部使用 Ticker 轮询检查条件。
 */
class FAsyncCondition : public TSharedFromThis<FAsyncCondition>
{
public:
	FAsyncCondition(const FAsyncConditionDelegate& Condition);
	FAsyncCondition(TFunction<EAsyncConditionResult()>&& Condition);
	virtual ~FAsyncCondition();

protected:
	/** 条件是否已满足 */
	bool IsComplete() const;
	/** 绑定条件满足后的回调，返回是否绑定成功 */
	bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);

private:
	/** Ticker 轮询回调——检查条件是否满足 */
	bool TryToContinue(float DeltaTime);

	FTSTicker::FDelegateHandle RepeatHandle;	// 轮询定时器句柄
	FAsyncConditionDelegate UserCondition;		// 用户提供的条件判断函数
	FSimpleDelegate CompletionDelegate;			// 条件满足后的回调

	friend FAsyncMixin;
};
