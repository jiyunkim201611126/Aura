#include "AuraProjectileSpell.h"

#include "Aura/Actor/AuraProjectile.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Interaction/EnemyInterface.h"

int32 UAuraProjectileSpell::GetProjectileNumsToSpawn(const int32 Level) const
{
	return NumOfProjectiles.GetValueAtLevel(Level);
}

void UAuraProjectileSpell::SetTarget(const FGameplayAbilityTargetDataHandle& Handle)
{
	const FHitResult* HitResult = Handle.Get(0)->GetHitResult();
	if (AActor* HitActor = HitResult->GetActor())
	{
		if (HitActor->Implements<UEnemyInterface>())
		{
			ProjectileTargetLocation = HitActor->GetActorLocation();
			HomingTarget = HitActor;
		}
		else
		{
			ProjectileTargetLocation = HitResult->Location;
			HomingTarget = nullptr;
		}
	}
}

TArray<AAuraProjectile*> UAuraProjectileSpell::SpawnProjectile(FVector& InProjectileSpawnLocation, FVector& InProjectileTargetLocation, const float PitchOverride)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return TArray<AAuraProjectile*>();
	}

	// 스폰할 Projectile의 개수를 가져옵니다.
	const int32 NumProjectilesToSpawn = GetProjectileNumsToSpawn(GetAbilityLevel());
	
	// Projectile이 스폰될 위치와 날아갈 방향을 결정합니다.
	const FVector Forward = InProjectileTargetLocation - InProjectileSpawnLocation;
	
	// ProjectileSpread가 중심각이 되는 부채꼴 모양으로 퍼지도록 계산합니다.
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectilesToSpawn);
	if (PitchOverride > 0.f)
	{
		for (auto& Rotation : Rotations)
		{
			Rotation.Pitch += PitchOverride;
		}
	}

	return SpawnProjectile(NumProjectilesToSpawn, InProjectileSpawnLocation, InProjectileTargetLocation, Rotations);
}

TArray<AAuraProjectile*> UAuraProjectileSpell::SpawnProjectileWithCircle(FVector& InProjectileSpawnLocation, FVector& InProjectileTargetLocation, const float PitchOverride)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return TArray<AAuraProjectile*>();
	}

	const int32 NumProjectilesToSpawn = GetProjectileNumsToSpawn(GetAbilityLevel());
	
	const FVector Forward = InProjectileTargetLocation - InProjectileSpawnLocation;
	
	// 투사체가 360도 내에서 균일하게 퍼지도록 Rotation을 계산합니다.
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotatorsWithCircle(Forward, FVector::UpVector, NumProjectilesToSpawn);
	if (PitchOverride > 0.f)
	{
		for (auto& Rotation : Rotations)
		{
			Rotation.Pitch += PitchOverride;
		}
	}
	
	return SpawnProjectile(NumProjectilesToSpawn, InProjectileSpawnLocation, InProjectileTargetLocation, Rotations);
}

TArray<AAuraProjectile*> UAuraProjectileSpell::SpawnProjectile(int32 NumProjectilesToSpawn, const FVector& InProjectileSpawnLocation, const FVector& InProjectileTargetLocation, const TArray<FRotator>& Rotations)
{
	// SpawnActor는 생성 직후 BeginPlay까지 호출하지만 SpawnActorDeferred는 구성만 하고 생성은 대기합니다.
	TArray<AAuraProjectile*> Projectiles;
	Projectiles.Reserve(NumProjectilesToSpawn);
	for (const auto& Rotation : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(InProjectileSpawnLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform, GetAvatarActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectiles.Add(Projectile);

		Projectile->OwningAbility = this;
		Projectile->EffectPolicies = EffectPolicies;
		Projectile->VolumeMultiplier = NumProjectilesToSpawn > 3 ? 0.5f : 1.f;
		
		Projectile->FinishSpawning(SpawnTransform);
	}

	// 타겟을 추적하는 Ability인 경우 들어가는 분기입니다.
	if (bHoming)
	{
		for (const auto& Projectile : Projectiles)
		{
			if (HomingTarget.IsValid() && HomingTarget->Implements<UCombatInterface>())
			{
				// 추적할 타겟이 검출된 경우 들어오는 분기입니다.
				Projectile->MulticastSetHomingTargetComponent(HomingTarget->GetRootComponent(), FMath::RandRange(HomingAccelerationMin, HomingAccelerationMax));
			}
			else
			{
				// 추적할 타겟이 마우스로 검출되지 않은 경우 들어오는 분기입니다.
				Projectile->MulticastSpawnHomingTargetComponent(InProjectileTargetLocation, FMath::RandRange(HomingAccelerationMin, HomingAccelerationMax));
			}
		}
	}

	// 타겟 추적 관련 변수를 초기화합니다.
	ProjectileTargetLocation = FVector::ZeroVector;
	HomingTarget = nullptr;

	return Projectiles;
}
