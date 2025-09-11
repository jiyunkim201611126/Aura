#include "DamageTextActor.h"

ADamageTextActor::ADamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 2.f;
}

void ADamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	InitMovement();

	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle,
		[this]()
		{
			Destroy();
		},
		2.0f,
		false);
}

void ADamageTextActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float AppliedGravity = Gravity;
	if (bIsFalling)
	{
		// 최고점 도달 이후부터 중력을 약하게 적용합니다.
		AppliedGravity *= DescentGravityScale;
	}

	// 시간에 따라 수직 속도를 감소시킵니다.
	Velocity.Z -= AppliedGravity * DeltaSeconds;

	// 최고점 도달 시 속도를 음수로 전환, 하강을 시작합니다.
	if (!bIsFalling && Velocity.Z < 0)
	{
		bIsFalling = true;
	}

	const FVector DeltaPosition = Velocity * DeltaSeconds;
	AddActorLocalOffset(DeltaPosition, true);
}

void ADamageTextActor::InitMovement()
{
	// 랜덤으로 수평 방향을 지정합니다.
	FVector HorizontalDirection = FMath::VRand();
	HorizontalDirection.Z = 0;
	HorizontalDirection.Normalize();

	// 초기 속도를 설정합니다.
	Velocity = HorizontalDirection * InitialSpeed;
	Velocity.Z = InitialSpeed;
}
