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
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UAuraChannelingAbility::StoreOwnerVariables()
{
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
