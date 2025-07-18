﻿#include "AuraProjectile.h"

#include "Components/SphereComponent.h"
#include "Aura/Aura.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Manager/FXManagerSubsystem.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

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
	SetLifeSpan(LifeSpan);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);

	if (const UFXManagerSubsystem* FXManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
	{
		USoundBase* LoopingSound = FXManagerSubsystem->GetSound(LoopingSoundTag);
		if (LoopingSound)
		{
			LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
		}
	}
}

void AAuraProjectile::Destroyed()
{
	if (!GetWorld() || !GetWorld()->GetGameInstance())
	{
		return;
	}
	
	// 클라이언트에서 bHit이 false라면 아직 사운드와 나이아가라가 재생되지 않은 상태
	// 그 상태로 Destroyed 함수가 호출됐다면 사운드와 나이아가라를 재생해줌
	if (!bHit)
	{
		if (const UFXManagerSubsystem* FXManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
		{
			USoundBase* ImpactSound = FXManagerSubsystem->GetSound(ImpactSoundTag);
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
			}
			UNiagaraSystem* ImpactEffect = FXManagerSubsystem->GetNiagara(ImpactEffectTag);
			if (ImpactEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
			}
		}
		
		if (LoopingSoundComponent)
		{
			LoopingSoundComponent->Stop();
		}
		bHit = true;
	}
	
	Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetWorld() || !GetWorld()->GetGameInstance())
	{
		return;
	}
	
	// GameplayEffectSpec이 아직 유효하지 않을 때 Overlap되거나, Projectile을 발사한 캐릭터 자신이 부딪히면 이 이벤트를 무시함 
	if (!DamageEffectSpecHandle.Data.IsValid() || DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor)
	{
		return;
	}
	
	if (UAuraAbilitySystemLibrary::IsFriend(GetOwner(), OtherActor))
	{
		return;
	}
	
	// 서버, 클라이언트 모두 사운드와 나이아가라 재생
	if (!bHit)
	{
		if (const UFXManagerSubsystem* FXManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
		{
			USoundBase* ImpactSound = FXManagerSubsystem->GetSound(ImpactSoundTag);
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
			}
			UNiagaraSystem* ImpactEffect = FXManagerSubsystem->GetNiagara(ImpactEffectTag);
			if (ImpactEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
			}
		}
		
		if (LoopingSoundComponent)
		{
			LoopingSoundComponent->Stop();
		}
	}

	// 서버인 경우 데미지를 주며 Destroy 이벤트 바인드
	if (HasAuthority())
	{
		// Overlap 대상에게 데미지 부여
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}

		// 스폰과 동시에 Overlap 이벤트가 일어난 경우 바로 Destroy되면서 액터가 클라이언트로 복제되지 않는 현상이 있음
		// 따라서 0.05초의 딜레이를 주어 액터 스폰이 클라이언트로 복제되는 것을 보장
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

	// 기존 수명으로 인해 Destroy되면 원치 않는 Destroy가 발생할 수 있으므로 LifeSpan 초기화
	SetLifeSpan(0.f);
	// 서버와 클라이언트 모두 Overlap 이벤트가 한 번 일어났음을 저장
	bHit = true;
	// 더이상 Overlap 이벤트가 필요하지 않으므로 바인드 해제
	Sphere->OnComponentBeginOverlap.Clear();
}
