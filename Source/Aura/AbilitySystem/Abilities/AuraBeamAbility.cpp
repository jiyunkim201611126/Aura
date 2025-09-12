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
	int32 NumOfHitTargets = MaxAdditionalHitTargets;
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
		// 첫 적중 대상이 Enemy가 아닌 경우(벽이나 바닥일 때) 총 적중 대상이 일치할 수 있도록 Beam을 하나 더 생성합니다.
		NumOfHitTargets++;
		SphereOrigin = MouseHitLocation;
	}
	
	UAuraAbilitySystemLibrary::GetOverlappedLivePawnsWithinRadius(GetAvatarActorFromActorInfo(), OverlappingActors, ActorsToIgnore, SplashRadius, SphereOrigin);
	UAuraAbilitySystemLibrary::GetClosestTargets(NumOfHitTargets, OverlappingActors, OutAdditionalTargets, SphereOrigin);
}
