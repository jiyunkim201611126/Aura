#include "AuraProjectileSpell.h"

#include "Aura/Actor/AuraProjectile.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Manager/AuraTextManager.h"
#include "GameFramework/ProjectileMovementComponent.h"

FText UAuraProjectileSpell::GetDescription_Implementation(const int32 Level)
{
	return FText::Format(FAuraTextManager::GetText(EStringTableTextType::UI, DescriptionKey), Level, -GetManaCost(Level), GetCooldown(Level), GetDamageTexts(Level), GetProjectileNumsToSpawn(Level));
}

int32 UAuraProjectileSpell::GetProjectileNumsToSpawn(const int32 Level) const
{
	return NumOfProjectiles.GetValueAtLevel(Level);
}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraProjectileSpell::SpawnProjectile(FVector& ProjectileSpawnLocation, FVector& ProjectileTargetLocation, const float PitchOverride, const AActor* HomingTarget)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	// 스폰할 Projectile의 개수를 가져옵니다.
	const int32 NumProjectilesToSpawn = GetProjectileNumsToSpawn(GetAbilityLevel());
	
	// Projectile이 스폰될 위치와 날아갈 방향을 결정합니다.
	ProjectileSpawnLocation.Z = GetAvatarActorFromActorInfo()->GetActorLocation().Z;
	const FVector Forward = ProjectileTargetLocation - ProjectileSpawnLocation;
	
	// ProjectileSpread가 중심각이 되는 부채꼴 모양으로 퍼지도록 계산합니다.
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectilesToSpawn);
	if (PitchOverride > 0.f)
	{
		for (auto& Rotation : Rotations)
		{
			Rotation.Pitch += PitchOverride;
		}
	}

	// SpawnActor는 생성 직후 BeginPlay까지 호출하지만 SpawnActorDeferred는 구성만 하고 생성은 대기합니다.
	TArray<AAuraProjectile*> Projectiles;
	Projectiles.Reserve(NumProjectilesToSpawn);
	for (const auto& Rotation : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(ProjectileSpawnLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform, GetAvatarActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectiles.Add(Projectile);
		
		SetHandlesToProjectile(Projectile, ProjectileTargetLocation);
		
		Projectile->FinishSpawning(SpawnTransform);
	}

	if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
	{
		for (const auto& Projectile : Projectiles)
		{
			UProjectileMovementComponent* ProjectileMovement = Projectile->ProjectileMovement;
			ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
			ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
			ProjectileMovement->bIsHomingProjectile = true;
		}
	}
}

void UAuraProjectileSpell::SetHandlesToProjectile(AAuraProjectile* Projectile, const FVector& TargetLocation)
{
	// Ability를 소유한 AvatarActor의 AbilitySystemComponent 가져옵니다.
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

	if (DamageTypes.Num() > 0)
	{
		// Damage Context를 생성 및 초기화합니다.
		if (!DamageEffectContextHandle.IsValid())
		{
			DamageEffectContextHandle = SourceASC->MakeEffectContext();
			DamageEffectContextHandle.SetAbility(this);
			FHitResult HitResult;
			HitResult.Location = TargetLocation;
			DamageEffectContextHandle.AddHitResult(HitResult);
		}
		
		Projectile->DamageEffectContextHandle = DamageEffectContextHandle;
		Projectile->DeathImpulseMagnitude = DeathImpulseMagnitude;

		if (FMath::FRandRange(0.f, 100.f) < KnockbackChance)
		{
			Projectile->KnockbackForceMagnitude = KnockbackForceMagnitude;
		}

		// 적중 시 데미지를 줄 수 있도록 Projectile에 Spec을 할당합니다.
		Projectile->DamageEffectSpecHandle = MakeDamageSpecHandle();
	}

	if (DebuffData.Num() > 0)
	{
		// Debuff Context를 생성 및 초기화합니다.
		if (!DebuffEffectContextHandle.IsValid())
		{
			DebuffEffectContextHandle = SourceASC->MakeEffectContext();
			DebuffEffectContextHandle.SetAbility(this);
		}

		Projectile->DebuffEffectContextHandle = DebuffEffectContextHandle;

		// 적중 시 디버프를 줄 수 있도록 Projectile에 Spec을 할당합니다.
		Projectile->DebuffEffectSpecHandle = MakeDebuffSpecHandle();
	}
}
