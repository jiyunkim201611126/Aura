#include "AuraSummonAbility.h"

#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();

	// 스폰 지점을 정면 기준으로 부채꼴 모양으로 펼칩니다.
	TArray<FVector> SpawnLocations;
	if (NumMinions > 0)
	{
		float DeltaSpread = NumMinions > 1 ? SpawnSpread / (NumMinions - 1) : 0.f;

		const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
		for (int32 i = 0; i < NumMinions; i++)
		{
			const FVector Direction = NumMinions > 1 ? LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector) : Forward;
			FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

			FHitResult Hit;
			GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation, ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
			if (Hit.bBlockingHit)
			{
				ChosenSpawnLocation = Hit.ImpactPoint;
			}
			
			SpawnLocations.Add(ChosenSpawnLocation);
		}
	}

	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass() const
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}
