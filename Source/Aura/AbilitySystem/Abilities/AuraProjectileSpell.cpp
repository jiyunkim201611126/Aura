#include "AuraProjectileSpell.h"

#include "Aura/Actor/AuraProjectile.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraProjectileSpell::SpawnProjectile(FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	// Character가 가진 Weapon의 Socket 위치를 가져와야 하므로, OwningActor(PlayerState)가 아닌 AvatarActor(Character)를 필요로 함
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		// Projectile이 스폰될 위치와 날아갈 방향 결정
		float ProjectileHeight = GetAvatarActorFromActorInfo()->GetActorLocation().Z / 2.f;
		FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), FAuraGameplayTags::Get().Montage_Attack_Weapon, FName("TipSocket"));
		SocketLocation.Z = ProjectileHeight;
		ProjectileTargetLocation.Z = ProjectileHeight;
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());

		// SpawnActor는 생성 직후 BeginPlay까지 호출하지만 SpawnActorDeferred는 구성만 하고 생성은 대기함
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// Ability를 소유한 AvatarActor의 AbilitySystemComponent 가져오기
		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

		// Context 생성 및 초기화
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(Projectile);
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(Projectile);
		EffectContextHandle.AddActors(Actors);
		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);

		// 할당받은 DamageEffectClass를 기반으로 Projectile이 가질 GameplayEffectSpecHandle을 생성
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

		FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

		// 바로 위에서 만든 GE에 Damage 값을 할당해주기 위한 구문, Damage 타입별 태그와 함께 Damage 값을 부여
		for (auto& Pair : DamageTypes)
		{
			// Value는 FScalableFloat으로, 에디터에서 할당한 커브 테이블을 이용해 값을 가져옴
			const float AbilityLevel = GetAbilityLevel();
			const float ScaledDamage = Pair.Value.GetValueAtLevel(AbilityLevel);
			// Spec 안에 SetByCallerMagnitudes라는 이름의 TMap이 있으며, 거기에 Tag를 키, Damage를 밸류로 값을 추가하는 함수
			// 이 값은 GetSetByCallerMagnitude로 꺼내올 수 있음
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}
		
		Projectile->DamageEffectSpecHandle = SpecHandle;

		// 액터 스폰
		Projectile->FinishSpawning(SpawnTransform);
	}
}
