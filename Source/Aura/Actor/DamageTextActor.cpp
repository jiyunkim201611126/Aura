#include "DamageTextActor.h"

#include "GameFramework/ProjectileMovementComponent.h"

ADamageTextActor::ADamageTextActor()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 2.f;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.5f;
}

void ADamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	InitMovement();
}

void ADamageTextActor::InitMovement()
{
	// 랜덤으로 수평 방향을 지정합니다.
	FVector HorizontalDirection = FMath::VRand();
	HorizontalDirection.Z = 0;
	HorizontalDirection.Normalize();

	// 초기 속도를 설정합니다.
	FVector InitialVelocity = HorizontalDirection * InitialSpeed;
	InitialVelocity.Z = InitialSpeed;

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = InitialVelocity;
	}
}
