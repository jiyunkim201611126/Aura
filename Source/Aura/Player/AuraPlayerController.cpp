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
	AutoRun();
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

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		// 곡선 경로 전체 중 캐릭터의 위치로부터 가장 가까운 Location을 구함 
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		// 구해진 해당 Location을 통해 곡선의 방향을 구함
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		// 해당 방향으로 이동 시작
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
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

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	// RMB Input인 경우 들어가는 분기
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		// 현재 마우스 아래에 Enemy가 존재하면 bTargeting이 true가 됨
		bTargeting = ThisActor ? true : false;
		// 꾹 누른 시간 초기화하며 측정 시작
		FollowTime = 0.f;
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
		
		// FollowTime(꾹 누르고 있던 시간)을 ShortPressThreshold(Released 이벤트가 발생하지 않는 임계점)와 비교
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 마우스 위치로 LineTrace해서 위치 캐싱
			FHitResult Hit;
			if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
			{
				CachedDestination = Hit.ImpactPoint;
			}
			
			// 캐릭터의 현재 위치로부터 마우스가 Release된 Location까지 최단거리 계산
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				// 기존 경로 삭제
				Spline->ClearSplinePoints();
				// SplineComponent를 이용해 최단거리인 직선 경로를 곡선으로 변환 
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
				}
				// NavigationSystem이 지정한 목적지(곡선 경로의 끝)로 값 변경 
				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				// 목적지까지 자동 달리기 시작
				bAutoRunning = true;
			}
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
