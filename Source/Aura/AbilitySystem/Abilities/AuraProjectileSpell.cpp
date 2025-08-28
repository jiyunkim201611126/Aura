#include "AuraProjectileSpell.h"

#include "Aura/Actor/AuraProjectile.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Manager/AuraTextManager.h"

FText UAuraProjectileSpell::GetDescription_Implementation(int32 Level)
{
	return FText::Format(FAuraTextManager::GetText(EStringTableTextType::UI, DescriptionKey), Level, -GetManaCost(Level), GetCooldown(Level), GetDamageTexts(Level), NumOfProjectiles);
}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraProjectileSpell::SpawnProjectile(FVector& ProjectileSpawnLocation, FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	// Character가 가진 Weapon의 Socket 위치를 가져와야 하므로, OwningActor(PlayerState)가 아닌 AvatarActor(Character)를 필요로 합니다.
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		// Projectile이 스폰될 위치와 날아갈 방향을 결정합니다.
		ProjectileSpawnLocation.Z = GetAvatarActorFromActorInfo()->GetActorLocation().Z;
		FRotator Rotation = (ProjectileTargetLocation - ProjectileSpawnLocation).Rotation();
		
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(ProjectileSpawnLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());

		// SpawnActor는 생성 직후 BeginPlay까지 호출하지만 SpawnActorDeferred는 구성만 하고 생성은 대기합니다.
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform, GetAvatarActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// Ability를 소유한 AvatarActor의 AbilitySystemComponent 가져옵니다.
		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

		if (DamageTypes.Num() > 0)
		{
			// Damage Context를 생성 및 초기화합니다.
			DamageEffectContextHandle = SourceASC->MakeEffectContext();
			DamageEffectContextHandle.SetAbility(this);
			DamageEffectContextHandle.Get()->AddSourceObject(Projectile);
			FHitResult HitResult;
			HitResult.Location = ProjectileTargetLocation;
			DamageEffectContextHandle.AddHitResult(HitResult);

			Projectile->DamageEffectContextHandle = DamageEffectContextHandle;
			Projectile->DeathImpulseMagnitude = DeathImpulseMagnitude;

			// 적중 시 데미지를 줄 수 있도록 Projectile에 Spec을 할당합니다.
			Projectile->DamageEffectSpecHandle = MakeDamageSpecHandle();
		}

		if (DebuffData.Num() > 0)
		{
			// Debuff Context를 생성 및 초기화합니다.
			DebuffEffectContextHandle = SourceASC->MakeEffectContext();
			DebuffEffectContextHandle.SetAbility(this);

			Projectile->DebuffEffectContextHandle = DebuffEffectContextHandle;

			// 적중 시 디버프를 줄 수 있도록 Projectile에 Spec을 할당합니다.
			Projectile->DebuffEffectSpecHandle = MakeDebuffSpecHandle();
		}

		// 액터 스폰
		Projectile->FinishSpawning(SpawnTransform);
	}
}
