#include "AuraSummonAbility.h"

#include "Aura/Character/Component/SummonComponent.h"

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
			Avatar->FinishAddComponent(NewComponent, false, FTransform::Identity);
		}
	}

	// 이 Ability가 부여될 때, 대상에게 StackableAbilityComponent를 붙여줍니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		// 이 Ability의 충전 타이머를 등록합니다.
		Comp->RegisterAbility(GetAssetTags().First(), StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
	}
}

bool UAuraSummonAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 소환 가능한 하수인 수를 계산합니다.
	int32 SpawnableCount = 0;
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (USummonComponent* SummonComponent = Avatar->FindComponentByClass<USummonComponent>())
		{
			SpawnableCount = SummonComponent->SpawnableSummonMinionCount;
			SpawnableCount = FMath::Min(SpawnableCount, NumMinions);
		}
	}

	// 소환 가능한 하수인이 없다면 false를 반환합니다.
	if (SpawnableCount <= 0)
	{
		return false;
	}

	// 충전된 스택이 없다면 false를 반환합니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		if (!Comp->CheckCost(GetAssetTags().First()))
		{
			return false;
		}
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UAuraSummonAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 충전된 스택을 소모합니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		if (ActorInfo->IsNetAuthority())
		{
			Comp->ApplyCost(GetAssetTags().First());
		}
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UAuraSummonAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// 이 Ability가 제거될 때, 이 Ability의 충전 타이머를 제거합니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		Comp->UnregisterAbility(GetAssetTags().First());
	}
	
	Super::OnRemoveAbility(ActorInfo, Spec);
}

UStackableAbilityComponent* UAuraSummonAbility::GetStackableAbilityComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	// 이미 StackableAbilityComponent가 있다면 그 컴포넌트에 이 Ability를 등록하고, 없다면 직접 스폰 후 붙여줍니다.
	if (AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : GetAvatarActorFromActorInfo())
	{
		UStackableAbilityComponent* Comp = AvatarActor->FindComponentByClass<UStackableAbilityComponent>();
		if (Comp)
		{
			return Comp;
		}

		// Component가 없는 경우 여기로 내려옵니다. OnGiveAbility를 통해 들어온 경우에만 작동되는 구문입니다.
		Comp = Cast<UStackableAbilityComponent>(AvatarActor->AddComponentByClass(UStackableAbilityComponent::StaticClass(), false, FTransform::Identity, true));
		AvatarActor->FinishAddComponent(Comp, false, FTransform::Identity);
		return Comp;
	}

	return nullptr;
}
