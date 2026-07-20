


#include "Tasks/AbilityTasks/AbilityTask_WaitMontageNotifys.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"


UTask_WaitMontageNotifys* UTask_WaitMontageNotifys::WaitMontageNotifys( UGameplayAbility* OwningAbility )
{
	UTask_WaitMontageNotifys* MyObj = NewAbilityTask<UTask_WaitMontageNotifys>( OwningAbility );
	return MyObj;
}

void UTask_WaitMontageNotifys::Activate()
{
	if ( Ability == nullptr ) return;

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

	if ( AnimInstance ) {
		
		if ( FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage( AbilitySystemComponent->GetCurrentMontage() ) ) 
		{
			MontageInstanceID = MontageInstance->GetInstanceID();
		}

		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this , &ThisClass::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic( this , &ThisClass::OnNotifyEndReceived );
	}
}

void UTask_WaitMontageNotifys::OnDestroy( bool bInOwnerFinished )
{
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if ( AnimInstance ) {
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic( this , &ThisClass::OnNotifyBeginReceived );
		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic( this , &ThisClass::OnNotifyEndReceived );
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

bool UTask_WaitMontageNotifys::IsNotifyValid( FName NotifyName , const FBranchingPointNotifyPayload& BranchingPointNotifyPayload ) const
{
	return ( ( MontageInstanceID != INDEX_NONE ) && ( BranchingPointNotifyPayload.MontageInstanceID == MontageInstanceID ) );
}

void UTask_WaitMontageNotifys::OnNotifyBeginReceived( FName NotifyName , const FBranchingPointNotifyPayload& BranchingPointNotifyPayload )
{
	if ( IsNotifyValid( NotifyName , BranchingPointNotifyPayload ) ) {
		OnNotifyBegin.Broadcast( NotifyName );
	}
}

void UTask_WaitMontageNotifys::OnNotifyEndReceived( FName NotifyName , const FBranchingPointNotifyPayload& BranchingPointNotifyPayload )
{
	if ( IsNotifyValid( NotifyName , BranchingPointNotifyPayload ) ) {
		OnNotifyEnd.Broadcast( NotifyName );
	}
}