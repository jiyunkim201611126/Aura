#include "AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Input/AuraInputComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	/**
	 * 커서에서 라인 트레이스, 아래는 경우의 수 5가지
	 *  1. LastActor == null && ThisActor == null
	 *		- Do nothing
	 *	2. LastActor == null && ThisActor == valid
	 *		- Highlight ThisActor
	 *	3. LastActor == valid && ThisActor == null
	 *		- UnHighlight LastActor
	 *	4. Both actors are valid, but LastActor != ThisActor
	 *		- UnHighlight LastActor
	 *		- Highlight ThisActor
	 *	5. Both actors are valid, and LastActor == ThisActor
	 *		- Do nothing
	 */

	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			// case 2
			ThisActor->HighlightActor();
		}
		else
		{
			// case 1 - both are null
		}
	}
	else // LastActor is valid
	{
		if (ThisActor == nullptr)
		{
			// case 3
			LastActor->UnHighlightActor();
		}
		else
		{
			if (LastActor != ThisActor)
			{
				// case 4
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
			else
			{
				// case 5
			}
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// InputComponent는 ProjectSettings의 Input 탭에서 지정한다.
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	// InputComponent에게 InputConfig(DataAsset)과 함수 포인터들을 전달
	AuraInputComponent->BindAbilityActions(InputConfig, this,
		&ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2d InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	// RMB Input인 경우 들어가는 분기
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		// 현재 마우스 아래에 Enemy가 존재하면 bTargeting이 true가 됨
		bTargeting = ThisActor ? true : false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	// RMB Input이 아닌 경우 들어가는 분기
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// RMB Input인 경우 들어가는 분기
	if (bTargeting)
	{
		// 마우스 아래 Enemy가 있으면 Ability 사용
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		// 마우스 아래 Enemy가 없으면 Move를 원한다고 간주, 얼마나 오랜 시간 동안 누르고 있었는지 기록 시작
		FollowTime += GetWorld()->GetDeltaSeconds();
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	// RMB Input이 아닌 경우 들어가는 분기
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}
	
	// RMB Input인 경우 들어가는 분기
	if (bTargeting)
	{
		// 마우스 아래 Enemy가 있으면 Ability 사용
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
	}
	else
	{
		// 마우스 아래 Enemy가 없으면 Move를 원한다고 간주
		APawn* ControlledPawn = GetPawn();
		
		// FollowTime(꾹 누르고 있던 시간)을 ShortPressThreshold와 비교
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 마우스 위치로 LineTrace해서 위치 캐싱
			FHitResult Hit;
			if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
			{
				CachedDestination = Hit.Location;
			}
			
			// 경로 계산
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
				}
			}
			FollowTime = 0.f;
			bTargeting = false;
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}
