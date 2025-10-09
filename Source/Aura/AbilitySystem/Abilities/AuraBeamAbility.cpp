#include "AuraBeamAbility.h"

#include "Aura/Aura.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Character/AuraCharacterBase.h"
#include "Aura/Interaction/EnemyInterface.h"

void UAuraBeamAbility::TraceFirstTarget()
{
	check(OwnerCharacter);

	// SphereTrace를 위한 변수들을 선언 및 Trace해 결과를 가져옵니다.
	FHitResult HitResult;
	
	FCollisionQueryParams Params(FName("TraceFirstTarget"), false, OwnerCharacter);
	Params.AddIgnoredActor(OwnerCharacter);
	const FVector TraceStartLocation = ICombatInterface::Execute_GetCombatSocketLocation(OwnerCharacter, FName("TipSocket"), true);
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, MouseHitLocation, ECC_Target, Params);
	
	// Trace 결과가 Enemy라면 해당 액터를 할당하고 return합니다.
	if (HitResult.bBlockingHit && HitResult.GetActor() && HitResult.GetActor()->Implements<UEnemyInterface>())
	{
		TargetHitActor = HitResult.GetActor();
	}
	else
	{
		TargetHitActor = MouseHitActor;
	}
}

bool UAuraBeamAbility::CheckRange()
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !TargetHitActor)
	{
		return false;
	}

	FVector TargetLocation;
	TargetHitActor->Implements<UEnemyInterface>() ? TargetLocation = TargetHitActor->GetActorLocation() : TargetLocation = MouseHitLocation;
	
	if (FVector::DistSquared(AvatarActor->GetActorLocation(), TargetLocation) <= FMath::Square(BeamRange))
	{
		return true;
	}
	
	return false;
}

void UAuraBeamAbility::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	int32 NumOfHitTargets = MaxHitTargets;
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;
	FVector SphereOrigin;
	ActorsToIgnore.Add(TargetHitActor);
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	if (TargetHitActor->Implements<UEnemyInterface>())
	{
		// 첫 적중 대상이 Enemy인 경우 MaxHitTargets - 1 만큼 추가 대상을 지정합니다.
		NumOfHitTargets--;
		SphereOrigin = TargetHitActor->GetActorLocation();
	}
	else
	{
		SphereOrigin = MouseHitLocation;
	}

	// 첫 적중 위치에서 가장 가까운 적을 NumOfHitTarget만큼 지정, OutAdditionalTargets에 채웁니다.
	UAuraAbilitySystemLibrary::GetOverlappedLivePawnsWithinRadius(GetAvatarActorFromActorInfo(), OverlappingActors, ActorsToIgnore, SplashRadius, SphereOrigin);
	UAuraAbilitySystemLibrary::GetClosestTargets(NumOfHitTargets, OverlappingActors, OutAdditionalTargets, SphereOrigin);
}
