#include "AuraProjectile.h"
#include "Components/SphereComponent.h"
#include "Aura/Aura.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/AbilitySystem/Abilities/AbilityEffectPolicy/AbilityEffectPolicy.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/FXManagerSubsystem.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InitialLifeSpan = 1.f;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 500.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);

	if (LoopingSoundTag.IsValid())
	{
		if (UFXManagerSubsystem* FXManager = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
		{
			FXManager->AsyncGetSound(LoopingSoundTag, [this](USoundBase* LoopingSound)
			{
				if (LoopingSound)
				{
					LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
					LoopingSoundComponent->SetVolumeMultiplier(VolumeMultiplier);
				}
			});
		}
	}
}

void AAuraProjectile::Destroyed()
{
	if (!GetWorld() || !GetWorld()->GetGameInstance())
	{
		return;
	}

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	
	PlayHitFXs();
	
	Super::Destroyed();
}

void AAuraProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ProjectileMovement->HomingTargetComponent.IsValid())
	{
		// 추적 중인 타겟이 사망한 경우 일반 Projectile로 변경합니다.
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
	}
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(OtherActor))
	{
		return;
	}
	
	if (UAuraAbilitySystemLibrary::IsFriend(GetOwner(), OtherActor))
	{
		return;
	}
	
	if (!bDestroyWithOverlap)
	{
		PlayHitFXs();
	}

	// 서버인 경우 Effect를 부여합니다.
	if (HasAuthority())
	{
		// Overlap 대상에게 Effect 부여합니다.
		for (const auto EffectPolicy : EffectPolicies)
		{
			// RadialFallOffDamage를 갖고 있을 수 있으므로, 그에 필요한 변수를 할당해 부여합니다.
			FEffectPolicyContext EffectPolicyContext;
			EffectPolicyContext.OriginVector = GetActorLocation();
			EffectPolicy->ApplyEffect(OwningAbility.Get(), OtherActor, EffectPolicyContext);
		}
		
		if (bDestroyWithOverlap)
		{
			// 스폰과 동시에 Overlap 이벤트가 일어난 경우 바로 Destroy되면서 액터가 클라이언트로 복제되지 않는 현상이 있습니다.
			// 따라서 0.05초의 딜레이를 주어 액터 스폰이 클라이언트로 복제되는 것을 보장합니다.
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				FTimerDelegate::CreateLambda([this]()
				{
					Destroy();
				}),
				0.05f,
				false
			);
		}
	}
	
	if (bDestroyWithOverlap)
	{
		// 기존 수명으로 인해 Destroy되면 원치 않는 Destroy가 발생할 수 있으므로 LifeSpan 초기화
		SetLifeSpan(0.f);
		// 더이상 Overlap 이벤트가 필요하지 않으므로 바인드 해제
		Sphere->OnComponentBeginOverlap.Clear();
	}
}

void AAuraProjectile::PlayHitFXs() const
{
	if (UFXManagerSubsystem* FXManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
	{
		if (ImpactSoundTag.IsValid())
		{
			FXManagerSubsystem->AsyncPlaySoundAtLocation(ImpactSoundTag, GetActorLocation(), FRotator::ZeroRotator, VolumeMultiplier);
		}
		if (ImpactEffectTag.IsValid())
		{
			FXManagerSubsystem->AsyncSpawnNiagaraAtLocation(ImpactEffectTag, GetActorLocation());
		}
	}
}

void AAuraProjectile::MulticastSpawnHomingTargetComponent_Implementation(const FVector_NetQuantize& TargetLocation, const float HomingAcceleration)
{
	USceneComponent* HomingTargetComponent = NewObject<USceneComponent>();
	HomingTargetComponent->SetWorldLocation(TargetLocation);
	ProjectileMovement->HomingTargetComponent = HomingTargetComponent;
	
	// 추적 속도를 결정합니다.
	ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
	ProjectileMovement->bIsHomingProjectile = true;

	// 적을 추적하는 Projectile은 추적 도중 적이 사망했을 때를 대비한 로직을 위해 Tick을 활성화합니다.
	SetActorTickEnabled(true);
	SetActorTickInterval(0.2f);
}

void AAuraProjectile::MulticastSetHomingTargetComponent_Implementation(USceneComponent* HomingTargetComponent, const float HomingAcceleration)
{
	ProjectileMovement->HomingTargetComponent = HomingTargetComponent;
	ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
	ProjectileMovement->bIsHomingProjectile = true;
	
	SetActorTickEnabled(true);
	SetActorTickInterval(0.2f);
}
