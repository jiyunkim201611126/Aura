#include "AuraSummonAbility.h"

#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Character/AuraEnemy.h"
#include "Aura/Character/Component/SummonComponent.h"
#include "Kismet/KismetMathLibrary.h"

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass() const
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}

TArray<FVector_NetQuantize> UAuraSummonAbility::GetSpawnLocations()
{
	int32 SpawnNum = NumMinions;
	if (const AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (const USummonComponent* SummonComponent = Avatar->FindComponentByClass<USummonComponent>())
		{
			SpawnNum = FMath::Min(NumMinions, SummonComponent->MaxSummonMinionCount);
		}
	}
	
	// 소환 위치를 지정합니다.
	TArray<FVector_NetQuantize> SpawnLocations;
	SpawnLocations.Reserve(SpawnNum);
	if (SpawnNum > 0)
	{
		const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
		const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();

		// 소환 가능한 수만큼 부채꼴로 펼칩니다.
		TArray<FVector> Directions = UAuraAbilitySystemLibrary::EvenlyRotatedVectors(Forward, FVector::UpVector, SpawnSpread, SpawnNum);
		
		for (const FVector& Direction : Directions)
		{
			// 부채꼴 모양으로 펼쳐 허공에 점을 찍고, 그 위치에서 바닥으로 라인트레이스해 바닥을 검출해냅니다.
			FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

			FHitResult Hit;
			GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation, ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
			if (Hit.bBlockingHit)
			{
				// 바닥보다 살짝 위를 지정합니다.
				ChosenSpawnLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 30.f);
			}
			
			SpawnLocations.Add(ChosenSpawnLocation);
		}
	}

	return SpawnLocations;
}

void UAuraSummonAbility::SpawnNiagaras(const TArray<FVector_NetQuantize>& SpawnLocations)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		// GC에서 사용하기 위한 Location을 할당합니다.
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		UAuraAbilitySystemLibrary::SetLocationsToContext(ContextHandle, SpawnLocations);
		
		FGameplayTagContainer NiagaraTag;
		NiagaraTag.AddTag(SpawnNiagaraTag);

		FGameplayCueParameters CueParams;
		CueParams.EffectContext = ContextHandle;
		CueParams.AggregatedSourceTags = NiagaraTag;
		
		const FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Common.NiagaraWithLocationArray"));
		ASC->ExecuteGameplayCue(CueTag, CueParams);
	}
}

void UAuraSummonAbility::SpawnMinion(const FVector_NetQuantize& SpawnLocation)
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor)
	{
		if (APawn* SpawnedMinion = GetWorld()->SpawnActorDeferred<APawn>(GetRandomMinionClass(), FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
		{
			// 스폰할 Transform 세팅
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SpawnLocation);
			const FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(AvatarActor->GetActorLocation(), SpawnLocation);
			SpawnTransform.SetRotation(SpawnRotation.Quaternion());

			// AI 컨트롤러를 할당합니다.
			SpawnedMinion->SpawnDefaultController();
			if (USummonComponent* SummonComponent = AvatarActor->GetComponentByClass<USummonComponent>())
			{
				SummonComponent->AddMinion(SpawnedMinion);
			}
			
			SpawnedMinion->FinishSpawning(SpawnTransform);
			
			// 클라이언트에게 SpawnAnimation이 재생될 수 있도록 설정해줍니다.
			// Client RPC를 사용할 경우, 클라이언트에게 하수인이 스폰되지 않았을 때 호출해버려 애니메이션이 재생되지 않을 수 있습니다.
			// 따라서 Replicate 기반으로 작동하는 로직을 작성했습니다.
			if (IEnemyInterface* Enemy = Cast<IEnemyInterface>(SpawnedMinion))
			{
				Enemy->ShouldPlaySpawnAnimation();
			}
			
			// 위 함수는 Replicate 기반으로 작동하므로, 서버도 SpawnAnimation이 재생되도록 직접 호출합니다.
			IEnemyInterface::Execute_PlaySpawnAnimation(SpawnedMinion);
		}
	}
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
