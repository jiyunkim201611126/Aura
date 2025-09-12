#include "AuraChannelingAbility.h"

#include "Aura/Character/AuraCharacterBase.h"

void UAuraChannelingAbility::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		// Trace 결과가 아무것도 없다면 Ability 발동을 취소합니다.
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
	
	if (CurrentActorInfo)
	{
		OwnerCharacter = Cast<AAuraCharacterBase>(CurrentActorInfo->AvatarActor);
	}
}

void UAuraChannelingAbility::PlayLoopAnimMontage()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->MulticastPlayLoopAnimMontage(LoopAnimMontage);
	}
}

void UAuraChannelingAbility::StopLoopAnimMontage()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->MulticastStopLoopAnimMontage(LoopAnimMontage);
	}
}
