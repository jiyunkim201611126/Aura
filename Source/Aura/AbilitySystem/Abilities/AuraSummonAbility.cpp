#include "AuraSummonAbility.h"

#include "Aura/Character/Component/SummonComponent.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	
	// 소환 위치를 지정합니다.
	TArray<FVector> SpawnLocations;
	if (NumMinions > 0)
	{
		// 소환 가능한 수만큼 부채꼴 모양으로 펼칩니다.
		const float DeltaSpread = NumMinions > 1 ? SpawnSpread / (NumMinions - 1) : 0.f;

		const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
		for (int32 i = 0; i < NumMinions; i++)
		{
			const FVector Direction = NumMinions > 1 ? LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector) : Forward;
			FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

			// 부채꼴 모양으로 펼쳐 허공에 점을 찍고, 그 위치에서 바닥으로 라인트레이스해 바닥을 검출해냅니다.
			FHitResult Hit;
			GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation, ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
			if (Hit.bBlockingHit)
			{
				// 바닥보다 살짝 위를 지정합니다.
				ChosenSpawnLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 10.f);
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

void UAuraSummonAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 이 Ability가 부여될 때, 대상에게 SummonComponent를 붙여줍니다.
	if (AActor* Avatar = ActorInfo->AvatarActor.Get())
	{
		if (!Avatar->FindComponentByClass<USummonComponent>())
		{
			USummonComponent* NewComponent = Cast<USummonComponent>(Avatar->AddComponentByClass(USummonComponent::StaticClass(), false, FTransform::Identity, true));
			Avatar->FinishAddComponent(NewComponent, false, FTransform::Identity);
		}
	}
}

bool UAuraSummonAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const AActor* Avatar = ActorInfo->AvatarActor.Get())
	{
		if (const USummonComponent* SummonComponent = Avatar->FindComponentByClass<USummonComponent>())
		{
			if (!SummonComponent->CanSummon())
			{
				return false;
			}
		}
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}
