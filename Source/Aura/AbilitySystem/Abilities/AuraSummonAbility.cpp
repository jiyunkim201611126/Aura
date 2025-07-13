#include "AuraSummonAbility.h"

#include "Aura/Character/Component/SummonComponent.h"

void UAuraSummonAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 이 Ability가 부여될 때, 대상에게 SummonComponent를 붙여줍니다.
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	if (Avatar)
	{
		if (!Avatar->FindComponentByClass<USummonComponent>())
		{
			USummonComponent* NewComponent = Cast<USummonComponent>(Avatar->AddComponentByClass(USummonComponent::StaticClass(), false, FTransform::Identity, true));
			if (NewComponent)
			{
				NewComponent->SpawnableSummonMinionCount = MaxMinionCount;
			}

			Avatar->FinishAddComponent(NewComponent, false, FTransform::Identity);
		}
	}
}

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();

	// 스폰 가능한 하수인 수를 계산합니다.
	int32 SpawnableCount = 0;
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (USummonComponent* SummonComponent = Avatar->FindComponentByClass<USummonComponent>())
		{
			SpawnableCount = SummonComponent->SpawnableSummonMinionCount;
			SpawnableCount = FMath::Min(SpawnableCount, NumMinions);
		}
	}
	
	// 스폰 지점을 정면 기준으로 부채꼴 모양으로 펼칩니다.
	TArray<FVector> SpawnLocations;
	if (SpawnableCount > 0)
	{
		float DeltaSpread = SpawnableCount > 1 ? SpawnSpread / (SpawnableCount - 1) : 0.f;

		const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
		for (int32 i = 0; i < SpawnableCount; i++)
		{
			const FVector Direction = SpawnableCount > 1 ? LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector) : Forward;
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
