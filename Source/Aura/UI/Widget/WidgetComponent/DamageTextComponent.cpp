#include "DamageTextComponent.h"

void UDamageTextComponent::BeginPlay()
{
	Super::BeginPlay();
	InitMovement();
}

void UDamageTextComponent::InitMovement()
{
	// 랜덤 수평 방향 선언
	FVector HorizontalDir = FMath::VRand();
	HorizontalDir.Z = 0;
	HorizontalDir.Normalize();

	// 초기 속도 설정 (수평 + 수직)
	Velocity = HorizontalDir * InitialSpeed;
	Velocity.Z = InitialSpeed;
}

void UDamageTextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 중력 적용
	float AppliedGravity = Gravity;
	if (bIsFalling)
	{
		// 최고점 도달 이후 중력 약하게 적용
		AppliedGravity *= DescentGravityScale;
	}

	// 수직 속도 감소
	Velocity.Z -= AppliedGravity * DeltaTime;

	// 최고점 도달 시 속도 음수로 전환 (하강 시작)
	if (!bIsFalling && Velocity.Z < 0)
	{
		bIsFalling = true;
	}

	// 위치 업데이트
	FVector DeltaPosition = Velocity * DeltaTime;
	AddLocalOffset(DeltaPosition, true);
}