#include "AuraProjectileSpell.h"

#include "Aura/Actor/AuraProjectile.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Manager/AuraTextManager.h"

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

void UAuraProjectileSpell::SpawnProjectile(FVector& ProjectileSpawnLocation, FVector& ProjectileTargetLocation)
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
	
	TArray<FTransform> SpawnTransforms;
	SpawnTransforms.Reserve(NumProjectilesToSpawn);

	// ProjectileSpread가 중심각이 되는 부채꼴 모양으로 퍼지도록 계산합니다.
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-ProjectileSpread / 2.f, FVector::UpVector);
	const float DeltaSpread = NumProjectilesToSpawn > 1 ? ProjectileSpread / (NumProjectilesToSpawn - 1) : 0.f;

	for (int i = 0; i < NumProjectilesToSpawn; i++)
	{
		FTransform SpawnTransform;
		const FVector Direction = NumProjectilesToSpawn > 1 ? LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector) : Forward;
		SpawnTransform.SetLocation(ProjectileSpawnLocation);
		SpawnTransform.SetRotation(Direction.Rotation().Quaternion());
		SpawnTransforms.Add(SpawnTransform);
	}

	// SpawnActor는 생성 직후 BeginPlay까지 호출하지만 SpawnActorDeferred는 구성만 하고 생성은 대기합니다.
	TArray<AAuraProjectile*> Projectiles;
	Projectiles.Reserve(NumProjectilesToSpawn);
	for (const auto& SpawnTransform : SpawnTransforms)
	{
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform, GetAvatarActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectiles.Add(Projectile);
	}

	// Ability를 소유한 AvatarActor의 AbilitySystemComponent 가져옵니다.
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

	if (DamageTypes.Num() > 0)
	{
		// Damage Context를 생성 및 초기화합니다.
		DamageEffectContextHandle = SourceASC->MakeEffectContext();
		DamageEffectContextHandle.SetAbility(this);
		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		DamageEffectContextHandle.AddHitResult(HitResult);
		const bool bKnockback = FMath::FRandRange(0.f, 100.f) < KnockbackChance;

		for (const auto& Projectile : Projectiles)
		{
			Projectile->DamageEffectContextHandle = DamageEffectContextHandle;
			Projectile->DeathImpulseMagnitude = DeathImpulseMagnitude;

			if (bKnockback)
			{
				Projectile->KnockbackForceMagnitude = KnockbackForceMagnitude;
			}

			// 적중 시 데미지를 줄 수 있도록 Projectile에 Spec을 할당합니다.
			Projectile->DamageEffectSpecHandle = MakeDamageSpecHandle();
		}
	}

	if (DebuffData.Num() > 0)
	{
		// Debuff Context를 생성 및 초기화합니다.
		DebuffEffectContextHandle = SourceASC->MakeEffectContext();
		DebuffEffectContextHandle.SetAbility(this);

		for (const auto& Projectile : Projectiles)
		{
			Projectile->DebuffEffectContextHandle = DebuffEffectContextHandle;

			// 적중 시 디버프를 줄 수 있도록 Projectile에 Spec을 할당합니다.
			Projectile->DebuffEffectSpecHandle = MakeDebuffSpecHandle();
		}
	}

	int32 Index = 0;
	for (const auto& Projectile : Projectiles)
	{
		// 액터 스폰
		Projectile->FinishSpawning(SpawnTransforms[Index++]);
	}
}
