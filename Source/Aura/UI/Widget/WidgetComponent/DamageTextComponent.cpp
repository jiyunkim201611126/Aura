#include "DamageTextComponent.h"

void UDamageTextComponent::BeginPlay()
{
	Super::BeginPlay();
	InitMovement();
}

void UDamageTextComponent::InitMovement()
{
	// 랜덤한 수평 방향 계산 (X,Y)
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
		AppliedGravity *= DescentGravityScale; // 하강 시 중력 감소
	}

	// DeltaTime에 비례해 수직 속도 감소
	Velocity.Z -= AppliedGravity * DeltaTime;

	// 최고점 도달 체크 (속도가 음수가 되는 순간)
	if (!bIsFalling && Velocity.Z < 0)
	{
		bIsFalling = true;
	}

	// 위치 업데이트
	FVector DeltaPosition = Velocity * DeltaTime;
	AddLocalOffset(DeltaPosition, true);
}