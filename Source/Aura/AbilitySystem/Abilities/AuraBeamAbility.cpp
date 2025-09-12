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
	
	// Trace 결과 부딪힌 액터가 있다면 해당 액터를, 없다면 처음 Mouse 기준으로 Trace했던 결과를 그대로 반환합니다.
	if (HitResult.bBlockingHit)
	{
		TargetHitActor = HitResult.GetActor();
	}
	else
	{
		TargetHitActor = MouseHitActor;
	}
}

void UAuraBeamAbility::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;
	FVector SphereOrigin;
	ActorsToIgnore.Add(TargetHitActor);
	if (TargetHitActor->Implements<UEnemyInterface>())
	{
		ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
		SphereOrigin = TargetHitActor->GetActorLocation();
	}
	else
	{
		SphereOrigin = MouseHitLocation;
	}
	
	UAuraAbilitySystemLibrary::GetOverlappedLivePawnsWithinRadius(GetAvatarActorFromActorInfo(), OverlappingActors, ActorsToIgnore, SplashRadius, SphereOrigin);
	UAuraAbilitySystemLibrary::GetClosestTargets(MaxAdditionalHitTargets, OverlappingActors, OutAdditionalTargets, SphereOrigin);
}
